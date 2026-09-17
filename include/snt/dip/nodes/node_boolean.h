#ifndef DIP_NODE_BOOLEAN_H
#define DIP_NODE_BOOLEAN_H

#include <snt/dip/nodes/node_base.h>
#include <snt/dip/nodes/node_value.h>

namespace snt::dip {

    /** DIPL value node containing boolean data. */
    class BooleanNode : public ValueNode {
        val::BaseValue::PointerType cast_scalar_value(const std::string& value_input) const override;
        val::BaseValue::PointerType cast_array_value(
            const val::Array::StringType& value_inputs, const val::Array::ShapeType& shape
        ) const override;

      public:
        /** Return a Boolean node when the parser is positioned at a Boolean literal.
         * @param parser Parser state to inspect.
         */
        static ValueNode::PointerType is_node(Parser& parser);
        /** Copy a Boolean node. @param other Node to copy. */
        BooleanNode(const BooleanNode& other);
        /** Construct a Boolean node at a path from an existing value. */
        BooleanNode(const Path& pth, val::BaseValue::PointerType val)
            : ValueNode(pth, std::move(val), NodeDtype::Boolean) {};
        /** Construct a Boolean node from parser input. */
        BooleanNode(Parser& parser) : ValueNode(parser, NodeDtype::Boolean, core::DataType::Boolean) {};
        BaseNode::ListType parse(Environment& env) override;
        BaseNode::PointerType clone(const Path& pth, std::optional<size_t> indent = std::nullopt) const override;
        void validate_options() const override;
        std::string to_string(const core::StringFormatType& format = core::StringFormatType()) const override;
    };

} // namespace snt::dip

#endif // DIP_NODE_BOOLEAN_H
