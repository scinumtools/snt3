#include "writer_common.h"
#include "naming.h"

#include <snt/dip/exceptions.h>
#include <unordered_set>
#include <vector>

namespace snt::dip::generate {
    namespace {
        class CppWriter {
          public:
            explicit CppWriter(std::ostream& output) : output_(output) {}

            void write(const Node& root) {
                write_definitions(root);
                output_ << "inline constexpr Parameters parameters = " << initializer(root) << ";\n";
            }

          private:
            std::ostream& output_;
            std::unordered_set<std::string> emitted_types_;
            std::unordered_map<std::string, std::string> type_paths_;

            void register_type(const std::string& type, const std::string& path) {
                const auto found = type_paths_.find(type);
                if (found == type_paths_.end()) {
                    type_paths_.emplace(type, path);
                    return;
                }
                if (found->second != path)
                    throw dip::EnvironmentException(
                        "Export name collision",
                        "The DIPL paths `" + found->second + "` and `" + path + "` map to the same C++ type name `" +
                            type + "`.",
                        "Rename one of the colliding DIPL nodes before exporting.",
                        __FILE__,
                        __LINE__
                    );
            }

            void validate_collection(const Node& collection) const {
                if (collection.children.empty())
                    return;
                const Node& prototype = *collection.children.front();
                for (size_t index = 1; index < collection.children.size(); ++index) {
                    if (!same_shape(prototype, *collection.children[index]))
                        throw dip::EnvironmentException(
                            "Non-homogeneous collection export",
                            "The collection `" + collection.path + "` has entries with different shapes.",
                            "Use entries with the same fields and value types before exporting to C++.",
                            __FILE__,
                            __LINE__
                        );
                }
            }

            std::string type(const Node& node) const {
                if (is_scalar(node))
                    return cpp_value_type(node);
                if (node.kind == NodeKind::Map)
                    return "std::array<" + type_stem(node.path) + "Entry, " + std::to_string(node.children.size()) +
                           ">";
                if (node.kind == NodeKind::List)
                    return "std::array<" + cpp_item_type(*node.children.front()) + ", " +
                           std::to_string(node.children.size()) + ">";
                return cpp_object_type(node);
            }

            void write_definitions(const Node& node) {
                if (is_scalar(node))
                    return;
                if (node.kind == NodeKind::Map || node.kind == NodeKind::List) {
                    validate_collection(node);
                    if (!node.children.empty())
                        write_definitions(*node.children.front());
                    if (node.kind == NodeKind::Map)
                        write_map_entry(node);
                    return;
                }
                for (const auto& child : node.children)
                    write_definitions(*child);
                write_object(node);
            }

            void write_map_entry(const Node& map) {
                const std::string type_name = type_stem(map.path) + "Entry";
                register_type(type_name, map.path + "[]");
                if (!emitted_types_.insert(type_name).second)
                    return;
                output_ << "struct " << type_name << " {\n"
                        << "  std::string_view key;\n"
                        << "  " << cpp_item_type(*map.children.front()) << " value;\n"
                        << "};\n\n";
            }

            void write_object(const Node& node) {
                const std::string type_name = cpp_object_type(node);
                register_type(type_name, node.kind == NodeKind::Root ? "" : node.path);
                if (!emitted_types_.insert(type_name).second)
                    return;
                output_ << "struct " << type_name << " {\n";
                std::unordered_set<std::string> used;
                for (const auto& child : node.children)
                    output_ << "  " << type(*child) << ' ' << identifier(child->name, used) << ";\n";
                output_ << "};\n\n";
            }

            std::string initializer(const Node& node) const {
                if (is_scalar(node))
                    return cpp_initializer(node);
                std::string output = "{ ";
                if (node.kind == NodeKind::Map) {
                    output += "{ ";
                    for (size_t index = 0; index < node.children.size(); ++index) {
                        if (index)
                            output += ", ";
                        const Node& item = *node.children[index];
                        output += "{ \"" + cpp_escape(item.name) + "\", " + initializer(item) + " }";
                    }
                    return output + " } }";
                }
                if (node.kind == NodeKind::List) {
                    output += "{ ";
                    for (size_t index = 0; index < node.children.size(); ++index) {
                        if (index)
                            output += ", ";
                        output += initializer(*node.children[index]);
                    }
                    return output + " } }";
                }
                for (size_t index = 0; index < node.children.size(); ++index) {
                    if (index)
                        output += ", ";
                    output += initializer(*node.children[index]);
                }
                return output + " }";
            }
        };

    } // namespace

    void write_cpp(std::ostream& output, const Node& root) {
        CppWriter(output).write(root);
    }
} // namespace snt::dip::generate
