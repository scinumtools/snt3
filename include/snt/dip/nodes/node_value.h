#ifndef DIP_NODE_VALUE_H
#define DIP_NODE_VALUE_H

#include "node_base.h"

#include <deque>
#include <optional>
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

      protected:
        struct OptionStruct {
            val::BaseValue::PointerType value;
            std::string value_raw;
            std::string units_raw;
        };
        core::DataType value_dtype;

      public:
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

        ValueNode() : constant(false) {};
        ValueNode(const ValueNode& other);
        ValueNode(const core::DataType vdt) : constant(false), value_dtype(vdt) {};
        ValueNode(const Parser& parser, const NodeDtype dt, const core::DataType vdt = core::DataType::None)
            : BaseNode(parser, dt), constant(false), value_dtype(vdt) {};
        ValueNode(
            const BaseNode::PointerType other, const NodeDtype dt, const core::DataType vdt = core::DataType::None
        );
        ValueNode(const Path& pth, const core::DataType vdt, const NodeDtype dt = NodeDtype::None);
        ValueNode(
            const Path& pth,
            val::BaseValue::PointerType val,
            const NodeDtype dt,
            std::optional<puq::Quantity> unt = std::nullopt
        );
        virtual ~ValueNode() = default;
        val::BaseValue::PointerType cast_value();
        val::BaseValue::PointerType cast_value(val::Array::StringType& value_input, const val::Array::ShapeType& shape);
        void set_value(val::BaseValue::PointerType value_input = nullptr);
        void set_units(const std::optional<puq::Quantity>& units_input = std::nullopt);
        virtual void modify_value(const BaseNode::PointerType& node, Environment& env);
        virtual bool set_property(PropertyType property, val::Array::StringType& values, std::string& units) override;
        virtual std::string to_string(const core::StringFormatType& format = core::StringFormatType()) const override {
            throw std::runtime_error("String represenation of the value node is not implemented");
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
