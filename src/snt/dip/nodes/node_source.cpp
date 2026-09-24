#include "node_source.h"

#include "../parsers.h"

#include <array>
#include <fstream>
#include <snt/dip/dip.h>
#include <snt/dip/environment.h>
#include <snt/val/values.h>

namespace snt::dip {

    BaseNode::PointerType SourceNode::is_node(Parser& parser) {
        if (parser.kwd_source()) {
            parser.part_keyword();
            parser.part_equal();
            if (!parser.part_reference())
                parser.part_string();
            parser.part_comment();
            return std::make_shared<SourceNode>(parser);
        }
        return nullptr;
    }

    BaseNode::ListType SourceNode::parse(Environment& env) {
        // TODO: implement import of a source
        std::string source_file = value_raw.at(1);
        if (value_origin == ValueOrigin::Reference) {
            ValueNodeData data = env.request_node_data(value_raw.at(1));
            if (!data.value) {
                throw dip::SyntaxException(
                    "Undefined source reference",
                    "The requested source value reference `" + value_raw.at(1) + "` does not resolve to a value.",
                    "Ensure that the referenced node exists and has a defined scalar string value.",
                    __FILE__,
                    __LINE__,
                    line
                );
            }
            if (data.value->get_dtype() != core::DataType::String || data.value->get_size() != 1) {
                throw dip::SyntaxException(
                    "Invalid source reference value",
                    "A `$source` value reference must resolve to a scalar string value.",
                    "Reference a scalar `str` node containing the source file path.",
                    __FILE__,
                    __LINE__,
                    line
                );
            }
            const auto* source_path = dynamic_cast<const val::ArrayValue<std::string>*>(data.value.get());
            source_file = source_path->get_value(0);
        } else if (value_origin != ValueOrigin::String) {
            throw dip::SyntaxException(
                "Invalid source value reference",
                "A `$source` value injection must reference a node value using a `?` path.",
                "Use a node reference such as `{?source_path}` or provide a quoted source file path.",
                __FILE__,
                __LINE__,
                line
            );
        }
        EnvSource senv = parse_source(value_raw.at(0), source_file, line.source);
        env.sources.append(value_raw.at(0), senv);
        return {};
    }

} // namespace snt::dip
