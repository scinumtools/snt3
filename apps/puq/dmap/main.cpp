#include <chrono>
#include <ctime>
#include <fstream>
#include <iostream>
#include <limits>
#include <snt/puq/math/pow.h>
#include <snt/puq/math/to_string.h>
#include <snt/puq/solver/unit_solver.h>
#include <snt/puq/systems/unit_system.h>
#include <snt/puq/value/dimensions.h>
#include <sstream>
#include <sys/stat.h>

using namespace snt;

void add_line(std::stringstream& ss, const std::string& symbol, puq::Dimensions& dim, const std::string& name) {
  ss << "{";
  ss << std::setfill(' ') << std::setw(15) << std::left;
  ss << "\"" + symbol + "\",";
  ss << "{";
  int precision = std::numeric_limits<double>::digits10;
  ss << std::setfill(' ') << std::setw(25) << std::left;
  val::ArrayValue<double> value(dim.numerical.estimate.get());
  ss << math::to_string(value.get_value(0), precision) + ",";
  ss << std::setfill(' ') << std::setw(25) << std::left;
  if (dim.numerical.uncertainty == nullptr) {
    ss << math::to_string(0, precision) + ",";
  } else {
    val::ArrayValue<double> uncertainty(dim.numerical.uncertainty.get());
    ss << math::to_string(uncertainty.get_value(0), precision) + ",";
  }
  // ss << std::setfill(' ') << std::setw(25) << std::left;
  // ss << math::to_string(dim.numerical.estimate[0], precision) + ",";
  // ss << std::setfill(' ') << std::setw(25) << std::left;
  // ss << math::to_string(dim.numerical.uncertainty[0], precision) + ",";
  ss << "{";
  for (int i = 0; i < puq::Config::num_basedim; i++) {
    if (i > 0)
      ss << ",";
    ss << std::setfill(' ') << std::setw(2) << std::right;
    if (std::holds_alternative<int>(dim.physical[i])) {
      ss << std::get<int>(dim.physical[i]);
    } else {
      puq::Exponent& exp = std::get<puq::Exponent>(dim.physical[i]);
      if (exp.denominator == 1) {
        ss << exp.numerator;
      } else {
        ss << "(FRC){" + std::to_string(exp.numerator) + "," + std::to_string(exp.denominator) + "}";
      }
    }
  }
  ss << "} } },     // " + name + "\n";
  // std::cout << ss.str() << std::endl;
}

void solve_bu_prefix(puq::Dimensions& dim, puq::BaseUnit& bu) {
  auto prefix = puq::UnitPrefixList.find(bu.prefix);
  if (prefix != puq::UnitPrefixList.end()) {
    dim.numerical *= math::pow(prefix->second.magnitude, exponent_to_float(bu.exponent));
  }
}

bool solve_bu_unit(puq::DimensionMapType& dmap, puq::Dimensions& dim, puq::BaseUnit& bu) {
  auto it = dmap.find(bu.unit);
  if (it != dmap.end()) {
    puq::Result m(it->second.magnitude, it->second.uncertainty);
    dim.numerical *= math::pow(m, exponent_to_float(bu.exponent));
    for (int i = 0; i < puq::Config::num_basedim; i++) {
      dim.physical[i] = add_exp(dim.physical[i], mul_exp(it->second.dimensions[i], bu.exponent));
    }
    return true;
  }
  return false;
}

inline void solve_units(std::stringstream& ss, puq::DimensionMapType& dmap, puq::UnitSolver& solver) {
  for (const auto& unit : puq::UnitSystem::Data->UnitList) {
    if (dmap.find(unit.first) != dmap.end())
      continue;
    if ((unit.second.utype & puq::Utype::BAS) == puq::Utype::BAS)
      continue;
    puq::UnitAtom atom = solver.solve(unit.second.definition);
    puq::Dimensions dim(atom.value.magnitude);
    std::string missing = "";
    for (auto bu : atom.value.baseunits) {
      solve_bu_prefix(dim, bu);
      if (!solve_bu_unit(dmap, dim, bu)) {
        missing = bu.unit;
        break;
      }
    }
    if ((unit.second.utype & puq::Utype::LOG) == puq::Utype::LOG)
      dim.numerical = 1; // for logarithmic units this has to be 1
    if (missing == "") {
      add_line(ss, unit.first, dim, unit.second.name);
      val::ArrayValue<double> value(dim.numerical.estimate.get());
      if (dim.numerical.uncertainty == nullptr) {
        dmap.insert({unit.first, {value.get_value(0), 0., dim.physical}});
      } else {
        val::ArrayValue<double> uncertainty(dim.numerical.uncertainty.get());
        dmap.insert({unit.first, {value.get_value(0), uncertainty.get_value(0), dim.physical}});
      }
      // dmap.insert({unit.first, {dim.numerical.estimate[0], dim.numerical.uncertainty[0], dim.physical}});
    }
  }
}

inline void solve_quantities(std::stringstream& ss, puq::DimensionMapType& dmap, puq::UnitSolver& solver) {
  std::cout << "Solving " << puq::UnitSystem::Data->SystemAbbrev << " system" << '\n';
  for (const auto& quant : puq::UnitSystem::Data->QuantityList) {
    // solve a quantity definition
    puq::UnitAtom atom = solver.solve(quant.second.definition);
    puq::Dimensions dim(atom.value.magnitude);
    for (auto bu : atom.value.baseunits) {
      solve_bu_prefix(dim, bu);
      if (!solve_bu_unit(dmap, dim, bu)) {
        throw puq::DimensionMapExcept(puq::UnitSystem::Data->SystemAbbrev + " quantity '" + quant.first +
                                      "' could not be constructued from a definition '" +
                                      quant.second.definition + "'. Missing unit: " + bu.unit);
      }
    }
    std::string symbol = std::string(puq::Symbols::quantity_start) + quant.first + std::string(puq::Symbols::quantity_end);
    add_line(ss, symbol, dim, puq::QuantityNames.at(quant.first));
    val::ArrayValue<double> value(dim.numerical.estimate.get());
    if (dim.numerical.uncertainty == nullptr) {
      dmap.insert({symbol, {value.get_value(0), 0., dim.physical}});
    } else {
      val::ArrayValue<double> uncertainty(dim.numerical.uncertainty.get());
      dmap.insert({symbol, {value.get_value(0), uncertainty.get_value(0), dim.physical}});
    }
    // dmap.insert({symbol, {dim.numerical.estimate[0], dim.numerical.uncertainty[0], dim.physical}});

    // solve a quantity IS conversion factor, if exists
    if (quant.second.sifactor != "") {
      atom = solver.solve(quant.second.sifactor);
      dim = puq::Dimensions(atom.value.magnitude);
      for (auto bu : atom.value.baseunits) {
        solve_bu_prefix(dim, bu);
        if (!solve_bu_unit(dmap, dim, bu)) {
          throw puq::DimensionMapExcept(puq::UnitSystem::Data->SystemAbbrev + " quantity '" + quant.first +
                                        "' could not be constructued from a definition '" +
                                        quant.second.definition + "'. Missing unit: " + bu.unit);
        }
      }
      // account for the conversion from MGS to MKS dimensions
      if (exponent_to_float(dim.physical[1]) != 0)
        dim.numerical *= math::pow(1e-3, exponent_to_float(dim.physical[1]));
      // clear physical dimensions to make conversion factors dimensionless
      std::fill(std::begin(dim.physical), std::end(dim.physical), 0);
      // register the conversion factor
      symbol = std::string(puq::Symbols::si_factor_start) + quant.first + std::string(puq::Symbols::si_factor_end);
      add_line(ss, symbol, dim, puq::QuantityNames.at(quant.first) + " SI factor");
      val::ArrayValue<double> value(dim.numerical.estimate.get());
      if (dim.numerical.uncertainty == nullptr) {
        dmap.insert({symbol, {value.get_value(0), 0., dim.physical}});
      } else {
        val::ArrayValue<double> uncertainty(dim.numerical.uncertainty.get());
        dmap.insert({symbol, {value.get_value(0), uncertainty.get_value(0), dim.physical}});
      }
      // dmap.insert({symbol, {dim.numerical.value[0], dim.numerical.uncertainty[0], dim.physical}});
    }
  }
}

void create_map(const std::string& file_header) {
  puq::UnitSolver solver;
  puq::DimensionMapType dmap;
  std::stringstream ss;
  // add dimension of base units
  for (const auto& d : puq::SystemData::_BASE_MAP) {
    dmap.insert(d);
    puq::Dimensions dim(d.second.magnitude, d.second.dimensions);
    std::string name;
    auto bu = puq::UnitSystem::Data->UnitList.at(d.first);
    add_line(ss, d.first, dim, bu.name);
  }
  // iterate over units until all are solved
  size_t nit = 0;
  size_t nmax = puq::UnitSystem::Data->UnitList.size();
  while (dmap.size() < nmax && nit < nmax) {
    nit++;
    solve_units(ss, dmap, solver);
  }
  // construct all quantities and their corresponding SI factors
  nmax += puq::UnitSystem::Data->QuantityList.size();
  solve_quantities(ss, dmap, solver);

  if (dmap.size() < nmax)
    throw puq::DimensionMapExcept("Dimension map is not complete: " + std::to_string(dmap.size()) + "/" + std::to_string(nmax));

  // Write dimension map table
  auto now = std::chrono::system_clock::now();
  std::time_t now_time = std::chrono::system_clock::to_time_t(now);
  std::ofstream fs;
  fs.open(file_header, std::ios::out | std::ios::trunc);
  fs << "/*" << '\n';
  fs << " * Do not modify this file!" << '\n';
  fs << " * This file can be updated using 'dmap' executable." << '\n';
  fs << " * " << '\n';
  fs << " * Unit system:  " << puq::UnitSystem::Data->SystemName << " (" << puq::UnitSystem::Data->SystemAbbrev << ")" << '\n';
  fs << " * Last update:  " << std::ctime(&now_time);
  fs << " * Code version: " << CODE_VERSION << '\n';
  fs << " * " << '\n';
  fs << " * Symbol legend:" << '\n';
  fs << " * ..    units" << '\n';
  fs << " * {..}  constants" << '\n';
  fs << " * {#..} collapsed constants" << '\n';
  fs << " * <..>  quantities" << '\n';
  fs << " * |..|  quantity SI conversion factors" << '\n';
  fs << " */" << '\n';
  fs << ss.str();
  fs.close();
}

class InputParser {
public:
  InputParser(int& argc, char** argv) {
    for (int i = 1; i < argc; ++i)
      this->tokens.push_back(std::string(argv[i]));
  }
  const std::deque<std::string> getCmdOption(const std::string& option, const int nstr = 1) const {
    std::vector<std::string>::const_iterator itr;
    std::deque<std::string> strs;
    itr = std::find(this->tokens.begin(), this->tokens.end(), option);
    for (int i = 0; i < nstr; i++) {
      if (itr != this->tokens.end() && ++itr != this->tokens.end()) {
        strs.push_back(*itr);
      }
    }
    return strs;
  }
  bool cmdOptionExists(const std::string& option) const {
    return std::find(this->tokens.begin(), this->tokens.end(), option) != this->tokens.end();
  }
  bool cmdEmpty() const {
    return this->tokens.size() == 0;
  }

private:
  std::vector<std::string> tokens;
};

int main(int argc, char* argv[]) {

  InputParser input(argc, argv);

  std::cout << "Generating dimension maps:" << '\n';
  for (auto sys : puq::SystemMap) {
    std::string file_header = "src/snt/puq/systems/dmaps/dmap_" + sys.second->SystemAbbrev + ".h";
    if (input.cmdOptionExists("-e")) {
      std::ofstream fs;
      fs.open(file_header, std::ios::out | std::ios::trunc);
      fs << "// Empty file";
      fs.close();
      std::cout << "Generating empty dmap file: " << file_header << '\n';
    } else {
      puq::UnitSystem us(sys.first);
      create_map(file_header);
      std::cout << "Generating dmap file: " << file_header << '\n';
    }
  }
}
