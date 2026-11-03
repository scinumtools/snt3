#ifndef SNT_DIP_GENERATE_WRITER_COMMON_H
#define SNT_DIP_GENERATE_WRITER_COMMON_H

#include "model.h"

#include <iosfwd>
#include <string>

namespace snt::dip::generate {
    /**
     * Make two spaces per nesting level.
     * @param depth Nesting level.
     */
    std::string indent(size_t depth);
    /**
     * Check whether a node holds one value.
     * @param node Node to inspect.
     */
    bool is_scalar(const Node& node);
    /**
     * Map a DIPL scalar type to C++.
     * @param type DIPL data type.
     */
    std::string cpp_scalar_type(core::DataType type);
    /**
     * Determine the generated C++ value type.
     * @param node Value node.
     */
    std::string cpp_value_type(const Node& node);
    /**
     * Normalize a scalar literal for C++.
     * @param literal DIPL literal.
     */
    std::string cpp_literal(std::string literal);
    /**
     * Render a C++ initializer.
     * @param node Node and its evaluated value.
     */
    std::string cpp_initializer(const Node& node);
    /**
     * Escape text for a C++ string literal.
     * @param value Unescaped text.
     */
    std::string cpp_escape(const std::string& value);
    /**
     * Turn a DIPL path into a generated type stem.
     * @param path Logical node path.
     */
    std::string type_stem(const std::string& path);
    /**
     * Return the path used to name a collection.
     * @param node Collection node.
     */
    std::string collection_path(const Node& node);
    /**
     * Determine a generated C++ object type.
     * @param node Object node.
     */
    std::string cpp_object_type(const Node& node);
    /**
     * Determine a generated C++ collection item type.
     * @param item Item node.
     */
    std::string cpp_item_type(const Node& item);
    /**
     * Compare array shapes.
     * @param left First node.
     * @param right Second node.
     */
    bool same_shape(const Node& left, const Node& right);

    /**
     * Write a C++ representation.
     * @param output Destination stream.
     * @param root Model root.
     */
    void write_cpp(std::ostream& output, const Node& root);
    /**
     * Write a C representation.
     * @param output Destination stream.
     * @param root Model root.
     */
    void write_c(std::ostream& output, const Node& root);
    /**
     * Write a Rust representation.
     * @param output Destination stream.
     * @param root Model root.
     */
    void write_rust(std::ostream& output, const Node& root);
    /**
     * Write a Julia representation.
     * @param output Destination stream.
     * @param root Model root.
     */
    void write_julia(std::ostream& output, const Node& root);
    /**
     * Write a Fortran representation.
     * @param output Destination stream.
     * @param root Model root.
     */
    void write_fortran(std::ostream& output, const Node& root);
} // namespace snt::dip::generate

#endif
