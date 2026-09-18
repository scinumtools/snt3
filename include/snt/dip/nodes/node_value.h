#ifndef DIP_NODE_VALUE_H
#define DIP_NODE_VALUE_H

#include "node_base.h"

#include <deque>
#include <optional>
#include <snt/dip/exceptions.h>
#include <snt/puq/quantity.h>

namespace snt::dip {

    /**
     * Forward declaration of the Environment
     */
    class Environment;

    /**
     * Set of node and value metadata
     */
    struct ValueMetadata {
        std::string description; ///< Value description
        std::string authors;     ///< Publication authors
        std::string title;       ///< Publication title
        std::string journal;     ///< Publication journal
        std::string year;        ///< Publication year
        std::string volume;      ///< Publication volume
        std::string issue;       ///< Publication issue
        std::string pages;       ///< Publication pages
        std::string doi;         ///< Publication DOI
        std::string url;         ///< Online URL of the publication
        std::string version;     ///< Publication version
        std::string created;     ///< Date of creation
        std::string modified;    ///< Date of modification
        std::string license;     ///< License of the value
    };

    /**
     * Structure that holds value node data
     *
     * This structure is used in solvers as a lightweight value node data
     */
    struct ValueNodeData {
        val::BaseValue::PointerType value;  ///< Node value
        std::optional<puq::Quantity> units; ///< Node quantity
    };

    class ValueNode : public BaseNode {
        virtual val::BaseValue::PointerType cast_scalar_value(const std::string& value_input) const = 0;
        virtual val::BaseValue::PointerType cast_array_value(
            const val::Array::StringType& value_inputs, const val::Array::ShapeType& shape
        ) const = 0;

      public:
        struct OptionStruct {
            val::BaseValue::PointerType value;
            std::string value_raw;
            std::string units_raw;
        };

        core::DataType value_dtype;
        using PointerType = std::shared_ptr<ValueNode>;
        using ListType = std::deque<ValueNode::PointerType>;

        // content
        val::BaseValue::PointerType value;
        std::optional<puq::Quantity> units;

        // directives
        val::Array::StringType tags;
        bool constant;
        std::string condition;
        std::vector<OptionStruct> options;
        std::string format;

        // metadata
        ValueMetadata metadata;

        /** Construct an empty value node. */
        ValueNode() : constant(false), value_dtype(core::DataType::None) {};
        /** Copy a value node.
         * @param other Node whose metadata and value are copied.
         */
        ValueNode(const ValueNode& other);
        /** Construct a value node with a declared value data type.
         * @param vdt Data type of the stored value.
         */
        ValueNode(const core::DataType vdt) : constant(false), value_dtype(vdt) {};
        /** Construct a value node from parser state.
         * @param parser Parser providing node metadata.
         * @param dt DIP node data type.
         * @param vdt Stored value data type.
         */
        ValueNode(const Parser& parser, const NodeDtype dt, const core::DataType vdt = core::DataType::None)
            : BaseNode(parser, dt), constant(false), value_dtype(vdt) {};
        /** Construct a value node by copying another node's value.
         * @param other Source node.
         * @param dt DIP node data type.
         * @param vdt Stored value data type.
         */
        ValueNode(
            const BaseNode::PointerType other, const NodeDtype dt, const core::DataType vdt = core::DataType::None
        );
        /** Construct a value node from an existing value node.
         * @param other Source node.
         * @param dt DIP node data type.
         */
        ValueNode(const ValueNode::PointerType other, const NodeDtype dt);
        /** Construct a value node at a path with a value data type.
         * @param pth Node path.
         * @param vdt Stored value data type.
         * @param dt DIP node data type.
         */
        ValueNode(const Path& pth, const core::DataType vdt, const NodeDtype dt = NodeDtype::None);
        /** Construct a value node at a path from a value and optional units.
         * @param pth Node path.
         * @param val Stored value.
         * @param dt DIP node data type.
         * @param unt Optional physical quantity units.
         */
        ValueNode(
            const Path& pth,
            val::BaseValue::PointerType val,
            const NodeDtype dt,
            std::optional<puq::Quantity> unt = std::nullopt
        );
        ~ValueNode() override = default;
        val::BaseValue::PointerType parse_function(
            Environment& env, const std::string& name, std::optional<std::string_view> units
        ) const;
        val::BaseValue::PointerType parse_reference(
            Environment& env, std::string query, std::optional<std::string_view> units, ValueOrigin origin
        ) const;
        val::BaseValue::PointerType parse_expression(
            Environment& env,
            const std::string& expression,
            std::optional<std::string_view> units,
            const NodeDtype ntype
        ) const;
        val::BaseValue::PointerType cast_value() const;
        val::BaseValue::PointerType cast_value(
            const val::Array::StringType& value_input, const val::Array::ShapeType& shape
        ) const;
        /** Replace the node's stored value and update its value metadata.
         * @param value_input Owned VAL value; `nullptr` clears the current value.
         */
        void set_value(val::BaseValue::PointerType value_input = nullptr);
        /** Replace the physical quantity metadata associated with the node.
         * @param units_input Quantity metadata to assign; `std::nullopt` removes the units.
         */
        void set_units(const std::optional<puq::Quantity>& units_input = std::nullopt);
        virtual void modify_value(const BaseNode::PointerType& node, Environment& env);
        bool set_property(PropertyType property, val::Array::StringType& values, std::string& units) override;
        std::string to_string(const core::StringFormatType& format = core::StringFormatType()) const override {
            throw dip::MissingException(
                "String representation of this value node is not implemented.", __FILE__, __LINE__, line
            );
            return "";
        };
        void validate_constant() const;
        void validate_definition() const;
        void validate_condition(Environment& env) const;
        virtual void validate_options() const;
        virtual void validate_format() const;

      private:
        void validate_dimensions() const;
    };

} // namespace snt::dip

#endif // DIP_NODE_VALUE_H
