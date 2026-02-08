#include "converter.h"

#include "nostd/exp.h"
#include "nostd/log.h"
#include "nostd/log10.h"
#include "nostd/pow.h"

#include <iostream>
#include <set>

namespace snt::puq {

  MAGNITUDE_TYPE Converter::_convert_linear(const MAGNITUDE_TYPE& m1, const MAGNITUDE_TYPE& m2) {
    return (m1 * dimensions1.numerical) / (m2 * dimensions2.numerical);
  }

  inline MAGNITUDE_TYPE _convert_B_B(const MAGNITUDE_TYPE& value, const MAGNITUDE_TYPE& exp = 0) {
      return value + exp;
  }
  
  inline MAGNITUDE_TYPE _convert_B_Np(const MAGNITUDE_TYPE& value) {
      return (MAGNITUDE_TYPE)1.151277918 * value;
  }
  
  inline MAGNITUDE_TYPE _convert_Np_B(const MAGNITUDE_TYPE& value) {
      return value / (MAGNITUDE_TYPE)1.151277918;
  }
  
  inline MAGNITUDE_TYPE _convert_Ratio_B(const MAGNITUDE_TYPE& value, const MAGNITUDE_TYPE& exp) {
      return exp * nostd::log10(value);
  }
  
  inline MAGNITUDE_TYPE _convert_B_Ratio(const MAGNITUDE_TYPE& value, const MAGNITUDE_TYPE& exp) {
      return nostd::pow((MAGNITUDE_TYPE)10, value / exp);
  }
  
  inline MAGNITUDE_TYPE _convert_Ratio_Np(const MAGNITUDE_TYPE& value, const MAGNITUDE_TYPE& exp) {
      return exp * nostd::log(value);
  }
  
  inline MAGNITUDE_TYPE _convert_Np_Ratio(const MAGNITUDE_TYPE& value, const MAGNITUDE_TYPE& exp) {
      return nostd::exp(value / exp);
  }
  
  inline MAGNITUDE_TYPE _convert_Mw_Mo(const MAGNITUDE_TYPE& value) {
      return nostd::pow((MAGNITUDE_TYPE)10, 1.5 * (value + (MAGNITUDE_TYPE)10.7));
  }
  
  inline MAGNITUDE_TYPE _convert_Mo_Mw(const MAGNITUDE_TYPE& value) {
      return 2. / 3. * nostd::log10(value) - (MAGNITUDE_TYPE)10.7;
  }
  
  MAGNITUDE_TYPE Converter::_convert_logarithmic(const MAGNITUDE_TYPE& m) {
      const std::string s1 = baseunits1[0].unit;
      const std::string s2 = baseunits2[0].unit;
      const MAGNITUDE_TYPE n1 = dimensions1.numerical;
      const MAGNITUDE_TYPE n2 = dimensions2.numerical;
      if (s1 == "Np") { // Nepers -> Bel or Ratio
	if (s2 == "B")
	  return _convert_Np_B(m * n1) / n2;
	else if (s2 == "AR")
	  return _convert_Np_Ratio(m * n1, 1) / n2;
	else if (s2 == "PR")
	  return _convert_Np_Ratio(m * n1, 0.5) / n2;
      } else if (s2 == "Np") { // Bel or Ratio -> Nepers
	if (s1 == "B")
	  return _convert_B_Np(m * n1) / n2;
	else if (s1 == "AR")
	  return _convert_Ratio_Np(m * n1, 1) / n2;
	else if (s1 == "PR")
	  return _convert_Ratio_Np(m * n1, 0.5) / n2;
      } else if (s1 == "Mw" && (s2 == "Mo" || s2 == "dyn")) { // moment magnitude -> seismic moment
	return _convert_Mw_Mo(m * n1) / (n2 * 1e4);           // 1e4 to MGS
      } else if ((s1 == "Mo" || s1 == "dyn") && s2 == "Mw") { // seismic moment -> moment magnitude
	return _convert_Mo_Mw(m * n1 * 1e4) / n2;             // 1e4 to MGS
      } else {
	struct pair {
	  std::string first;
	  std::string second;
	  MagnitudeFloat exp;
	  MagnitudeFloat conv;
	};
	const std::vector<pair> pairs_B_B = {
          {"BW", "Bm", 3},
          {"BW", "BmW", 3},
          {"Bm", "BmW", 0},
          {"BV", "BuV", 12},
	};
	for (auto& pair : pairs_B_B) {
	  if (s1 == pair.first && s2 == pair.second) { // BelX -> BelY
	    return _convert_B_B(m * n1, pair.exp) / n2;
	  } else if (s1 == pair.second && s2 == pair.first) { // BelY -> BelX
	    return _convert_B_B(m * n1, -pair.exp) / n2;
	  }
	}
	const std::vector<pair> pairs_B_Ratio = {
          {"B", "PR", 1, 1},
          {"Bm", "W", 1, 1},
          {"BmW", "W", 1, 1},
          {"BW", "W", 1, 1e-3},
          {"BSIL", "W", 1, 1e9},
          {"BSWL", "W", 1, 1e9},
          {"B", "AR", 2, 1},
          {"BV", "V", 2, 1e-3},
          {"BuV", "V", 2, 1e3},
          {"BA", "A", 2, 1},
          {"BuA", "A", 2, 1e6},
          {"BOhm", "Ohm", 2, 1e-3},
          {"BSPL", "Pa", 2, 50},
	};
	for (auto& pair : pairs_B_Ratio) {
	  if (s1 == pair.first && s1 == s2) { // Bel -> Bel
	    return _convert_B_B(m * n1) / n2;
	  } else if (s1 == pair.first && s2 == pair.second) { // Bel -> Ratio
	    return _convert_B_Ratio(m * n1, pair.exp) / (n2 * pair.conv);
	  } else if (s1 == pair.second && s2 == pair.first) { // Ratio -> Bel
	    return _convert_Ratio_B(m * n1 * pair.conv, pair.exp) / n2;
	  }
	}
      }
      throw NoConvExcept(s1, s2);
  }

  MAGNITUDE_TYPE Converter::_convert_temperature(MAGNITUDE_TYPE m) {
    std::string s1 = baseunits1[0].unit;
    std::string s2 = baseunits2[0].unit;
    MAGNITUDE_TYPE const9div5 = 9. / 5.;
    MAGNITUDE_TYPE const5div9 = 5. / 9.;
    if (s1 == s2) {
      return m;
    } else {
      m = m * dimensions1.numerical;
    }
    if (s1 == "K" && s2 == "Cel") {
      m = m - (MAGNITUDE_TYPE)273.15;
    } else if (s1 == "K" && s2 == "degF") {
      m = (m - (MAGNITUDE_TYPE)273.15) * const9div5 + (MAGNITUDE_TYPE)32;
    } else if (s1 == "degR" && s2 == "degF") {
      m = m * const9div5 - (MAGNITUDE_TYPE)459.67;
    } else if (s1 == "degR" && s2 == "Cel") {
      m = (m * const9div5 - (MAGNITUDE_TYPE)491.67) * const5div9;
    } else if (s1 == "Cel" && s2 == "K") {
      m = m + (MAGNITUDE_TYPE)273.15;
    } else if (s1 == "Cel" && s2 == "degF") {
      m = (m * const9div5) + (MAGNITUDE_TYPE)32;
    } else if (s1 == "Cel" && s2 == "degR") {
      m = ((m * const9div5) + (MAGNITUDE_TYPE)491.67) * const5div9;
    } else if (s1 == "degF" && s2 == "K") {
      m = ((m - (MAGNITUDE_TYPE)32) * const5div9) + (MAGNITUDE_TYPE)273.15;
    } else if (s1 == "degF" && s2 == "Cel") {
      m = (m - (MAGNITUDE_TYPE)32) * const5div9;
    } else if (s1 == "degF" && s2 == "degR") {
      m = (m + (MAGNITUDE_TYPE)459.67) * const5div9;
    } else {
      throw NoConvExcept(s1, s2);
    }
    return m / dimensions2.numerical;
  }

  Converter::Converter(const BaseUnits& bu1, const BaseUnits& bu2) : baseunits1(bu1), baseunits2(bu2), utype(Utype::NUL) {
    // determine dimensions
    dimensions1 = bu1.dimensions();
    dimensions2 = bu2.dimensions();
    if (dimensions1 != dimensions2)
      throw snt::puq::ConvDimExcept(bu1, bu2);

    // determine conversion type
    // std::cout << baseunits1.to_string() << " " << baseunits2.to_string() << std::endl;
    // std::cout << std::bitset<8>((int)dimensions1.utype) << std::endl;
    // std::cout << std::bitset<8>((int)dimensions2.utype) << std::endl;
    if constexpr (Config::use_units_logarithmic) {
      if (((dimensions1.utype | dimensions2.utype) & Utype::LOG) == Utype::LOG) {
	if ((baseunits1.size() == 1 || baseunits1.size() == 2) &&
	    (baseunits2.size() == 1 || baseunits2.size() == 2)) {
	  utype = Utype::LOG;
	  goto finalize_init;
	}
      }
    }
    if constexpr (Config::use_units_temperature) {
      if (((dimensions1.utype | dimensions2.utype) & Utype::TMP) == Utype::TMP) {
	if ((baseunits1.size() == 1 || baseunits1.size() == 2) &&
	    (baseunits2.size() == 1 || baseunits2.size() == 2)) {
	  utype = Utype::TMP;
	  goto finalize_init;
	}
      }
    }
    if (((dimensions1.utype & dimensions2.utype) & Utype::LIN) == Utype::LIN) {
      utype = Utype::LIN;
    }
  finalize_init:
    ;
  };

  MAGNITUDE_TYPE Converter::convert(const MAGNITUDE_TYPE& m1, const MAGNITUDE_TYPE& m2) {
    if constexpr (Config::debug_converter) {
      std::clog << "CONV:  Solving: ";
      std::clog << m1.to_string() << Symbols::multiply << baseunits1.to_string();
      std::clog << " -> ";
      std::clog << m2.to_string() << Symbols::multiply << baseunits2.to_string();
      std::clog << std::endl;
    }
    MAGNITUDE_TYPE mag;
    if constexpr (Config::use_units_logarithmic) {
      if (utype == Utype::LOG && m2 == (MAGNITUDE_TYPE)1) {
	mag = _convert_logarithmic(m1);
	goto converted;
      }
    }
    if constexpr (Config::use_units_temperature) {
      if (utype == Utype::TMP && m2 == (MAGNITUDE_TYPE)1) {
	mag = _convert_temperature(m1);
	goto converted;
      }
    }
    if (utype == Utype::LIN)
      mag = _convert_linear(m1, m2);
    else
      throw NoConvExcept(baseunits1.to_string(), baseunits2.to_string());
  converted:
    if constexpr (Config::debug_converter) {
      std::clog << "CONV:  Result: ";
      std::clog << mag.to_string() << Symbols::multiply << baseunits2.to_string();
      std::clog << std::endl;
    }
    return mag;
  };

} // namespace snt::puq
