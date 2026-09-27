#ifndef DIP_LIST_OVERRIDE_H
#define DIP_LIST_OVERRIDE_H

#include <map>
#include <snt/dip/nodes/node_base.h>
#include <string>
#include <vector>

namespace snt::dip {

    /** Collected, exact-path value modifications applied before node evaluation. */
    class OverrideList {
      private:
        struct Entry {
            BaseNode::PointerType node;
            bool consumed = false;
        };
        std::map<std::string, Entry> entries_;

      public:
        /** Register one modification; a repeated target is an error. */
        void append(const BaseNode::PointerType& node);
        /** Find a modification by fully qualified target path, or return nullptr. */
        BaseNode::PointerType find(const std::string& path) const;
        /** Mark a target consumed after successful application. */
        void consume(const std::string& path);
        /** Return all target paths not matched by a normal declaration. */
        std::vector<std::string> unresolved() const;
    };

} // namespace snt::dip

#endif // DIP_LIST_OVERRIDE_H
