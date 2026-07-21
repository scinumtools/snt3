#ifndef DIP_SETTINGS_H
#define DIP_SETTINGS_H

#include <cstdint>
#include <snt/core/settings.h>
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
    constexpr char SIGN_ARRAY_OPEN = '[';
    constexpr char SIGN_ARRAY_CLOSE = ']';
    constexpr char SIGN_REFERENCE_OPEN = '{';
    constexpr char SIGN_REFERENCE_CLOSE = '}';
    constexpr char SIGN_EXPRESSION_OPEN = '(';
    constexpr char SIGN_EXPRESSION_CLOSE = ')';
    constexpr char SIGN_EQUAL = '=';
    constexpr std::string_view SIGN_BLOCK = "\"\"\"";

    // Keywords
    constexpr std::string_view KEYWORD_NONE = "none";
    constexpr std::string_view KEYWORD_BOOLEAN = "bool";
    constexpr std::string_view KEYWORD_INTEGER = "int";
    constexpr std::string_view KEYWORD_FLOAT = "float";
    constexpr std::string_view KEYWORD_STRING = "str";

    // directives
    constexpr std::string_view KEYWORD_CONSTANT = "!constant";
    constexpr std::string_view KEYWORD_CONDITION = "!condition";
    constexpr std::string_view KEYWORD_FORMAT = "!format";
    constexpr std::string_view KEYWORD_TAGS = "!tags";
    constexpr std::string_view KEYWORD_OPTIONS = "!options";
    constexpr std::string_view KEYWORD_DELIMITER = "!delimiter";

    // metadata
    constexpr std::string_view KEYWORD_DESCRIPTION = "?descr";
    constexpr std::string_view KEYWORD_AUTHORS = "?authors";
    constexpr std::string_view KEYWORD_TITLE = "?title";
    constexpr std::string_view KEYWORD_JOURNAL = "?journal";
    constexpr std::string_view KEYWORD_YEAR = "?year";
    constexpr std::string_view KEYWORD_VOLUME = "?volume";
    constexpr std::string_view KEYWORD_ISSUE = "?issue";
    constexpr std::string_view KEYWORD_PAGES = "?pages";
    constexpr std::string_view KEYWORD_DOI = "?doi";
    constexpr std::string_view KEYWORD_URL = "?url";
    constexpr std::string_view KEYWORD_VERSION = "?version";
    constexpr std::string_view KEYWORD_CREATED = "?created";
    constexpr std::string_view KEYWORD_MODIFIED = "?modified";
    constexpr std::string_view KEYWORD_LICENSE = "?license";

    constexpr std::string_view KEYWORD_IF = "if";
    constexpr std::string_view KEYWORD_ELIF = "elif";
    constexpr std::string_view KEYWORD_ELSE = "else";
    constexpr std::string_view KEYWORD_END = "end";

    constexpr std::string_view KEYWORD_SOURCE = "$source";
    constexpr std::string_view KEYWORD_UNIT = "$unit";
    constexpr std::string_view KEYWORD_SCHEMA = "$schema";

    // Regex Patterns
    constexpr std::string_view PATTERN_KEYWORD = "[a-zA-Z0-9_-]";
    constexpr std::string_view PATTERN_NUMBER = "[+-]?[0-9]+([.][0-9]+)?([eE][+-]?[0-9]+)?";
    constexpr std::string_view PATTERN_PATH = "[a-zA-Z0-9._-]";

    // Various settings
    constexpr std::string_view FILE_SUFFIX_DIP1 = ".dip";  ///< Suffix name of a DIPL file
    constexpr std::string_view FILE_SUFFIX_DIP2 = ".dipl"; ///< Suffix name of a DIPL file
    constexpr int INDENT_STEP = 2;                         ///< Number of white spaces in an indent step

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

    enum class CaseType { IF, ELIF, ELSE, END };

    enum class ValueOrigin {
        Empty,        // empty value
        None,         // value is set as "none"
        Keyword,      // value is a keyword
        Boolean,      // value is a boolean
        Number,       // value is a number
        String,       // value is a string
        Array,        // value was parsed from an array
        Reference,    // value was obtained from a reference
        ReferenceRaw, // value was optained from a raw reference
        Function,     // value was returned from a function
        Expression,   // value was parsed from an expression
        Schema        // value is a schema name
    };

    enum class NodeDtype {
        None,
        Empty, // empty
        Unit,
        Source, // environment
        Group,
        Case,
        Schema,
        Import, // node structure
        Boolean,
        Integer,
        Float,
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
        Format, // global properties
        Options,

        Description, // proverance metadata
        Authors,
        Title,
        Journal,
        Year,
        Volume,
        Issue,
        Pages,
        DOI,
        URL,
        Version,
        Created,
        Modified,
        License,

        Delimiter // specific properties
    };

    namespace detail {
        // is_std_vector
        template <typename T> struct is_std_vector : std::false_type {};
        template <typename T, typename Alloc> struct is_std_vector<std::vector<T, Alloc>> : std::true_type {};
        template <typename T> inline constexpr bool is_std_vector_v = is_std_vector<T>::value;
        // is_std_array
        template <typename T> struct is_std_array : std::false_type {};
        template <typename T, std::size_t N> struct is_std_array<std::array<T, N>> : std::true_type {};
        template <typename T> inline constexpr bool is_std_array_v = is_std_array<T>::value;
        // element_type
        template <typename T> struct element_type {
            using type = T;
        };
        template <typename T, typename Alloc> struct element_type<std::vector<T, Alloc>> {
            using type = T;
        };
        template <typename T, std::size_t N> struct element_type<std::array<T, N>> {
            using type = T;
        };
        template <typename T> using element_type_t = typename element_type<T>::type;
        // storage_type
        template <typename T> struct storage_type {
            using type = T;
        };
        template <> struct storage_type<bool> {
            using type = uint8_t;
        };
        template <typename T> using storage_type_t = typename storage_type<T>::type;
    } // namespace detail

} // namespace snt::dip

#endif // DIP_SETTINGS_H
