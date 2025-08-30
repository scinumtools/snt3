#include "main.h"

#include <deque>
#include <iostream>
#include <limits>
#include <map>
#include <sstream>
#include <stdexcept>

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

std::string get_expression(std::deque<std::string>& convert) {
  if (convert.size()) {
    std::string expr = convert.front();
    convert.pop_front();
    return expr;
  } else {
    return "";
  }
}

puq::SystemType get_system(std::deque<std::string>& convert) {
  for (auto sys : puq::SystemMap) {
    if (sys.second->SystemAbbrev == convert.front()) {
      convert.pop_front();
      return sys.first;
    }
  }
  return puq::SystemType::NONE;
}

void change_system(puq::UnitSystem& us, std::deque<std::string>& convert) {
  puq::SystemType system = get_system(convert);
  if (system != puq::SystemType::NONE)
    us.change(system);
}

void convert_quantity(puq::Quantity& q, std::deque<std::string>& convert) {
  puq::SystemType sys2 = get_system(convert);
  std::string expr2 = get_expression(convert);
  std::string quant = get_expression(convert);
  if (quant == "") {
    if (sys2 == puq::SystemType::NONE)
      q = q.convert(expr2);
    else
      q = q.convert(expr2, sys2);
  } else {
    if (sys2 == puq::SystemType::NONE)
      q = q.convert(expr2, puq::UnitSystem::System, quant);
    else {
      q = q.convert(expr2, sys2, quant);
    }
  }
}

void solve_expression(std::deque<std::string>& convert) {
  std::string expr1 = get_expression(convert);
  puq::Calculator calc;
  puq::Quantity q = calc.solve(expr1).value;
  if (convert[0] != "")
    convert_quantity(q, convert);
  std::cout << q.to_string() << '\n';
}

void convert_units(std::deque<std::string>& convert) {
  puq::SystemType sys1 = get_system(convert);
  std::string expr1 = get_expression(convert);
  puq::Quantity q;
  if (sys1 == puq::SystemType::NONE)
    q = puq::Quantity(expr1);
  else
    q = puq::Quantity(expr1, sys1);
  convert_quantity(q, convert);
  std::cout << q.to_string() << '\n';
}

int main(int argc, char* argv[]) {
  InputParser input(argc, argv);
  if (input.cmdOptionExists("-h") || input.cmdEmpty()) {
    std::cout << '\n';
    std::cout << "Physical Units and Quantities (PUQ)" << '\n';
    std::cout << '\n';
    std::cout << "puq -h                            display help" << '\n';
    std::cout << "puq -v                            display code version" << '\n';
    std::cout << "puq -i [s] <e>                    get information about an expression <e> in a unit system [s]" << '\n';
    std::cout << "puq -l [s] <l>                    display list <l>=prefix/base/deriv/log/temp/const/quant in a unit system [s], or all available unit systems <l>=sys" << '\n';
    std::cout << "puq -s [s1] <e1> [[s2] <e2> [q]]  solve an arithmetic unit expression <e1> in a unit system [s1], optionally convert into expression <e2> in a system [s2] as a quantity [q]" << '\n';
    std::cout << "puq -c [s1] <e1> [s2] <e2> [q]    convert expression <e1> in a unit system [s1] into expression <e2> in a system [s2] as a quantity [q]" << '\n';
    std::cout << '\n';
    std::cout << "Example of use:" << '\n';
    std::cout << '\n';
    std::cout << "puq -i \"23*kg*m2/s2\"" << '\n';
    std::cout << "puq -i IU \"23*yd\"" << '\n';
    std::cout << "puq -l deriv" << '\n';
    std::cout << "puq -l IU quant" << '\n';
    std::cout << "puq -s \"23*cm + 3*m\"" << '\n';
    std::cout << "puq -s US \"23*ft + 3*yd\"" << '\n';
    std::cout << "puq -s US \"23*ft + 3*yd\" SI \"cm\"" << '\n';
    std::cout << "puq -c \"35*eV\" \"J\"" << '\n';
    std::cout << "puq -c ESU \"12*statA\" SI \"A\" \"I\"" << '\n';
    std::cout << '\n';
  } else if (input.cmdOptionExists("-v")) {
    std::cout << CODE_VERSION << '\n';
  }
  try {
    std::deque<std::string> convert;
    puq::UnitSystem us(puq::SystemType::SI);
    convert = input.getCmdOption("-i", 2);
    if (!convert.empty()) {
      change_system(us, convert);
      display_info(convert[0]);
    }
    convert = input.getCmdOption("-s", 4);
    if (!convert.empty()) {
      change_system(us, convert);
      solve_expression(convert);
    }
    convert = input.getCmdOption("-c", 5);
    if (!convert.empty()) {
      convert_units(convert);
    }
    convert = input.getCmdOption("-l", 2);
    if (!convert.empty()) {
      change_system(us, convert);
      if (convert[0] == "prefix")
        std::cout << puq::lists::prefixes();
      else if (convert[0] == "base")
        std::cout << puq::lists::base_units();
      else if (convert[0] == "deriv")
        std::cout << puq::lists::derived_units();
#ifdef UNITS_LOGARITHMIC
      else if (convert[0] == "log")
        std::cout << puq::lists::logarithmic_units();
#endif
#ifdef UNITS_TEMPERATURES
      else if (convert[0] == "temp")
        std::cout << puq::lists::temperature_units();
#endif
      else if (convert[0] == "const")
        std::cout << puq::lists::constants();
      else if (convert[0] == "quant")
        std::cout << puq::lists::quantities();
      else if (convert[0] == "sys")
        std::cout << puq::lists::unit_systems();
      else {
        display_lists(convert);
      }
    } else if (input.cmdOptionExists("-l")) {
      std::deque<std::string> convert;
      display_lists(convert);
    }
  } catch (std::exception& e) {
    std::cout << e.what() << '\n';
  }
}
