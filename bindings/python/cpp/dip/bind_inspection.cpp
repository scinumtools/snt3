#include "../bindings/python/cpp/val/bind_to_value.h"

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/stl/filesystem.h>
#include <snt/dip/inspection.h>
#include <snt/core/diagnostic.h>

#include <utility>

namespace py = pybind11;

namespace snt::bind::python {

void init_inspection(py::module_& m) {
    py::enum_<core::DiagnosticSeverity>(m, "DiagnosticSeverity")
        .value("Info", core::DiagnosticSeverity::Info)
        .value("Warning", core::DiagnosticSeverity::Warning)
        .value("Error", core::DiagnosticSeverity::Error);

    py::class_<core::SourceLocation>(m, "SourceLocation", "Source file, line, and optional code and column.")
        .def_readonly("source", &core::SourceLocation::source)
        .def_readonly("line", &core::SourceLocation::line)
        .def_readonly("code", &core::SourceLocation::code)
        .def_readonly("column", &core::SourceLocation::column);

    py::class_<core::Diagnostic>(m, "Diagnostic", "Structured error information for a DIP operation.")
        .def_readonly("severity", &core::Diagnostic::severity)
        .def_readonly("code", &core::Diagnostic::code)
        .def_readonly("message", &core::Diagnostic::message)
        .def_readonly("details", &core::Diagnostic::details)
        .def_readonly("suggestion", &core::Diagnostic::suggestion)
        .def_readonly("node_path", &core::Diagnostic::node_path)
        .def_readonly("location", &core::Diagnostic::location)
        .def_readonly("origin", &core::Diagnostic::origin);

    py::enum_<dip::ArtifactKind>(m, "ArtifactKind")
        .value("Unknown", dip::ArtifactKind::Unknown)
        .value("Project", dip::ArtifactKind::Project)
        .value("DIPL", dip::ArtifactKind::DIPL)
        .value("TableText", dip::ArtifactKind::TableText)
        .value("DIPH5", dip::ArtifactKind::DIPH5);

    py::enum_<dip::ValueChangeKind>(m, "ValueChangeKind")
        .value("Declaration", dip::ValueChangeKind::Declaration)
        .value("Modification", dip::ValueChangeKind::Modification)
        .value("Override", dip::ValueChangeKind::Override);

    py::class_<dip::ValueChange>(m, "ValueChange", "An applied change in evaluation order.")
        .def_readonly("kind", &dip::ValueChange::kind)
        .def_readonly("location", &dip::ValueChange::location);

    py::class_<dip::ValueInspection>(m, "ValueInspection", "Owned snapshot of an evaluated value and its provenance.")
        .def_readonly("path", &dip::ValueInspection::path)
        .def_readonly("type", &dip::ValueInspection::type)
        .def_readonly("shape", &dip::ValueInspection::shape)
        .def_property_readonly("value", [](const dip::ValueInspection& value) { return to_python_value(value.value); })
        .def("to_numpy", [](const dip::ValueInspection& value) { return to_numpy_value(value.value); })
        .def_readonly("units", &dip::ValueInspection::units)
        .def_readonly("metadata", &dip::ValueInspection::metadata)
        .def_readonly("tags", &dip::ValueInspection::tags)
        .def_readonly("provenance", &dip::ValueInspection::provenance)
        .def_readonly("declaration_location", &dip::ValueInspection::declaration_location)
        .def_readonly("override_location", &dip::ValueInspection::override_location)
        .def_readonly("applied_schemas", &dip::ValueInspection::applied_schemas)
        .def_readonly("contributing_schema", &dip::ValueInspection::contributing_schema)
        .def_readonly("table_path", &dip::ValueInspection::table_path)
        .def_readonly("changes", &dip::ValueInspection::changes);

    py::class_<dip::InspectionCapabilities>(m, "InspectionCapabilities", "Available inspection operations at a path.")
        .def_readonly("has_value", &dip::InspectionCapabilities::hasValue)
        .def_readonly("has_children", &dip::InspectionCapabilities::hasChildren)
        .def_readonly("has_source", &dip::InspectionCapabilities::hasSource)
        .def_readonly("has_provenance", &dip::InspectionCapabilities::hasProvenance)
        .def_readonly("has_tabular_data", &dip::InspectionCapabilities::hasTabularData)
        .def_readonly("has_array_data", &dip::InspectionCapabilities::hasArrayData)
        .def_readonly("has_reference_graph", &dip::InspectionCapabilities::hasReferenceGraph)
        .def_readonly("source_editable", &dip::InspectionCapabilities::sourceEditable)
        .def_readonly("directly_writable", &dip::InspectionCapabilities::directlyWritable);

    py::enum_<exs::CompositionKind>(m, "CompositionKind")
        .value("Operand", exs::CompositionKind::Operand)
        .value("Operator", exs::CompositionKind::Operator)
        .value("Group", exs::CompositionKind::Group);
    py::enum_<exs::OperationType>(m, "OperationType")
        .value("Unary", exs::UNARY_OPERATION)
        .value("Binary", exs::BINARY_OPERATION)
        .value("Ternary", exs::TERNARY_OPERATION)
        .value("Group", exs::GROUP_OPERATION);
    py::class_<exs::CompositionNode>(m, "CompositionNode")
        .def_readonly("kind", &exs::CompositionNode::kind)
        .def_readonly("text", &exs::CompositionNode::text)
        .def_readonly("operator_type", &exs::CompositionNode::operator_type)
        .def_readonly("operation", &exs::CompositionNode::operation)
        .def_readonly("children", &exs::CompositionNode::children);
    py::class_<exs::CompositionGraph>(m, "CompositionGraph")
        .def_readonly("nodes", &exs::CompositionGraph::nodes)
        .def_readonly("root", &exs::CompositionGraph::root);
    py::enum_<dip::DependencyEventKind>(m, "DependencyEventKind")
        .value("Value", dip::DependencyEventKind::Value)
        .value("Condition", dip::DependencyEventKind::Condition)
        .value("Decision", dip::DependencyEventKind::Decision);
    py::class_<dip::DependencyEdge>(m, "DependencyEdge")
        .def_readonly("target", &dip::DependencyEdge::target)
        .def_readonly("request", &dip::DependencyEdge::request)
        .def_readonly("operand", &dip::DependencyEdge::operand);
    py::class_<dip::DependencyEvent>(m, "DependencyEvent")
        .def_readonly("owner", &dip::DependencyEvent::owner)
        .def_readonly("kind", &dip::DependencyEvent::kind)
        .def_readonly("location", &dip::DependencyEvent::location)
        .def_readonly("controlled_by", &dip::DependencyEvent::controlled_by)
        .def_readonly("expression", &dip::DependencyEvent::expression)
        .def_readonly("composition", &dip::DependencyEvent::composition)
        .def_readonly("reads", &dip::DependencyEvent::reads);
    py::class_<dip::DependencyGraph>(m, "DependencyGraph")
        .def_readonly("recorded", &dip::DependencyGraph::recorded)
        .def_readonly("events", &dip::DependencyGraph::events)
        .def("latest", [](const dip::DependencyGraph& graph, const std::string& owner,
                           dip::DependencyEventKind kind) -> py::object {
            const auto* event = graph.latest(owner, kind);
            return event ? py::cast(*event) : py::none();
        }, py::arg("owner"), py::arg("kind"))
        .def("dependencies", &dip::DependencyGraph::dependencies, py::arg("owner"))
        .def("referenced_by", &dip::DependencyGraph::referenced_by, py::arg("target"));

    py::class_<dip::TableColumnInspection>(m, "TableColumnInspection", "Metadata for an evaluated table column.")
        .def_readonly("index", &dip::TableColumnInspection::index)
        .def_readonly("name", &dip::TableColumnInspection::name)
        .def_readonly("path", &dip::TableColumnInspection::path)
        .def_readonly("type", &dip::TableColumnInspection::type)
        .def_readonly("units", &dip::TableColumnInspection::units)
        .def_readonly("metadata", &dip::TableColumnInspection::metadata);

    py::class_<dip::TableInspection>(m, "TableInspection", "Row count and columns in DIPL header order.")
        .def_readonly("path", &dip::TableInspection::path)
        .def_readonly("rows", &dip::TableInspection::rows)
        .def_readonly("columns", &dip::TableInspection::columns);

    m.def("detect_artifact", &dip::detect_artifact, py::arg("path"),
          "Classify a DIP path by its filename without reading its contents.");
    m.def("open_artifact", &dip::open_artifact, py::arg("path"),
          py::arg("record_dependency_graph") = false,
          "Open a DIPfile, DIPL source, or DIPH5 snapshot. Graph recording is opt-in for source; snapshots use their saved graph.");
    m.def("reload_artifact", &dip::reload_artifact, py::arg("env"), py::arg("path"),
          py::arg("record_dependency_graph") = false,
          "Replace an Environment after successful load; request graph recording again for parsed source.");
    m.def("inspect_value", &dip::inspect_value, py::arg("env"), py::arg("path"),
          "Return an owned snapshot of an evaluated value and its provenance.");
    m.def("inspect_values", [](const dip::Environment& env) {
        py::list result;
        for (auto& value : dip::inspect_values(env)) result.append(py::cast(std::move(value)));
        return result;
    }, py::arg("env"), "Return value snapshots in environment order.");
    m.def("inspect_capabilities", &dip::inspect_capabilities, py::arg("env"), py::arg("path"),
          "Return supported inspection operations and retained facts at a path.");
    m.def("inspect_dependency_graph", [](const dip::Environment& env) { return env.dependency_graph(); },
          py::arg("env"), "Return an owned graph of evaluated DIP reads and operation trees.");
    m.def("inspect_table", &dip::inspect_table, py::arg("env"), py::arg("path"),
          "Return table metadata with columns in DIPL header order.");
    m.def("inspect_tables", &dip::inspect_tables, py::arg("env"),
          "Return all evaluated tables in environment order.");
    m.def("read_value_slice", [](const dip::Environment& env, const std::string& path,
                                  const std::vector<std::pair<size_t, size_t>>& ranges, bool as_numpy) {
        val::Array::RangeType native_ranges;
        native_ranges.reserve(ranges.size());
        for (const auto& [first, last] : ranges) native_ranges.push_back({first, last});
        auto value = dip::read_value_slice(env, path, native_ranges);
        return as_numpy ? to_numpy_value(value) : to_python_value(value);
    }, py::arg("env"), py::arg("path"), py::arg("ranges"), py::kw_only(), py::arg("as_numpy") = false,
       "Read inclusive, zero-based ranges from an evaluated in-memory array.");
}

} // namespace snt::bind::python
