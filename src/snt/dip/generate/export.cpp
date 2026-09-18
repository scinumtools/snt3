#include "export.h"

#include "model.h"

#include <fstream>
#include <snt/dip/exceptions.h>

namespace snt::dip::generate {
    namespace {
        std::string indent(size_t depth) { return std::string(depth * 2, ' '); }

        void write_json(std::ostream& output, const Node& node, size_t depth) {
            if (node.kind == NodeKind::List) {
                output << '[';
                for (size_t index = 0; index < node.children.size(); ++index) {
                    if (index)
                        output << ',';
                    output << '\n' << indent(depth + 1);
                    write_json(output, *node.children[index], depth + 1);
                }
                if (!node.children.empty())
                    output << '\n' << indent(depth);
                output << ']';
                return;
            }
            if (node.kind == NodeKind::Value && node.children.empty()) {
                output << node.literal;
                return;
            }

            output << '{';
            for (size_t index = 0; index < node.children.size(); ++index) {
                if (index)
                    output << ',';
                const Node& child = *node.children[index];
                output << '\n' << indent(depth + 1) << '"' << child.name << "\": ";
                write_json(output, child, depth + 1);
            }
            if (!node.children.empty())
                output << '\n' << indent(depth);
            output << '}';
        }

        void write_yaml(std::ostream& output, const Node& node, size_t depth) {
            if (node.kind == NodeKind::Value && node.children.empty()) {
                output << node.literal;
                return;
            }
            if (node.kind == NodeKind::List) {
                for (const auto& child : node.children) {
                    output << indent(depth) << "- ";
                    if (child->kind == NodeKind::Value && child->children.empty()) {
                        write_yaml(output, *child, depth + 1);
                        output << '\n';
                    } else {
                        output << '\n';
                        write_yaml(output, *child, depth + 1);
                    }
                }
                return;
            }
            for (const auto& child : node.children) {
                output << indent(depth) << child->name << ':';
                if (child->kind == NodeKind::Value && child->children.empty()) {
                    output << ' ';
                    write_yaml(output, *child, depth + 1);
                    output << '\n';
                } else {
                    output << '\n';
                    write_yaml(output, *child, depth + 1);
                }
            }
        }
    } // namespace

    void write(const Environment& environment, ExportFormat format, const std::filesystem::path& file) {
        const std::unique_ptr<Node> model = build_model(environment);
        std::ofstream output(file, std::ios::binary | std::ios::trunc);
        if (!output)
            throw dip::IOException(
                "Unable to create exported environment",
                "The file `" + file.string() + "` could not be opened for writing.",
                "Check that its parent directory exists and is writable.",
                __FILE__,
                __LINE__
            );
        switch (format) {
        case ExportFormat::JSON:
            write_json(output, *model, 0);
            output << '\n';
            break;
        case ExportFormat::YAML:
            write_yaml(output, *model, 0);
            break;
        default:
            throw dip::MissingException("This export format is not implemented yet.", __FILE__, __LINE__);
        }
        if (!output)
            throw dip::IOException(
                "Unable to write exported environment",
                "Writing the file `" + file.string() + "` failed.",
                "Check that the destination has sufficient free space and is writable.",
                __FILE__,
                __LINE__
            );
    }
} // namespace snt::dip::generate
