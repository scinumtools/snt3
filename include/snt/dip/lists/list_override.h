#ifndef DIP_LIST_OVERRIDE_H
#define DIP_LIST_OVERRIDE_H

#include <map>
#include <optional>
#include <snt/dip/nodes/node_base.h>
#include <string>
#include <vector>

namespace snt::dip {

    class HierarchyList;

    /** Static classification of a terminal collection-item override path.
     * It does not evaluate the item or its child values.
     */
    struct OverrideItemTarget {
        enum class Status { Existing, Creatable, UnknownCollection, InvalidSelector, SchemaRequired, UnresolvedParent };
        Status status = Status::InvalidSelector;
        std::string resolved_path;
        std::vector<std::string> schemas;
    };

    OverrideItemTarget classify_override_item(const HierarchyList& hierarchy, const std::string& requested);

    /** Collected, exact-path value modifications applied before node evaluation. */
    class OverrideList {
      private:
        struct Entry {
            BaseNode::PointerType node;
            bool consumed = false;
        };
        std::map<std::string, Entry> entries_;

        // A terminal item group may create a new item or address an existing one.
        struct Addition {
            BaseNode::PointerType node;
            std::optional<size_t> parent;
            std::vector<BaseNode::PointerType> modifications;
            std::string resolved_path;
            bool has_children = false;
        };
        std::vector<Addition> additions_;
        std::string addition_path(size_t index) const;
        const Line& addition_line(size_t index) const;
        void activate_addition(size_t index, const std::string& resolved_path);
        void activate_concrete_descendants(size_t index);

      public:
        /** Register one modification; a repeated target is an error. */
        void append(const BaseNode::PointerType& node);
        /** Atomically collect a body, expanding indentation into fully qualified target paths. */
        void append(const BaseNode::ListType& nodes, size_t indent);
        /** Find a modification by fully qualified target path, or return nullptr. */
        BaseNode::PointerType find(const std::string& path) const;
        /** Mark a target consumed after successful application. */
        void consume(const std::string& path);
        /** Return all target paths not matched by a normal declaration. */
        std::vector<std::string> unresolved() const;
        /** Number of terminal item groups, in registration order. */
        size_t item_group_count() const;
        /** Return a new group for an absent item, or nullptr for an existing item. */
        BaseNode::PointerType materialize_item_group(size_t index, const HierarchyList& hierarchy);
    };

} // namespace snt::dip

#endif // DIP_LIST_OVERRIDE_H
