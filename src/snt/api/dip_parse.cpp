#include "dip_parse.h"

#include <iostream>
#include <sstream>

namespace snt::api {

    void DIPParse::argument_file(const std::string& file_name) {
        dip.add_file(file_name);
    }

    void DIPParse::argument_string(const std::vector<std::string>& list) {
        for (const auto& code : list)
            dip.add_string(code);
    }

    void DIPParse::argument_string(const std::string& code) {
        dip.add_string(code);
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

        // request nodes    TODO: implement tags
        dip::ValueNode::ListType vnodes;
        if (request.empty()) {
            vnodes = env.request_nodes("?");
        } else {
            vnodes = env.request_nodes(request);
        }

        // print if required
        std::stringstream ss;
        if (print == PrintOptions::ALL) {
            for (const auto& node : vnodes) {
                ss << node->name << " = " << node->to_string() << '\n';
            }
        }
        return ss.str();
    }

} // namespace snt::api
