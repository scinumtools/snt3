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

} // namespace snt::dip
