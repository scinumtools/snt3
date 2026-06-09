// #include <snt/puq/lists.h>
#include "argparser.h"
#include "main.h"
#include "snt/api/puq_convert.h"
#include "snt/api/puq_eval.h"
#include "snt/api/puq_info.h"
#include "snt/api/puq_list.h"
// #include "puq/commands.h"
// #include "puq/displays.h"

using namespace snt;

std::string help_puq() {
    return R"(
Scientific Numerical Tools v3 (SNT)
Module: Physical Units & Quantities (PUQ)

Usage:
  snt puq <command> [arguments]

Description:
  Work with physical units, quantities, and unit systems.

Commands:

  info [-s system] <expr>
      Show detailed information about an expression.

  list [-s system] <category>
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

  eval [-s sys1] <expr1> [[-S sys2] -U <expr2> [-Q q]]
      Solve and optionally convert expressions.

  convert [-s sys1] <expr1> [-S sys2] -U <expr2> [-Q q]
      Convert between unit systems.

Options:
  -h, --help
      Show help.
  -s, --input-system
      Unit system of original units
  -S, --output-system
      Unit system of target units
  -U, --output-units
      Target units
  -Q, --output-quantity
      Conversion quantity

Examples:
  snt puq info "23*kg*m2/s2"
  snt puq info "23*yd" -s IU

  snt puq list deriv
  snt puq list quant -s IU 

  snt puq eval "23*cm + 3*m"
  snt puq eval "23*ft + 3*yd" -s US
  snt puq eval "23*ft + 3*yd" -s US -S SI -U "cm"

  snt puq convert "35*eV" "J"
  snt puq convert "12*statA" "A" -s ESU -S SI -Q "I"
)";
}

void module_puq(ArgParser& argpar) {

    if (argpar.hasKeyword("-h") || argpar.numPositional() <= 2) {
        std::cout << help_puq();
        exit(0);
    }

    try {
        std::string command = argpar.getPositionalValue(1);
        std::string argument = argpar.getPositionalValue(2);
        std::vector<std::string> arguments;
        // puq::UnitSystem us(puq::SystemType::SI);
        if (command == "info") {
            api::PUQInfo cmd(argument);
            arguments = argpar.getKeywordValues("-s", "--system");
            if (!arguments.empty()) {
                cmd.argument_input_system(arguments[0]);
            }
            std::cout << cmd.execute() << '\n';
        }
        if (command == "eval") {
            api::PUQEval cmd(argument);
            arguments = argpar.getKeywordValues("-s", "--input-system");
            if (!arguments.empty()) {
                cmd.argument_input_system(arguments[0]);
            }
            arguments = argpar.getKeywordValues("-S", "--output-system");
            if (!arguments.empty()) {
                cmd.argument_output_system(arguments[0]);
            }
            arguments = argpar.getKeywordValues("-U", "--output-units");
            if (!arguments.empty()) {
                cmd.argument_output_units(arguments[0]);
            }
            arguments = argpar.getKeywordValues("-Q", "--output-quantity");
            if (!arguments.empty()) {
                cmd.argument_output_quantity(arguments[0]);
            }
            std::cout << cmd.execute() << '\n';
        }
        if (command == "convert") {
            std::string argument2 = argpar.getPositionalValue(3);
            api::PUQConvert cmd(argument, argument2);
            arguments = argpar.getKeywordValues("-s", "--input-system");
            if (!arguments.empty()) {
                cmd.argument_input_system(arguments[0]);
            }
            arguments = argpar.getKeywordValues("-S", "--output-system");
            if (!arguments.empty()) {
                cmd.argument_output_system(arguments[0]);
            }
            arguments = argpar.getKeywordValues("-Q", "--output-quantity");
            if (!arguments.empty()) {
                cmd.argument_output_quantity(arguments[0]);
            }
            std::cout << cmd.execute() << '\n';
        }
        if (command == "list") {
            api::PUQList cmd(argument);
            arguments = argpar.getKeywordValues("-s", "--system");
            if (!arguments.empty()) {
                cmd.argument_system(arguments[0]);
            }
            std::cout << cmd.execute() << '\n';
        }
    } catch (std::exception& e) {
        std::cout << e.what() << '\n';
    }
}
