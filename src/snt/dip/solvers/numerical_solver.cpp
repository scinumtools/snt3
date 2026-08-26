#include <snt/dip/exceptions.h>
#include <snt/dip/solvers/numerical_atom.h>
#include <snt/dip/solvers/numerical_solver.h>

namespace snt::dip {

    exs::Solver<NumericalAtom, NumericalSettings> NumericalSolver::solver = [] {
        exs::OperatorList operators;
        operators.append(
            exs::SINUS_OPERATOR, std::make_shared<exs::OperatorSinus>(exs::OperatorGroupSybols{"sin", "( ", " )", ", "})
        );
        operators.append(
            exs::COSINUS_OPERATOR,
            std::make_shared<exs::OperatorCosinus>(exs::OperatorGroupSybols{"cos", "( ", " )", ", "})
        );
        operators.append(
            exs::TANGENS_OPERATOR,
            std::make_shared<exs::OperatorTangens>(exs::OperatorGroupSybols{"tan", "( ", " )", ", "})
        );
        operators.append(
            exs::CUBIC_ROOT_OPERATOR,
            std::make_shared<exs::OperatorCubicRoot>(exs::OperatorGroupSybols{"cbrt", "( ", " )", ", "})
        );
        operators.append(
            exs::SQUARE_ROOT_OPERATOR,
            std::make_shared<exs::OperatorSquareRoot>(exs::OperatorGroupSybols{"sqrt", "( ", " )", ", "})
        );
        // operators.append(
        //     exs::POWER_BASE_OPERATOR,
        //     std::make_shared<exs::OperatorPowerBase>("powb( ", " )"));
        // operators.append(
        //     exs::LOGARITHM_BASE_OPERATOR,
        //     std::make_shared<exs::OperatorLogarithmBase>("logb( ", " )"));
        operators.append(
            exs::LOGARITHM_10_OPERATOR,
            std::make_shared<exs::OperatorLogarithm10>(exs::OperatorGroupSybols{"log10", "( ", " )", ", "})
        );
        operators.append(
            exs::LOGARITHM_OPERATOR,
            std::make_shared<exs::OperatorLogarithm>(exs::OperatorGroupSybols{"log", "( ", " )", ", "})
        );
        operators.append(
            exs::EXPONENT_OPERATOR,
            std::make_shared<exs::OperatorExponent>(exs::OperatorGroupSybols{"exp", "( ", " )", ", "})
        );
        operators.append(
            exs::PARENTHESES_OPERATOR,
            std::make_shared<exs::OperatorParentheses>(exs::OperatorGroupSybols{"", "( ", " )", ", "})
        );
        operators.append(exs::POWER_OPERATOR, std::make_shared<exs::OperatorPower>(" ** "));
        operators.append(exs::MULTIPLY_OPERATOR, std::make_shared<exs::OperatorMultiply>(" * "));
        operators.append(exs::DIVIDE_OPERATOR, std::make_shared<exs::OperatorDivide>(" / "));
        operators.append(exs::ADD_OPERATOR, std::make_shared<exs::OperatorAdd>(" +"));
        operators.append(exs::SUBTRACT_OPERATOR, std::make_shared<exs::OperatorSubtract>(" -"));

        exs::StepList steps;
        steps.append(
            exs::GROUP_OPERATION,
            {exs::EXPONENT_OPERATOR,
             exs::LOGARITHM_OPERATOR,
             exs::LOGARITHM_10_OPERATOR,
             // exs::LOGARITHM_BASE_OPERATOR, exs::POWER_BASE_OPERATOR,
             exs::SQUARE_ROOT_OPERATOR,
             exs::CUBIC_ROOT_OPERATOR,
             exs::SINUS_OPERATOR,
             exs::COSINUS_OPERATOR,
             exs::TANGENS_OPERATOR}
        );
        steps.append(exs::GROUP_OPERATION, {exs::PARENTHESES_OPERATOR});
        steps.append(exs::UNARY_OPERATION, {exs::ADD_OPERATOR, exs::SUBTRACT_OPERATOR});
        steps.append(exs::BINARY_OPERATION, {exs::POWER_OPERATOR});
        steps.append(exs::BINARY_OPERATION, {exs::MULTIPLY_OPERATOR, exs::DIVIDE_OPERATOR});
        steps.append(exs::BINARY_OPERATION, {exs::ADD_OPERATOR, exs::SUBTRACT_OPERATOR});

        return exs::Solver<NumericalAtom, NumericalSettings>(operators, steps);
    }();

    NumericalSolver::NumericalSolver(Environment& env, Path current) {

        NumericalSettings settings = {{}, &env, current};

        solver.set_settings(settings);
    }

    ValueNodeData NumericalSolver::eval(const std::string& expression, const std::string& units) {
        if (expression.empty())
            throw dip::SolverException(
                "Empty expression",
                "The numerical expression is empty; at least one operand or operation is required.",
                "Provide a valid numerical expression.",
                __FILE__,
                __LINE__
            );
        NumericalAtom ua = solver.eval(expression);

        // convert units if necessary
        if (ua.value.units && !units.empty()) {
            puq::Quantity quantity = std::move(ua.value.value) * (*ua.value.units);
            quantity = quantity.convert(units);
            ua.value.value = std::move(quantity.measurement.result.estimate);
            ua.value.units = puq::Quantity(units);
        } else if (ua.value.units) {
            if (ua.value.units->measurement.baseunits.has_dimensions())
                throw dip::UnitException(
                    "Dimension mismatch",
                    "The quantity is dimensionless, but a quantity with physical dimension `" + units +
                        "` is required.",
                    "Check the units of the input quantity.",
                    __FILE__,
                    __LINE__
                );
            else // converting nondimensional quantity into an empty 'units' string
                ua.value.units = std::nullopt;
        } else if (!units.empty()) {
            throw dip::UnitException(
                "Dimension mismatch",
                "The quantity has physical dimensions `" + ua.value.units->to_string() +
                    "`, but no physical dimensions are required.",
                "Check the units of the input quantity.",
                __FILE__,
                __LINE__
            );
        }

        return ValueNodeData({std::move(ua.value.value), std::move(ua.value.units)});
    }

} // namespace snt::dip
