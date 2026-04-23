#include "main.h"

#include <deque>
#include <iostream>
#include <limits>
#include <map>
#include <snt/dip/dip.h>
#include <snt/dip/environment.h>
#include <snt/dip/nodes/node_value.h>
#include <sstream>
#include <stdexcept>

using namespace snt;

class InputParser {
public:
  InputParser(int& argc, char** argv) {
    for (int i = 1; i < argc; ++i)
      this->tokens.push_back(std::string(argv[i]));
  }
  const std::deque<std::string> getCmdOption(const std::string& option, const int nstr = 1) const {
    std::vector<std::string>::const_iterator itr;
    std::deque<std::string> strs;
    itr = std::find(this->tokens.begin(), this->tokens.end(), option);
    for (int i = 0; i < nstr; i++) {
      if (itr != this->tokens.end() && ++itr != this->tokens.end()) {
        strs.push_back(*itr);
      }
    }
    return strs;
  }
  bool cmdOptionExists(const std::string& option) const {
    return std::find(this->tokens.begin(), this->tokens.end(), option) != this->tokens.end();
  }
  bool cmdEmpty() const {
    return this->tokens.size() == 0;
  }

private:
  std::vector<std::string> tokens;
};

int main(int argc, char* argv[]) {

  InputParser input(argc, argv);
  if (input.cmdOptionExists("-h") || input.cmdEmpty()) {
    std::cout << '\n';
    std::cout << "Dimensional Input Parameters (DIP)" << '\n';
    std::cout << '\n';
    std::cout << "dip -h                            display help" << '\n';
    std::cout << "dip -v                            display code version" << '\n';
    std::cout << "dip --add-file [filename]         file name of a .dip file" << '\n';
    std::cout << "dip --add-string [code]           input dip code string" << '\n';
    std::cout << "dip --request [query]             request specific nodes" << '\n';
    std::cout << "dip --print                       print nodes" << '\n';
    std::cout << "dip --print-values                print node values" << '\n';
    std::cout << '\n';
    std::cout << "Example of use:" << '\n';
    std::cout << '\n';
    std::cout << "dip --add-file parameters.dip --request \"?family.father\" --print" << '\n';
    std::cout << '\n';
    exit(0);
  } else if (input.cmdOptionExists("-v")) {
    std::cout << CODE_VERSION << '\n';
    exit(0);
  }

  try {
    std::deque<std::string> convert;
    dip::DIP d;
    // TODO: add multiple files, strings, sources and custom units
    convert = input.getCmdOption("--add-file", 2);
    if (!convert.empty()) {
      d.add_file(convert[0]);
    }
    convert = input.getCmdOption("--add-string", 2);
    if (!convert.empty()) {
      d.add_string(convert[0]);
    }
    // TODO: don't parse if no files, or strings added
    dip::Environment env = d.parse();
    dip::ValueNode::ListType vnodes;
    convert = input.getCmdOption("--request", 2);
    if (!convert.empty()) {
      vnodes = env.request_nodes(convert[0]);
    } else {
      vnodes = env.request_nodes("?");
    }
    // TODO: implement tag selectors
    // TODO: print can have an argument: format
    if (input.cmdOptionExists("--print")) {
      for (const auto& node : vnodes) {
        std::cout << node->name << " = " << node->to_string() << '\n';
      }
    }
    if (input.cmdOptionExists("--print-values")) {
      for (const auto& node : vnodes) {
        std::cout << node->to_string() << '\n';
      }
    }
    // TODO: implement exports to other languages: yaml, toml, c/c++, fortran,...
    // TODO: implement output into a file
  } catch (std::exception& e) {
    std::cout << e.what() << '\n';
  }
}
