#ifndef DIP_PARSER_H
#define DIP_PARSER_H

namespace snt::dip {

  class Parser : public Node {
  private:
    void strip(const std::string& text);
    static const std::array<std::string, 3> ESCAPE_SYMBOLS;

  public:
    std::string code; // in Python this was 'ccode', the original 'code' is now in the 'line' struct
    std::string comment;
    Parser(const Line& l) : Node(l), code(l.code) {};
    static void encode_escape_symbols(std::string& str);
    static void decode_escape_symbols(std::string& str);
    bool do_continue();
    bool kwd_case();
    bool kwd_unit();
    bool kwd_source();
    bool kwd_property(PropertyType& ptype);
    bool part_space(const bool required = true);
    bool part_indent();
    bool part_name(const bool required = true);
    bool part_type(const bool required = true);
    bool part_literal();
    bool part_dimension();
    bool part_equal(const bool required = true);
    bool part_reference();
    bool part_expression();
    bool part_function();
    bool part_array();
    bool part_string();
    bool part_keyword(const bool required = true);
    bool part_value();
    bool part_slice();
    bool part_units();
    bool part_comment();
    bool part_delimiter(const char symbol, const bool required = true);
  };

} // namespace snt::dip

#endif // DIP_PARSER_H
