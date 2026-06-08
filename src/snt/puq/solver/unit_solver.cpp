#include "operator_array.h"
#include "operator_parentheses.h"

#include <memory>
#include <regex>
#include <snt/puq/solver/unit_solver.h>
#include <snt/puq/systems/unit_system.h>

namespace snt::puq {

    /**
     * @brief Initialise EXS solver with predefined operators and steps
     *
     * @return Instance of an EXS solver
     */
    inline exs::Solver<UnitAtom> init_solver() {
        exs::OperatorList operators;
        operators.append(exs::PARENTHESES_OPERATOR, std::make_shared<OperatorParentheses>());
        operators.append(ARRAY_OPERATOR, std::make_shared<OperatorArray>());
        operators.append(
            exs::MULTIPLY_OPERATOR, std::make_shared<exs::OperatorMultiply>(std::string(Symbols::multiply))
        );
        operators.append(exs::DIVIDE_OPERATOR, std::make_shared<exs::OperatorDivide>(std::string(Symbols::divide)));

        exs::StepList steps;
        steps.append(exs::GROUP_OPERATION, {exs::PARENTHESES_OPERATOR, ARRAY_OPERATOR});
        steps.append(exs::BINARY_OPERATION, {exs::MULTIPLY_OPERATOR, exs::DIVIDE_OPERATOR});

        return exs::Solver<UnitAtom>(operators, steps);
    }

    exs::Solver<UnitAtom> UnitSolver::solver = init_solver();

    std::unordered_map<std::string, UnitAtom> UnitSolver::cache;

    UnitAtom UnitSolver::eval(const std::string& expression) {
        if constexpr (Config::debug_unit_solver) {
            std::clog << "UNIT:  Solving: " << expression << '\n';
        }
        // add prefix in front of the expression to distinguish between unit systems
        std::string system = std::to_string(static_cast<int>(puq::UnitSystem::current.type)) + "_";
        // check if expression already exists
        auto it = cache.find(system + expression);
        if (it != cache.end()) {
            // if exists, take atom from cache
            UnitAtom ua = it->second;
            if constexpr (Config::debug_unit_solver) {
                std::clog << "UNIT:  Result:  " << ua.value.to_string() << '\n';
            }
            return ua;
        } else {
            // if not exists, calculate atom using expression solver
            UnitAtom ua = solver.solve((expression == "") ? "1" : expression);
            cache.insert_or_assign(system + expression, ua);
            if constexpr (Config::debug_unit_solver) {
                std::clog << "UNIT:  Result:  " << ua.value.to_string() << '\n';
            }
            return ua;
        }
    }

} // namespace snt::puq
