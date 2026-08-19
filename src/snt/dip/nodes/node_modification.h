#ifndef DIP_NODE_MODIFICATION_H
#define DIP_NODE_MODIFICATION_H

#include <snt/dip/nodes/node_base.h>

namespace snt::dip {

    class ModificationNode : public BaseNode {
      public:
        static BaseNode::PointerType is_node(Parser& parser);

        /**
         * Modificatio constructor
         *
         * @param parser Parsed node object
         */
        ModificationNode(Parser& parser) : BaseNode(parser, NodeDtype::Modification) {};

        /**
         * @copydoc BaseNode::parse
         */
        BaseNode::ListType parse(Environment& env) override;
    };

} // namespace snt::dip

#endif // DIP_NODE_MODIFICATION_H
