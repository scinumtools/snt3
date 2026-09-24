#include "node_unit.h"

#include "../parsers.h"

#include <array>
#include <fstream>
#include <snt/dip/dip.h>
#include <snt/dip/environment.h>

namespace snt::dip {

    BaseNode::PointerType UnitNode::is_node(Parser& parser) {
        if (parser.kwd_unit()) {
            // TODO: implement import of a unit
            parser.part_keyword();
            parser.part_equal();
            if (!parser.part_reference())
                parser.part_units(
                    '\0'
                ); // an empty space before the units is already taken by ' = ' sign, so we don't enforce it here
            parser.part_comment();
            return std::make_shared<UnitNode>(parser);
        }
        return nullptr;
    }

    BaseNode::ListType UnitNode::parse(Environment& env) {
        std::string definition = units_raw;
        if (value_origin == ValueOrigin::Reference) {
            ValueNodeData data = env.request_node_data(value_raw.at(1));
            if (!data.value) {
                throw dip::SyntaxException(
                    "Undefined unit reference",
                    "The requested unit value reference `" + value_raw.at(1) + "` does not resolve to a value.",
                    "Ensure that the referenced node exists and has a defined scalar numeric value.",
                    __FILE__,
                    __LINE__,
                    line
                );
            }
            if (!core::any(data.value->get_dtype() & (core::DataType::Integer | core::DataType::Float)) ||
                data.value->get_size() != 1) {
                throw dip::SyntaxException(
                    "Invalid unit reference value",
                    "A `$unit` value reference must resolve to a scalar integer or floating-point value.",
                    "Reference a scalar `int` or `float` node when defining a custom unit.",
                    __FILE__,
                    __LINE__,
                    line
                );
            }
            definition = data.value->to_string();
            if (data.units)
                definition += "*" + data.units->to_string();
        } else if (value_origin != ValueOrigin::Keyword) {
            throw dip::SyntaxException(
                "Invalid unit value reference",
                "A `$unit` value injection must reference a node value using a `?` path.",
                "Use a node reference such as `{?mass}` or provide a PUEL unit expression.",
                __FILE__,
                __LINE__,
                line
            );
        }
        EnvUnit senv = {value_raw.at(0), definition};
        env.units.append(value_raw.at(0), senv, line.source.name);
        return {};
    }

} // namespace snt::dip
