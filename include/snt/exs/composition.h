#ifndef SNT_EXS_COMPOSITION_H
#define SNT_EXS_COMPOSITION_H

#include <cstddef>
#include <optional>
#include <snt/exs/operator_list.h>
#include <snt/exs/step_list.h>
#include <string>
#include <vector>

namespace snt::exs {

    enum class CompositionKind { Operand, Operator, Group };

    /** A structural expression node. Child IDs refer to nodes in the same graph. */
    struct CompositionNode {
        CompositionKind kind;
        std::string text; ///< Operand text or registered operator name.
        int operator_type = NONE_OPERATOR;
        std::optional<OperationType> operation; ///< Absent for operands.
        std::vector<std::size_t> children;      ///< Source order; operands have none.
    };

    /** Per-expression structure, independent of atom types and evaluated values. */
    struct CompositionGraph {
        std::vector<CompositionNode> nodes;
        std::size_t root = 0;
    };

    /** Build a graph with the solver's configured operators and reduction order.
     * This structural pass does not evaluate atoms. Call through
     * Solver::eval(expression, &graph) when evaluation must succeed first.
     */
    CompositionGraph build_composition_graph(
        const std::string& expression, OperatorList& operators, const StepList& steps
    );

} // namespace snt::exs

#endif // SNT_EXS_COMPOSITION_H
