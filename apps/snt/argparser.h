#ifndef APPS_SNT_ARGPARSER_H
#define APPS_SNT_ARGPARSER_H

#include <vector>
#include <string>

namespace snt::cli {

  struct Arg {
    std::string key;
    std::vector<std::string> values;
  };

  class ArgParser {
  private:
    std::vector<Arg> args;
    std::vector<std::string> positional;
    static bool isFlag(const std::string& s);
  public:
    ArgParser(int argc, char** argv);
    const std::vector<Arg>& sequence() const;
    std::vector<std::string> getAll(const std::string& key) const;
    bool has(const std::string& key) const;
    bool hasPositionals() const;
    bool hasArguments() const;
    bool isEmpty() const;
    const std::vector<std::string>& positionals() const;
  };

}

#endif // APPS_SNT_ARGPARSER_H
