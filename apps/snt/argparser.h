#ifndef APPS_SNT_ARGPARSER_H
#define APPS_SNT_ARGPARSER_H

#include <vector>
#include <string>

struct KeywordArgument {
  std::string key;                   ///< Argument key
  std::vector<std::string> values;   ///< Argument values
};

class ArgParser {
private:
  std::vector<std::string> positional;      ///< Positional arguments
  std::vector<KeywordArgument> keywords;    ///< Keyword arguments
    
  static bool isFlag(const std::string& s);
public:
    
  ArgParser(int argc, char** argv);
  
  const std::vector<KeywordArgument>& getAllKeywords() const;

  const std::vector<std::string>& getAllPositionals() const;

  std::string getPositionalValue(const size_t index) const;
    
  std::vector<std::string> getKeywordValues(const std::string& key1, const std::string& key2="") const;

  size_t numPositional() const;
    
  bool hasKeyword(const std::string& key) const;

  bool hasPositionals() const;
    
  bool hasKeywords() const;
    
  bool isEmpty() const;
    
};

#endif // APPS_SNT_ARGPARSER_H
