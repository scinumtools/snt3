#ifndef DIP_SETTINGS_H
#define DIP_SETTINGS_H

#include "../settings.h"

#include <sstream>
#include <string>

namespace snt::dip {

  // Source labels
  constexpr std::string_view ROOT_SOURCE = "ROOT";
  constexpr std::string_view FILE_SOURCE = "FILE";
  constexpr std::string_view STRING_SOURCE = "STRING";
  constexpr std::string_view DIRECT_SOURCE = "SOURCE";
  constexpr std::string_view TABLE_SOURCE = "TABLE";

  // Parsing separators
  constexpr char SEPARATOR_NEWLINE = '\n';
  constexpr char SEPARATOR_DIMENSION = ',';
  constexpr char SEPARATOR_ARRAY = ',';
  constexpr char SEPARATOR_SLICE = ':';
  constexpr char SEPARATOR_TABLE_COLUMNS = ' ';
  constexpr std::string_view SEPARATOR_TABLE_HEADER = "---";

  constexpr char SIGN_QUERY = '?';
  constexpr char SIGN_WILDCARD = '*';
  constexpr char SIGN_NEGATE = '~';
  constexpr char SIGN_DEFINED = '!';
  constexpr char SIGN_SEPARATOR = '.';
  constexpr char SIGN_CONDITION = '@';
  constexpr char SIGN_VARIABLE = '$';
  constexpr char SIGN_VALIDATION = '!';
  constexpr char SIGN_ARRAY_OPEN = '[';
  constexpr char SIGN_ARRAY_CLOSE = ']';
  constexpr char SIGN_REFERENCE_OPEN = '{';
  constexpr char SIGN_REFERENCE_CLOSE = '}';
  constexpr char SIGN_EQUAL = '=';
  constexpr std::string_view SIGN_BLOCK = "\"\"\"";

  // Keywords
  constexpr std::string_view KEYWORD_BOOLEAN = "bool";
  constexpr std::string_view KEYWORD_INTEGER = "int";
  constexpr std::string_view KEYWORD_REAL = "real";
  constexpr std::string_view KEYWORD_STRING = "str";

  constexpr std::string_view KEYWORD_CONSTANT = "constant";
  constexpr std::string_view KEYWORD_CONDITION = "condition";
  constexpr std::string_view KEYWORD_DESCRIPTION = "descr";
  constexpr std::string_view KEYWORD_FORMAT = "format";
  constexpr std::string_view KEYWORD_TAGS = "tags";
  constexpr std::string_view KEYWORD_OPTIONS = "options";
  constexpr std::string_view KEYWORD_DELIMITER = "delimiter";

  constexpr std::string_view KEYWORD_CASE = "case";
  constexpr std::string_view KEYWORD_ELSE = "else";
  constexpr std::string_view KEYWORD_END = "end";

  constexpr std::string_view KEYWORD_SOURCE = "source";
  constexpr std::string_view KEYWORD_UNIT = "unit";

  // Regex Patterns
  constexpr std::string_view PATTERN_KEY = "[a-zA-Z0-9_-]";
  constexpr std::string_view PATTERN_PATH = "[a-zA-Z0-9._-]";

  // Various settings
  constexpr std::string_view FILE_SUFFIX_DIP = ".dip";

  struct Source {
    std::string name;
    int line_number;
  };

  struct Line {
    std::string code;
    Source source;
    std::string to_string() {
      std::ostringstream oss;
      oss << "[" << source.name << ":" << source.line_number << "] " << code;
      return oss.str();
    };
  };

  enum class CaseType {
    Case,
    Else,
    End
  };

  enum class ValueOrigin {
    Keyword,
    String,
    Reference,
    ReferenceRaw,
    Function,
    Expression
  };

  enum class NodeDtype {
    None,
    Empty, // empty
    Unit,
    Source, // environment
    Group,
    Case,
    Import, // node structure
    Boolean,
    Integer,
    Real,
    String,
    Table,
    Modification, // data handling
    Property,     // properties
  };

  enum class PropertyType {
    None, // not a property
    Constant,
    Condition,
    Tags,
    Description, // global properties
    Format,
    Options,
    Delimiter // specific properties
  };

} // namespace snt::dip

#endif // DIP_SETTINGS_H
