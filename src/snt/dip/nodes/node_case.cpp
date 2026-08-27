#include <regex>
#include <snt/dip/environment.h>
#include <snt/dip/exceptions.h>
#include <snt/dip/nodes/node_case.h>
#include <snt/dip/solvers/logical_solver.h>

namespace snt::dip {

    BaseNode::PointerType CaseNode::is_node(Parser& parser) {
        if (parser.kwd_case()) {
            if (parser.path.name.substr(1) != KEYWORD_ELSE && parser.path.name.substr(1) != KEYWORD_END)
                parser.part_value();
            parser.part_comment();
            return std::make_shared<CaseNode>(parser);
        }
        return nullptr;
    }

    BaseNode::ListType CaseNode::parse(Environment& env) {
        std::ostringstream oss;
        oss << "^(" << PATTERN_PATH << "*[" << SIGN_CONDITION << "])";
        oss << "(" << KEYWORD_IF << "|" << KEYWORD_ELIF << "|" << KEYWORD_ELSE << "|" << KEYWORD_END << ")";
        std::regex pattern(oss.str());
        std::smatch matchResult;
        if (std::regex_search(path.name, matchResult, pattern)) {
            case_id = env.branching.register_case();
            if (matchResult[2].str() == KEYWORD_IF) {
                case_type = CaseType::IF;
            } else if (matchResult[2].str() == KEYWORD_ELIF) {
                case_type = CaseType::ELIF;
            } else if (matchResult[2].str() == KEYWORD_ELSE) {
                case_type = CaseType::ELSE;
            } else if (matchResult[2].str() == KEYWORD_END) {
                case_type = CaseType::END;
            } else {
                throw dip::SyntaxException(
                    "Unsupported case type",
                    "The case type `" + matchResult[2].str() + "` is not supported.",
                    "Use `if`, `elif`, `else`, or `end` as the case type.",
                    __FILE__,
                    __LINE__,
                    line
                );
            }
            path = Path(matchResult[1].str() + "C" + std::to_string(case_id));
            if (case_type == CaseType::IF || case_type == CaseType::ELIF) {
                if (value_raw.empty())
                    throw dip::SyntaxException(
                        "Missing case value",
                        "The `if` or `elif` case does not specify an input value.",
                        "Provide a value, function, or reference to use as the case condition.",
                        __FILE__,
                        __LINE__,
                        line
                    );
                switch (value_origin) {
                case ValueOrigin::Function:
                    value = env.request_value(value_raw.at(0), RequestType::Function)->all_of();
                    break;
                case ValueOrigin::Reference: {
                    value = env.request_value(value_raw.at(0), RequestType::Reference, units_raw)->all_of();
                    break;
                }
                case ValueOrigin::ReferenceRaw: {
                    throw dip::MissingException(
                        "Raw reference values are not supported for case conditions.", __FILE__, __LINE__, line
                    );
                    break;
                }
                case ValueOrigin::Expression: {
                    LogicalSolver solver(env, path);
                    ValueNodeData data = solver.eval(value_raw.at(0));
                    value = std::move(data.value)->all_of();
                    break;
                }
                case ValueOrigin::Keyword:
                case ValueOrigin::String:
                    if (value_raw.at(0) == core::KEYWORD_TRUE)
                        value = true;
                    else if (value_raw.at(0) == core::KEYWORD_FALSE)
                        value = false;
                    else
                        throw dip::SyntaxException(
                            "Invalid case value",
                            "The case condition contains an invalid boolean value: `" + value_raw.at(0) + "`.",
                            "Use `true` or `false` as the case condition.",
                            __FILE__,
                            __LINE__,
                            line
                        );
                    break;
                default:
                    throw dip::SyntaxException(
                        "Invalid value origin",
                        "The case condition uses an unsupported value origin.",
                        "Use a function, reference, or boolean value as the case condition.",
                        __FILE__,
                        __LINE__,
                        line
                    );
                    break;
                }
            } else if (case_type == CaseType::ELSE) {
                value = true;
            }
        }
        return {};
    }

} // namespace snt::dip
