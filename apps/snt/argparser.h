/** @file
 * @brief Token grouping for CLI positional and keyword arguments.
 */
#ifndef APPS_SNT_ARGPARSER_H
#define APPS_SNT_ARGPARSER_H

#include <string>
#include <vector>

/** One flag occurrence and the following non-flag tokens. */
struct KeywordArgument {
    std::string key;                 ///< Argument key
    std::vector<std::string> values; ///< Argument values
};

/** Group CLI tokens into initial positionals and ordered flag/value occurrences. */
class ArgParser {
  private:
    std::vector<std::string> positional;   ///< Positional arguments
    std::vector<KeywordArgument> keywords; ///< Keyword arguments

    /** Recognize tokens beginning with a dash and containing at least two characters. */
    static bool isFlag(const std::string& s);

  public:
    /** Parse argv after the executable name, preserving repeated flags. */
    ArgParser(int argc, char** argv);

    /** Return flag occurrences in their original order. */
    const std::vector<KeywordArgument>& getAllKeywords() const;

    /** Return tokens appearing before the first flag. */
    const std::vector<std::string>& getAllPositionals() const;

    /** Return a positional token; throw if its zero-based index is out of range. */
    std::string getPositionalValue(const size_t index) const;

    /** Collect values from all occurrences of either flag spelling. */
    std::vector<std::string> getKeywordValues(const std::string& key1, const std::string& key2 = "") const;

    /** Return the number of positional tokens. */
    size_t numPositional() const;

    /** Test whether the exact flag spelling appeared. */
    bool hasKeyword(const std::string& key) const;

    /** Test whether positional tokens were supplied. */
    bool hasPositionals() const;

    /** Test whether any flags were supplied. */
    bool hasKeywords() const;

    /** Test whether argv contained any arguments after the executable name. */
    bool isEmpty() const;
};

#endif // APPS_SNT_ARGPARSER_H
