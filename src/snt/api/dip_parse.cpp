#include "dip_parse.h"

#include <iostream>
#include <sstream>

namespace snt::api {

    void DIPParse::argument_input(const std::string& input_type, const std::vector<std::string>& input_values) {
        if (input_type == "file" && input_values.size() == 1) {
            dip.add_file(input_values[0]);
        } else if (input_type == "string" && input_values.size() == 1) {
            dip.add_string(input_values[0]);
        } else if (input_type == "source" && input_values.size() == 2) {
            dip.add_source(input_values[0], input_values[1]);
        } else if (input_type == "unit" && input_values.size() == 2) {
            dip.add_unit(input_values[0], input_values[1]);
        } else {
            throw std::runtime_error("Unrecognized input type or values");
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
