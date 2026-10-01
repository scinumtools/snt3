#ifndef SNT_DIP_DEPENDENCY_GRAPH_H
#define SNT_DIP_DEPENDENCY_GRAPH_H

#include <optional>
#include <snt/core/exceptions.h>
#include <snt/exs/composition.h>
#include <string>
#include <string_view>
#include <unordered_set>
#include <vector>

namespace snt::dip {

    /** An evaluated read of a DIP value. IDs use ?path or source?path. */
    struct DependencyEdge {
        std::string target;
        std::string request;
        std::string operand; ///< EXS operand text when this read came from an expression.
    };

    enum class DependencyEventKind { Value, Condition, Decision };

    /** One evaluation of a value, validation condition, or branch decision. */
    struct DependencyEvent {
        std::string owner;
        DependencyEventKind kind = DependencyEventKind::Value;
        std::optional<core::SourceLocation> location;
        std::vector<std::string> controlled_by; ///< Active branch decision IDs for a value event.
        std::string expression;
        std::optional<exs::CompositionGraph> composition;
        std::vector<DependencyEdge> reads;
    };

    /** Retains evaluation order and provides effective forward and reverse links. */
    class DependencyGraph {
      public:
        std::vector<DependencyEvent> events;

        /** Latest event for this owner and kind. */
        const DependencyEvent* latest(std::string_view owner, DependencyEventKind kind) const {
            for (auto it = events.rbegin(); it != events.rend(); ++it)
                if (it->owner == owner && it->kind == kind)
                    return &*it;
            return nullptr;
        }

        /** Effective reads of the value, excluding superseded declarations. */
        std::vector<DependencyEdge> dependencies(std::string_view owner) const {
            const auto* event = latest(owner, DependencyEventKind::Value);
            return event ? event->reads : std::vector<DependencyEdge>{};
        }

        /** Value nodes whose current evaluation read the target. */
        std::vector<std::string> referenced_by(std::string_view target) const {
            std::vector<std::string> result;
            std::unordered_set<std::string> seen;
            for (auto it = events.rbegin(); it != events.rend(); ++it) {
                if (it->kind != DependencyEventKind::Value || !seen.insert(it->owner).second)
                    continue;
                for (const auto& edge : it->reads)
                    if (edge.target == target) {
                        result.push_back(it->owner);
                        break;
                    }
            }
            return result;
        }
    };

} // namespace snt::dip

#endif
