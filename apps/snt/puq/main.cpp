#include <snt/puq/lists.h>

#include "main.h"
#include "commands.h"
#include "displays.h"

using namespace snt;

namespace snt::cli {

  std::string puq_help() {
    return R"(
Scientific Numerical Tools v3 (SNT)
Module: Physical Units & Quantities (PUQ)

Usage:
  snt puq <command> [arguments]

Description:
  Work with physical units, quantities, and unit systems.

Commands:

  info [system] <expr>
      Show detailed information about an expression.

  list [system] <category>
      List available items.

      Categories:
        prefix   Unit prefixes
        base     Base units
        deriv    Derived units
        log      Logarithmic units
        temp     Temperature units
        const    Physical constants
        quant    Quantities
        sys      Unit systems

  solve [sys1] <expr1> [[sys2] <expr2> [q]]
      Evaluate and optionally convert expressions.

  convert [sys1] <expr1> [sys2] <expr2> [q]
      Convert between unit systems.

Options:
  -h, --help
      Show help.

Examples:
  snt puq info "23*kg*m2/s2"
  snt puq info IU "23*yd"

  snt puq list deriv
  snt puq list IU quant

  snt puq solve "23*cm + 3*m"
  snt puq solve US "23*ft + 3*yd"
  snt puq solve US "23*ft + 3*yd" SI "cm"

  snt puq convert "35*eV" "J"
  snt puq convert ESU "12*statA" SI "A" "I"
)";
  }
  
  void puq_main(ArgParser& argpar) {
  
    if (argpar.has("-h") || !argpar.hasArguments()) {
      std::cout << puq_help();
    }
    try {
      std::vector<std::string> values;
      std::deque<std::string> arguments;
      puq::UnitSystem us(puq::SystemType::SI);
      values = argpar.getAll("info");
      if (!values.empty()) {
	arguments = std::deque(values.begin(), values.end());
	change_system(us, arguments);
	display_info(arguments[0]);
      }
      values = argpar.getAll("solve");
      if (!values.empty()) {
	arguments = std::deque(values.begin(), values.end());
	change_system(us, arguments);
	command_solve(arguments);
      }
      values = argpar.getAll("arguments");
      if (!values.empty()) {
	arguments = std::deque(values.begin(), values.end());
	command_convert(arguments);
      }
      values = argpar.getAll("list");
      if (!values.empty()) {
	arguments = std::deque(values.begin(), values.end());
	change_system(us, arguments);
	if constexpr (!puq::Config::use_units_logarithmic) {
	  if (arguments[0] == "log") {
	    std::cout << puq::lists::logarithmic_units();
	    goto found_output;
	  }
	}
	if constexpr (!puq::Config::use_units_temperature) {
	  if (arguments[0] == "temp") {
	    std::cout << puq::lists::temperature_units();
	    goto found_output;
	  }
	}
	if (arguments[0] == "prefix")
	  std::cout << puq::lists::prefixes();
	else if (arguments[0] == "base")
	  std::cout << puq::lists::base_units();
	else if (arguments[0] == "deriv")
	  std::cout << puq::lists::derived_units();
	else if (arguments[0] == "const")
	  std::cout << puq::lists::constants();
	else if (arguments[0] == "quant")
	  std::cout << puq::lists::quantities();
	else if (arguments[0] == "sys")
	  std::cout << puq::lists::unit_systems();
	else {
	  display_lists(arguments);
	}
      found_output:;
      } else if (argpar.has("list")) {
	std::deque<std::string> arguments;
	display_lists(arguments);
      }
    } catch (std::exception& e) {
      std::cout << e.what() << '\n';
    }
  }

}
