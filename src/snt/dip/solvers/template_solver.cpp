#include <snt/dip/exceptions.h>
#include <snt/dip/nodes/node_value.h>
#include <snt/dip/nodes/parser.h>
#include <snt/dip/solvers/template_solver.h>

namespace snt::dip {

    TemplateSolver::TemplateSolver(Environment& env, Path cur) {
        environment = &env;
        current = cur;
    }

    ValueNodeData TemplateSolver::eval(std::string expression) {
        if (expression.empty())
            throw dip::SolverException(
                "Empty expression",
                "The template expression must not be empty.",
                "An empty string was provided.",
                "Provide a non-empty template expression.",
                __FILE__,
                __LINE__
            );
        std::stringstream ss;
        bool openned = false;
        while (!expression.empty()) {
            char c = expression.front(); // get first character
            expression.erase(0, 1);      // remove first character
            if (c == SIGN_REFERENCE_OPEN) {
                openned = true;
                Parser parser({expression, {"TEMPLATE_SOLVER", 0}});
                if (parser.part_reference()) {
                    parser.part_slice();
                    parser.part_units();
                    parser.part_slice();
                    parser.part_format();

                    // request node from the environment and extract its value
                    std::string request = parser.value_raw.at(0);
                    // resolve relative path
                    if (!request.empty() && request[0] == SIGN_SEPARATOR)
                        request = std::string(1, SIGN_QUERY) + current.resolve(request).name;
                    // request absolute path
                    ValueNode::ListType nodes = environment->request_group(request, RequestType::Reference);
                    if (nodes.size() != 1) {
                        throw dip::SolverException(
                            "Ambiguous request result",
                            "The path request must return exactly one node: " + request,
                            "The request returned " + std::to_string(nodes.size()) + " nodes.",
                            "Check whether the requested path targets only one node.",
                            __FILE__,
                            __LINE__
                        );
                    }
                    const ValueNode::PointerType& vnode = nodes.front();

                    // apply slicing
                    if (!parser.value_slice.empty())
                        vnode->value = vnode->value->slice(parser.value_slice);

                    // set value string format
                    core::StringFormatType format;
                    format.stringQuotes = false;
                    if (!parser.formatting[0].empty()) {
                        if (parser.formatting[0][0] == '0')
                            format.paddingZeros = true;
                        format.paddingSize = std::stoi(parser.formatting[0]);
                    }
                    if (!parser.formatting[2].empty()) {
                        format.specifier = parser.formatting[2][0];
                    }
                    if (!parser.formatting[1].empty()) {
                        format.valuePrecision = std::stoi(parser.formatting[1]);
                        if (format.specifier == 'g')
                            format.valuePrecision += 1;
                    }

                    // parse value as string
                    ss << vnode->to_string(format);
                    expression = parser.code;
                }
            } else if (c == SIGN_REFERENCE_CLOSE) {
                if (openned)
                    openned = false;
                else
                    ss << c;
            } else {
                ss << c;
            }
        }
        // return value node data
        val::BaseValue::PointerType value = std::make_unique<val::ArrayValueStr>(ss.str());
        return ValueNodeData({std::move(value), std::nullopt});
    }

} // namespace snt::dip
