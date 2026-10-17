#include "node_property.h"

#include <snt/dip/exceptions.h>

namespace snt::dip {

    BaseNode::PointerType PropertyNode::is_node(Parser& parser) {
        PropertyType ptype = PropertyType::None;
        // determine property type
        parser.kwd_property(ptype);
        // parse rest of the parts
        if (ptype != PropertyType::None) {
            if (ptype == PropertyType::MetadataConstraint) {
                if (!parser.part_metadata_constraint())
                    throw dip::SyntaxException(
                        "Invalid metadata constraint",
                        "The !metadata directive requires a metadata name and either `required` or `nonempty`.",
                        "Write `!metadata authors required` or `!metadata url nonempty`.",
                        __FILE__,
                        __LINE__,
                        parser.line
                    );
            } else {
                parser.part_value();
            }
            if (ptype == PropertyType::Labels &&
                (parser.value_origin != ValueOrigin::Array || parser.value_shape.size() != 1))
                throw dip::SyntaxException(
                    "Invalid labels",
                    "The `!labels` directive requires a one-dimensional list.",
                    "Provide one label for each option in a single list.",
                    __FILE__,
                    __LINE__,
                    parser.line
                );
            parser.part_units();
            parser.part_comment();
            return std::make_shared<PropertyNode>(parser, ptype);
        }
        return nullptr;
    }

} // namespace snt::dip
