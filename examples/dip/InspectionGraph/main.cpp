#include <snt/dip/diagnostic.h>
#include <snt/dip/inspection.h>
#include <snt/val/value_base.h>

#include <exception>
#include <filesystem>
#include <iostream>
#include <ostream>
#include <string>

namespace fs = std::filesystem;

namespace {

void print_location(std::ostream& output, const snt::core::SourceLocation& location) {
    output << fs::path(location.source).filename().string() << ':' << location.line;
}

const char* change_name(snt::dip::ValueChangeKind kind) {
    switch (kind) {
    case snt::dip::ValueChangeKind::Declaration: return "declaration";
    case snt::dip::ValueChangeKind::Modification: return "modification";
    case snt::dip::ValueChangeKind::Override: return "override";
    }
    return "change";
}

void print_expression(const snt::exs::CompositionGraph& graph, std::size_t index, int depth = 0) {
    const auto& node = graph.nodes.at(index);
    std::cout << std::string(static_cast<std::size_t>(depth) * 2, ' ');
    if (node.kind == snt::exs::CompositionKind::Operand)
        std::cout << "operand ";
    else if (node.kind == snt::exs::CompositionKind::Operator)
        std::cout << "operator ";
    else
        std::cout << "group ";
    std::cout << node.text << '\n';
    for (const auto child : node.children)
        print_expression(graph, child, depth + 1);
}

} // namespace

int main() {
    try {
        const fs::path project = "examples/dip/InspectionGraph/DIPfile";
        auto env = snt::dip::open_artifact(project, true); // Graph recording is opt-in.

        const auto speed = snt::dip::inspect_value(env, "experiment.speed");
        std::cout << "Value: " << speed.path << " = " << speed.value->to_string();
        if (speed.units)
            std::cout << ' ' << speed.units->to_string();
        std::cout << '\n';
        std::cout << "Description: " << speed.metadata.description << '\n';
        std::cout << "Declared at: ";
        print_location(std::cout, speed.declaration_location);
        std::cout << '\n';

        const auto distance = snt::dip::inspect_value(env, "experiment.distance");
        std::cout << "\nDistance history:\n";
        for (const auto& change : distance.changes) {
            std::cout << "  " << change_name(change.kind) << " at ";
            print_location(std::cout, change.location);
            std::cout << '\n';
        }

        const auto table = snt::dip::inspect_table(env, "measurements");
        std::cout << "\nTable: " << table.path << " (" << table.rows << " rows)\n";
        for (const auto& column : table.columns)
            std::cout << "  " << column.name << " -> " << column.path << '\n';
        const auto slice = snt::dip::read_value_slice(env, "samples", {{1, 3}});
        std::cout << "\nSamples [1..3]: " << slice->to_string() << '\n';

        const auto& graph = env.dependency_graph();
        std::cout << "\nGraph recorded: " << (graph.recorded ? "yes" : "no") << '\n';
        std::cout << "\nSpeed reads:\n";
        for (const auto& edge : graph.dependencies("?experiment.speed"))
            std::cout << "  " << edge.request << " -> " << edge.target << '\n';
        std::cout << "\nReaders of ?experiment.distance:\n";
        for (const auto& owner : graph.referenced_by("?experiment.distance"))
            std::cout << "  " << owner << '\n';

        const auto* calculation = graph.latest("?experiment.speed", snt::dip::DependencyEventKind::Value);
        if (calculation && calculation->composition) {
            std::cout << "\nSpeed expression:\n";
            print_expression(*calculation->composition, calculation->composition->root);
        }
        const auto* condition = graph.latest("?experiment.speed", snt::dip::DependencyEventKind::Condition);
        if (condition) {
            std::cout << "\nSpeed condition: " << condition->expression << '\n';
            for (const auto& edge : condition->reads)
                std::cout << "  reads " << edge.target << '\n';
        }
        const auto* status = graph.latest("?status", snt::dip::DependencyEventKind::Value);
        if (status) {
            std::cout << "\nStatus selected by:\n";
            for (const auto& decision : status->controlled_by) {
                std::cout << "  " << decision << '\n';
                const auto* event = graph.latest(decision, snt::dip::DependencyEventKind::Decision);
                if (event) {
                    std::cout << "    expression: " << event->expression << '\n';
                    for (const auto& edge : event->reads)
                        std::cout << "    reads " << edge.target << '\n';
                }
            }
        }
        return 0;
    } catch (const std::exception& error) {
        const auto diagnostic = snt::dip::diagnostic_from_exception(error);
        std::cerr << diagnostic.code << ": " << diagnostic.message << '\n';
        if (diagnostic.location) {
            print_location(std::cerr, *diagnostic.location);
            std::cerr << '\n';
        }
        return 1;
    }
}
