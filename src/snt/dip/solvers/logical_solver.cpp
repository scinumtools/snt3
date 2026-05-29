#include <snt/dip/solvers/logical_atom.h>
#include <snt/dip/solvers/logical_operators.h>
#include <snt/dip/solvers/logical_solver.h>

namespace snt::dip {

    LogicalSolver::LogicalSolver(Environment& env) {

        LogicalSettings settings = {{}, &env};

        exs::OperatorList operators;
        operators.append(
            exs::PARENTHESES_OPERATOR,
            std::make_shared<exs::OperatorParentheses>(exs::OperatorGroupSybols("", "( ", " )", ", "))
        );
        operators.append(dip::DEFINED_OPERATOR, std::make_shared<dip::OperatorDefined>());
        operators.append(dip::NOT_DEFINED_OPERATOR, std::make_shared<dip::OperatorNotDefined>());
        operators.append(exs::EQUAL_OPERATOR, std::make_shared<exs::OperatorEqual>(" == "));
        operators.append(exs::NOT_EQUAL_OPERATOR, std::make_shared<exs::OperatorNotEqual>(" != "));
        operators.append(
            exs::NOT_OPERATOR,
            std::make_shared<exs::OperatorNot>("~")
        ); // needs to be after
           // NOT_EQUAL
        operators.append(exs::LESS_EQUAL_OPERATOR, std::make_shared<exs::OperatorLessEqual>(" <= "));
        operators.append(exs::GREATER_EQUAL_OPERATOR, std::make_shared<exs::OperatorGreaterEqual>(" >= "));
        operators.append(exs::LESS_OPERATOR, std::make_shared<exs::OperatorLess>(" < "));
        operators.append(exs::GREATER_OPERATOR, std::make_shared<exs::OperatorGreater>(" > "));
        operators.append(exs::AND_OPERATOR, std::make_shared<exs::OperatorAnd>(" && "));
        operators.append(exs::OR_OPERATOR, std::make_shared<exs::OperatorOr>(" || "));

        exs::StepList steps;
        steps.append(exs::GROUP_OPERATION, {dip::DEFINED_OPERATOR, dip::NOT_DEFINED_OPERATOR});
        steps.append(exs::GROUP_OPERATION, {exs::PARENTHESES_OPERATOR});
        steps.append(
            exs::BINARY_OPERATION,
            {exs::LESS_EQUAL_OPERATOR, exs::GREATER_EQUAL_OPERATOR, exs::LESS_OPERATOR, exs::GREATER_OPERATOR}
        );
        steps.append(exs::BINARY_OPERATION, {exs::EQUAL_OPERATOR, exs::NOT_EQUAL_OPERATOR});
        steps.append(exs::UNARY_OPERATION, {exs::NOT_OPERATOR});
        steps.append(exs::BINARY_OPERATION, {exs::AND_OPERATOR});
        steps.append(exs::BINARY_OPERATION, {exs::OR_OPERATOR});

        solver = std::make_unique<exs::Solver<LogicalAtom, LogicalSettings>>(operators, steps, settings);
    }

    ValueNodeData LogicalSolver::eval(const std::string& expression) {
        if (expression.empty())
            throw std::runtime_error("Logical expression cannot be empty");
        LogicalAtom ua = solver->solve(expression);
        return ValueNodeData({std::move(ua.value.value), std::move(ua.value.units)});
    }

} // namespace snt::dip
