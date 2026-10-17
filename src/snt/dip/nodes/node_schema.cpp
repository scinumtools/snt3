#include "node_schema.h"

#include "../parsers.h"

#include <array>
#include <fstream>
#include <snt/dip/dip.h>
#include <snt/dip/environment.h>

namespace snt::dip {

    BaseNode::PointerType SchemaNode::is_node(Parser& parser) {
        if (parser.kwd_schema()) {
            parser.part_keyword();
            parser.part_comment();
            return std::make_shared<SchemaNode>(parser);
        }
        return nullptr;
    }

    BaseNode::ListType SchemaNode::parse(Environment& env) {
        // TODO: implement import of a schema
        // TODO: implement injection of a schema file
        // TODO: implement injection a text file
        // EnvSchema senv = parse_schema(value_raw.at(0), line.source);
        // env.schemas.append(value_raw.at(0), senv);
        return {};
    }

    bool SchemaNode::set_property(PropertyType property, val::Array::StringType& values, std::string& units) {
        (void)units;
        if (property == PropertyType::MetadataConstraint) {
            if (!BaseNode::set_property(property, values, units))
                return false;
            for (auto constraint : metadata_constraints) {
                constraint.schema_name = value_raw.empty() ? std::string{} : value_raw.front();
                instance_metadata_constraints.push_back(std::move(constraint));
            }
            metadata_constraints.clear();
            return true;
        }
        return metadata.set_property(property, values);
    }

} // namespace snt::dip
