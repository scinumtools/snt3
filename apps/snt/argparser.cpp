#include "argparser.h"

bool ArgParser::isFlag(const std::string& s) {
  return s.size() > 1 && s[0] == '-';
}
  
ArgParser::ArgParser(int argc, char** argv) {
  for (int i = 1; i < argc; ++i) {
    std::string s = argv[i];
    
    if (isFlag(s)) {
      keywords.push_back({s, {}});
    } else if (!keywords.empty()) {
      keywords.back().values.push_back(s);
    } else {
      positional.push_back(s);
    }
  }
}

// ordered access
const std::vector<KeywordArgument>& ArgParser::getAllKeywords() const {
  return keywords;
}

// positional args
const std::vector<std::string>& ArgParser::getAllPositionals() const {
  return positional;
}

std::string ArgParser::getPositionalValue(const size_t index) const {
  if (index>=positional.size())
    throw std::runtime_error("Requesting positional argument "+std::to_string(index)+
			     " out of "+std::to_string(positional.size()));
  return positional[index];
}
  
// get all values of a repeated flag
std::vector<std::string> ArgParser::getKeywordValues(const std::string& key1, const std::string& key2) const {
  std::vector<std::string> out;
  for (const auto& a : keywords)
    if (a.key == key1 || (!key2.empty() && a.key==key2))
      out.insert(out.end(), a.values.begin(), a.values.end());
  return out;
}

size_t ArgParser::numPositional() const {
  return positional.size();
}
  
// check existence
bool ArgParser::hasKeyword(const std::string& key) const {
  for (const auto& a : keywords)
    if (a.key == key) return true;
  return false;
}

bool ArgParser::hasPositionals() const {
  return !positional.empty();
}

bool ArgParser::hasKeywords() const {
  return !keywords.empty();
}

bool ArgParser::isEmpty() const {
  return positional.empty() && keywords.empty();
}

