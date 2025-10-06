#include "node_case.h"

#include "../environment.h"
#include "../solvers/solver_logical.h"

#include <regex>

namespace snt::dip {

  BaseNode::PointerType CaseNode::is_node(Parser& parser) {
    if (parser.kwd_case()) {
      parser.part_value();
      parser.part_comment();
      return std::make_shared<CaseNode>(parser);
    }
    return nullptr;
  }

  BaseNode::ListType CaseNode::parse(Environment& env) {
    std::ostringstream oss;
    oss << "^(" << PATTERN_PATH << "*[" << SIGN_CONDITION << "])";
    oss << "(" << KEYWORD_CASE << "|" << KEYWORD_ELSE << "|" << KEYWORD_END << ")";
    std::regex pattern(oss.str());
    std::smatch matchResult;
    if (std::regex_search(name, matchResult, pattern)) {
      case_id = env.branching.register_case();
      if (matchResult[2].str() == KEYWORD_CASE) {
        case_type = CaseType::Case;
      } else if (matchResult[2].str() == KEYWORD_ELSE) {
        case_type = CaseType::Else;
      } else if (matchResult[2].str() == KEYWORD_END) {
        case_type = CaseType::End;
      } else {
        throw std::runtime_error("Unsupported case type: " + line.code);
      }
      name = matchResult[1].str() + "C" + std::to_string(case_id);
      if (case_type == CaseType::Case) {
        if (value_raw.empty())
          throw std::runtime_error("Case node requires an input value: " + line.code);
        switch (value_origin) {
        case ValueOrigin::Function:
          value = env.request_value(value_raw.at(0), RequestType::Function)->all_of();
          break;
        case ValueOrigin::Reference: {
          value = env.request_value(value_raw.at(0), RequestType::Reference, units_raw)->all_of();
          break;
        }
        case ValueOrigin::ReferenceRaw: {
          throw std::runtime_error("Raw reference value is not implemented: " + line.code);
          break;
        }
        case ValueOrigin::Expression: {
          LogicalSolver solver(env);
          LogicalAtom result = solver.eval(value_raw.at(0));
          value = std::move(result.value)->all_of();
          break;
        }
        default:
          if (value_raw.at(0) == snt::KEYWORD_TRUE)
            value = true;
          else if (value_raw.at(0) == snt::KEYWORD_FALSE)
            value = false;
          else
            throw std::runtime_error("Invalid value: " + line.code);
          break;
        }
      } else if (case_type == CaseType::Else) {
        value = true;
      }
    }
    return {};
  }

} // namespace snt::dip
