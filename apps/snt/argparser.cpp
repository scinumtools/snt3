#include "argparser.h"

namespace snt::cli {

  bool ArgParser::isFlag(const std::string& s) {
    return s.size() > 1 && s[0] == '-';
  }
  
  ArgParser::ArgParser(int argc, char** argv) {
    for (int i = 1; i < argc; ++i) {
      std::string s = argv[i];
    
      if (isFlag(s)) {
	args.push_back({s, {}});
      } else if (!args.empty()) {
	args.back().values.push_back(s);
      } else {
	positional.push_back(s);
      }
    }
  }

  // ordered access
  const std::vector<Arg>& ArgParser::sequence() const {
    return args;
  }

  // get all values of a repeated flag
  std::vector<std::string> ArgParser::getAll(const std::string& key) const {
    std::vector<std::string> out;
    for (const auto& a : args)
      if (a.key == key)
	out.insert(out.end(), a.values.begin(), a.values.end());
    return out;
  }

  // check existence
  bool ArgParser::has(const std::string& key) const {
    for (const auto& a : args)
      if (a.key == key) return true;
    return false;
  }

  bool ArgParser::hasPositionals() const {
    return !positional.empty();
  }

  bool ArgParser::hasArguments() const {
    return !args.empty();
  }

  bool ArgParser::isEmpty() const {
    return positional.empty() && args.empty();
  }

  // positional args
  const std::vector<std::string>& ArgParser::positionals() const {
    return positional;
  }

}
