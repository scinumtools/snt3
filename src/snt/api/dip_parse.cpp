#include "dip_parse.h"

#include <iostream>
#include <snt/api/exceptions.h>
#include <sstream>

namespace snt::api {

    void DIPParse::argument_add(const std::string& add_type, const std::vector<std::string>& add_values) {
        if (add_type == "file" && add_values.size() == 1) {
            dip.add_file(add_values[0]);
        } else if (add_type == "string" && add_values.size() == 1) {
            dip.add_string(add_values[0]);
        } else if (add_type == "source" && add_values.size() == 2) {
            dip.add_source(add_values[0], add_values[1]);
        } else if (add_type == "unit" && add_values.size() == 2) {
            dip.add_unit(add_values[0], add_values[1]);
        } else {
            throw api::ArgumentException(
                "Invalid add argument",
                "The input type `" + add_type + "` was not recognized or has an invalid number of values.",
                "Use `file` or `string` with one value, or `source` or `unit` with two values.",
                __FILE__,
                __LINE__
            );
        }
    }

    void DIPParse::argument_request(const std::string& path) {
        request = path;
    }

    void DIPParse::argument_tags(const std::vector<std::string>& list) {
        tags = list;
    }

    void DIPParse::argument_print() {
        print = PrintOptions::ALL;
    }

    std::string DIPParse::execute() {
        // parse code
        dip::Environment env = dip.parse();

        // request nodes
        dip::ValueNode::ListType vnodes;
        if (request.empty()) {
            vnodes = env.request_group("?", dip::RequestType::Reference, tags);
        } else {
            vnodes = env.request_group("?" + request, dip::RequestType::Reference, tags);
        }

        // print if required
        std::stringstream ss;
        if (print == PrintOptions::ALL) {
            for (const auto& node : vnodes) {
                ss << node->path.name << " = " << node->to_string() << '\n';
            }
        }
        return ss.str();
    }

} // namespace snt::api
