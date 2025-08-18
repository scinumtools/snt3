#include "../environment.h"
#include "nodes.h"

#include <regex>

namespace dip {

  BaseNode::PointerType CaseNode::is_node(Parser& parser) {
    if (parser.kwd_case()) {
      parser.part_value();
      parser.part_comment();
      return std::make_shared<CaseNode>(parser);
    }
    return nullptr;
  }

  BaseNode::NodeListType CaseNode::parse(Environment& env) {
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
        // TODO: use logical solver to solve cases
        if (value_raw.empty())
          throw std::runtime_error("Case node requires an input value: " + line.code);
        value = (value_raw.at(0) == snt::KEYWORD_TRUE) ? true : false;
      } else if (case_type == CaseType::Else) {
        value = true;
      }
    }
    return {};
  }

} // namespace dip
