#include "dip_parse.h"

#include <iostream>
#include <limits>
#include <map>
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
        request = (!path.empty() && path.front() == '?') ? path.substr(1) : path;
    }

    void DIPParse::argument_tags(const std::vector<std::string>& list) {
        tags = list;
    }

    void DIPParse::argument_print() {
        print = PrintOptions::ALL;
    }

    void DIPParse::argument_value(const std::string& type) {
        if (type != "" && type != "bool" && type != "integer" && type != "float" && type != "string") {
            throw api::ArgumentException(
                "Invalid scalar type",
                "Unknown type: " + type,
                "Use bool, integer, float, or string.",
                __FILE__,
                __LINE__
            );
        }
        print = PrintOptions::VALUE;
        value_type = type;
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
        if (print == PrintOptions::VALUE) {
            if (request.empty() || vnodes.size() != 1) {
                throw api::ArgumentException(
                    "Invalid scalar request",
                    "Request: " + request,
                    "Select exactly one scalar value with --request.",
                    __FILE__,
                    __LINE__
                );
            }
            const auto& node = vnodes.front();
            if (!node->value || !node->dimension.empty() || node->value->get_size() != 1 || node->units) {
                throw api::ArgumentException(
                    "Invalid scalar value",
                    "Request: " + request,
                    "Scalar output requires a defined value without arrays or units.",
                    __FILE__,
                    __LINE__
                );
            }
            const std::map<std::string, dip::NodeDtype> types = {
                {"bool", dip::NodeDtype::Boolean},
                {"integer", dip::NodeDtype::Integer},
                {"float", dip::NodeDtype::Float},
                {"string", dip::NodeDtype::String}
            };
            if (!value_type.empty() && node->dtype != types.at(value_type)) {
                throw api::ArgumentException(
                    "Scalar type mismatch",
                    "Request: " + request,
                    "Expected a DIPL " + value_type + " value.",
                    __FILE__,
                    __LINE__
                );
            }
            core::StringFormatType format;
            format.stringQuotes = false;
            format.valuePrecision = std::numeric_limits<double>::max_digits10;
            return node->value->to_string(format) + '\n';
        }
        if (print == PrintOptions::ALL) {
            for (const auto& node : vnodes) {
                ss << node->path.name << " = " << node->to_string() << '\n';
            }
        }
        return ss.str();
    }

} // namespace snt::api
