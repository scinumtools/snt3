#include "pch_tests.h"
#include <snt/puq/systems/unit_system.h>
#include <snt/puq/value/dimensions.h>
#include <snt/puq/value/unit_value.h>

#include <set>

using namespace snt;

inline void check_symbol(std::set<std::string>& set, const std::string& symbol) {
  auto result = set.insert(symbol);
  EXPECT_EQ(result.second, true) << "Duplicated symbol: " + symbol;
}

void test_unit_symbols() {

  // unique prefixes
  std::set<std::string> prefixes;
  for (const auto& prefix : puq::UnitPrefixList) {
    check_symbol(prefixes, prefix.first);
  }
  // unique unit symbols
  std::set<std::string> units;
  for (const auto& unit : puq::UnitSystem::Data->UnitList) {
    // unique unit symbol without a prefix
    check_symbol(units, unit.first);
    if (unit.second.use_prefixes) {
      if (unit.second.allowed_prefixes.size() > 0) {
        // unique symbols with allowed prefixes
        for (const auto& prefix : unit.second.allowed_prefixes) {
          check_symbol(units, std::string(prefix + unit.first));
        }
      } else {
        // unique symbols with all prefixes
        for (const auto& prefix : prefixes) {
          check_symbol(units, std::string(prefix + unit.first));
        }
      }
    }
  }
}

void test_unit_definitions() {

  for (const auto& unit : puq::UnitSystem::Data->UnitList) {

    // std::cout << "hello " << unit.first << " " << std::bitset<8>((int)unit.second.utype) << std::endl;
    // if ((unit.second.utype & Utype::LIN)!=Utype::LIN) // check only linear units
    //   continue;
    if (unit.second.utype == puq::Utype::NUL) // ignore null units
      continue;
    if ((unit.second.utype & puq::Utype::BAS) == puq::Utype::BAS) // ignore base units
      continue;

    puq::DimensionStruct dmap = puq::UnitSystem::Data->DimensionMap.at(unit.first);
#ifdef MAGNITUDE_ERRORS
    puq::MAGNITUDE_TYPE magnitude(dmap.magnitude, dmap.error);
#else
    puq::MAGNITUDE_TYPE magnitude(dmap.magnitude);
#endif
    puq::Dimensions dim1(magnitude, dmap.dimensions);
    std::string m1 = dim1.to_string();

    puq::UnitValue uv2(unit.second.definition);
    puq::Dimensions dim2 = uv2.baseunits.dimensions();
    dim2 = puq::Dimensions(uv2.magnitude * dim2.numerical, dim2.physical);
    std::string m2 = dim2.to_string();

    if constexpr (puq::Config::use_units_logarithmic) {
      if (unit.second.utype == puq::Utype::LOG) {
	// For logarithmic units compare only the physical dimensionality
	m1 = dim1.to_string({puq::Format::Display::UNITS});
	m2 = dim2.to_string({puq::Format::Display::UNITS});
      }
    }

    /*
    EXPECT_DOUBLE_EQ(unit.magnitude, dim2.numerical.value[0])
      << "Magnitude of unit '" << unit.first
      << "' does not match with its definition: "
      << puq::nostd::to_string(unit.magnitude) << " != "
      << puq::nostd::to_string(dim2.numerical);
    */
    EXPECT_EQ(m1, m2) << "Dimension of unit '" << unit.first
                      << "' do not match its definition: "
                      << m1 << " != " << m2;
  }
}

void test_quantities() {
  for (const auto& quantity : puq::UnitSystem::Data->QuantityList) {

    // check if quantity symbol is in the quantity name list
    EXPECT_FALSE(puq::QuantityNames.find(quantity.first) == puq::QuantityNames.end())
        << "Quantity symbol is not in a name list: " << quantity.first;

    /*
    // check if quantity definition matches magnitude and dimensions
    puq::Dimensions dim1(quantity.second.magnitude, quantity.second.dimensions);
    std::string m1 = dim1.to_string();

    puq::UnitValue uv2(quantity.second.definition);
    puq::Dimensions dim2 = uv2.baseunits.dimensions();
    dim2 = puq::Dimensions(uv2.magnitude*dim2.numerical, dim2.physical);
    std::string m2 = dim2.to_string();

    EXPECT_EQ(m1, m2) << "Dimensions of quantity '" << quantity.first
          << "' do not match its definition: "
          << m1 << " != " << m2;
    */
  }
}

TEST(Dmaps, QuantitySymbols) {

  std::set<std::string> quantities;
  for (const auto& quantity : puq::QuantityNames) {
    check_symbol(quantities, quantity.first);
  }
}

TEST(Dmaps, UnitDefinitionsSI) {

  test_unit_symbols();
  // test_unit_definitions();
  test_quantities();
}

TEST(Dmaps, UnitDefinitionsESU) {
  if constexpr (!puq::Config::use_system_cgs) {
    GTEST_SKIP() << "CGS unit system is disabled";
    return;
  }

  puq::UnitSystem us(puq::SystemType::ESU);
  test_unit_symbols();
  // test_unit_definitions();
}

TEST(Dmaps, UnitDefinitionsGauss) {
  if constexpr (!puq::Config::use_system_cgs) {
    GTEST_SKIP() << "CGS unit system is disabled";
    return;
  }

  puq::UnitSystem us(puq::SystemType::GU);
  test_unit_symbols();
  // test_unit_definitions();
}

TEST(Dmaps, UnitDefinitionsEMU) {
  if constexpr (!puq::Config::use_system_cgs) {
    GTEST_SKIP() << "CGS unit system is disabled";
    return;
  }

  puq::UnitSystem us(puq::SystemType::EMU);
  test_unit_symbols();
  // test_unit_definitions();
}

TEST(Dmaps, UnitDefinitionsAU) {
  if constexpr (!puq::Config::use_system_nus) {
    GTEST_SKIP() << "CGS unit system is disabled";
    return;
  }

  puq::UnitSystem us(puq::SystemType::AU);
  test_unit_symbols();
  // test_unit_definitions();
  test_quantities();
}

TEST(Dmaps, UnitDefinitionsIU) {
  if constexpr (!puq::Config::use_system_eus) {
    GTEST_SKIP() << "CGS unit system is disabled";
    return;
  }

  puq::UnitSystem us(puq::SystemType::IU);
  test_unit_symbols();
  // test_unit_definitions();
}

TEST(Dmaps, UnitDefinitionsUS) {
  if constexpr (!puq::Config::use_system_eus) {
    GTEST_SKIP() << "CGS unit system is disabled";
    return;
  }

  puq::UnitSystem us(puq::SystemType::US);
  test_unit_symbols();
  // test_unit_definitions();
}

TEST(Dmaps, DimensionMap) {

  auto it = puq::UnitSystem::Data->DimensionMap.find("<B>");
  EXPECT_TRUE(it != puq::UnitSystem::Data->DimensionMap.end());
}

#ifdef MAGNITUDE_ERRORS
TEST(Dmaps, DimensionMapErrors) {

  auto it = puq::UnitSystem::Data->DimensionMap.find("{a_0}");
  EXPECT_TRUE(it != puq::UnitSystem::Data->DimensionMap.end());
  EXPECT_FLOAT_EQ(it->second.magnitude, 5.2917721e-11);
  EXPECT_FLOAT_EQ(it->second.error, 8.2e-21);
}
#endif
