#include <algorithm>
#include <snt/exs/composition.h>
#include <snt/exs/exceptions.h>
#include <snt/exs/expression_scan.h>
#include <stdexcept>
#include <utility>

namespace snt::exs {
    namespace {
        struct Pending {
            bool atom = false;
            std::size_t node = 0;
            int operator_type = NONE_OPERATOR;
            std::size_t group_count = 0;
        };

        [[noreturn]] void invalid_graph(const std::string& expression) {
            throw ParserException(
                "Unable to build expression composition",
                "The expression could not be reduced to one composition node: `" + expression + "`.",
                "Check the expression and the configured operator steps.",
                __FILE__,
                __LINE__
            );
        }

        std::size_t append(
            CompositionGraph& graph,
            CompositionKind kind,
            std::string text,
            int type,
            std::optional<OperationType> operation = std::nullopt,
            std::vector<std::size_t> children = {}
        ) {
            const auto id = graph.nodes.size();
            graph.nodes.push_back({kind, std::move(text), type, operation, std::move(children)});
            return id;
        }

        bool included(const std::vector<int>& types, int type) {
            return std::find(types.begin(), types.end(), type) != types.end();
        }

        std::size_t build(
            const std::string& source, OperatorList& operators, const StepList& steps, CompositionGraph& graph
        ) {
            std::vector<Pending> tokens;
            const auto add_operand = [&](std::string value) {
                if (!value.empty())
                    tokens.push_back({true, append(graph, CompositionKind::Operand, std::move(value), NONE_OPERATOR)});
            };

            detail::scan_expression<true>(
                source,
                operators,
                add_operand,
                [&](const std::string& group) { tokens.push_back({true, build(group, operators, steps, graph)}); },
                [&](int type, std::size_t group_count) { tokens.push_back({false, 0, type, group_count}); }
            );

            for (const auto& [operation, types] : steps.steps) {
                if (operation == UNARY_OPERATION) {
                    // Prefix operators bind from the operand outward. A plus or minus
                    // preceded by an atom is left for the binary reduction step.
                    for (std::size_t i = tokens.size(); i-- > 0;) {
                        if (tokens[i].atom || !included(types, tokens[i].operator_type) || (i && tokens[i - 1].atom))
                            continue;
                        if (i + 1 >= tokens.size() || !tokens[i + 1].atom)
                            invalid_graph(source);
                        const auto* op = operators.select(tokens[i].operator_type);
                        const auto id = append(
                            graph,
                            CompositionKind::Operator,
                            op->name,
                            tokens[i].operator_type,
                            UNARY_OPERATION,
                            {tokens[i + 1].node}
                        );
                        tokens.erase(
                            tokens.begin() + static_cast<std::ptrdiff_t>(i),
                            tokens.begin() + static_cast<std::ptrdiff_t>(i + 2)
                        );
                        tokens.insert(tokens.begin() + static_cast<std::ptrdiff_t>(i), {true, id});
                    }
                    continue;
                }

                for (std::size_t i = 0; i < tokens.size();) {
                    if (tokens[i].atom || !included(types, tokens[i].operator_type)) {
                        ++i;
                        continue;
                    }
                    const auto* op = operators.select(tokens[i].operator_type);
                    std::size_t begin = i;
                    std::size_t end = i + 1;
                    CompositionKind kind = CompositionKind::Operator;
                    if (operation == GROUP_OPERATION) {
                        if (i < tokens[i].group_count)
                            invalid_graph(source);
                        begin = i - tokens[i].group_count;
                        kind = CompositionKind::Group;
                    } else if (operation == BINARY_OPERATION) {
                        if (i == 0 || i + 1 >= tokens.size())
                            invalid_graph(source);
                        begin = i - 1;
                        end = i + 2;
                    } else if (operation == TERNARY_OPERATION) {
                        if (i < 2 || i + 1 >= tokens.size())
                            invalid_graph(source);
                        begin = i - 2;
                        end = i + 2;
                    } else {
                        invalid_graph(source);
                    }
                    std::vector<std::size_t> children;
                    children.reserve(end - begin - 1);
                    for (std::size_t j = begin; j < end; ++j) {
                        if (j == i)
                            continue;
                        if (!tokens[j].atom)
                            invalid_graph(source);
                        children.push_back(tokens[j].node);
                    }
                    const auto id =
                        append(graph, kind, op->name, tokens[i].operator_type, operation, std::move(children));
                    tokens.erase(
                        tokens.begin() + static_cast<std::ptrdiff_t>(begin),
                        tokens.begin() + static_cast<std::ptrdiff_t>(end)
                    );
                    tokens.insert(tokens.begin() + static_cast<std::ptrdiff_t>(begin), {true, id});
                    i = begin + 1;
                }
            }
            if (tokens.size() != 1 || !tokens.front().atom)
                invalid_graph(source);
            return tokens.front().node;
        }
    } // namespace

    CompositionGraph build_composition_graph(
        const std::string& expression, OperatorList& operators, const StepList& steps
    ) {
        CompositionGraph graph;
        graph.root = build(expression, operators, steps, graph);
        return graph;
    }
} // namespace snt::exs
