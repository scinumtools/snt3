#ifndef DIP_NODE_DEFERRED_H
#define DIP_NODE_DEFERRED_H

#include <snt/dip/nodes/node_value.h>

namespace snt::dip {

    /**
     * Deferred value node
     *
     * This node is used as a value node placeholder for a non-value node declaration, e.g. table nodes in schemas.
     * If node with the same name and type is later fully declared and parsed, this placeholder node should is removed
     * from the final node list. Otherwise, it will trigger the `undefined value` error.
     */
    class DeferredNode : public ValueNode {
        /**
         * @copydoc ValueNode::cast_scalar_value
         */
        val::BaseValue::PointerType cast_scalar_value(const std::string& value_input) const override;

        /**
         * @copydoc ValueNode::cast_array_value
         */
        val::BaseValue::PointerType cast_array_value(
            const val::Array::StringType& value_inputs, const val::Array::ShapeType& shape
        ) const override;

      public:
        BaseNode::PointerType node; ///< Pointer of a declared non-value node

        /**
         * Node constructor
         *
         * @param nd Pointer of a declared non-value node
         */
        DeferredNode(BaseNode::PointerType nd);

        /**
         * @copydoc BaseNode::parse
         */
        BaseNode::ListType parse(Environment& env) override;
    };

} // namespace snt::dip

#endif // DIP_NODE_DEFERRED_H
