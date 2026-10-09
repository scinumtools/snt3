#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/stl/filesystem.h>

#include <snt/dip/inspect/inspector.h>
#include <snt/dip/preview.h>

namespace py = pybind11;

namespace snt::bind::python {
void init_semantic(py::module_& m) {
    py::class_<dip::SchemaMemberInspection>(m, "SchemaMemberInspection")
        .def_readonly("name", &dip::SchemaMemberInspection::name)
        .def_readonly("relative_path", &dip::SchemaMemberInspection::relative_path)
        .def_readonly("kind", &dip::SchemaMemberInspection::kind)
        .def_readonly("type", &dip::SchemaMemberInspection::type)
        .def_readonly("units", &dip::SchemaMemberInspection::units)
        .def_property_readonly("dimensions", [](const dip::SchemaMemberInspection& member) {
            std::vector<std::pair<size_t, std::optional<size_t>>> ranges;
            for (const auto& range : member.dimensions)
                ranges.emplace_back(range.dmin, range.dmax == val::Array::max_range
                    ? std::nullopt : std::optional<size_t>{range.dmax});
            return ranges;
        })
        .def_readonly("schema_refs", &dip::SchemaMemberInspection::schema_refs)
        .def_readonly("options", &dip::SchemaMemberInspection::options)
        .def_readonly("condition", &dip::SchemaMemberInspection::condition)
        .def_readonly("metadata", &dip::SchemaMemberInspection::metadata)
        .def_readonly("origin", &dip::SchemaMemberInspection::origin)
        .def_readonly("members", &dip::SchemaMemberInspection::members);
    py::class_<dip::SchemaDefinitionInspection>(m, "SchemaDefinitionInspection")
        .def_readonly("id", &dip::SchemaDefinitionInspection::id)
        .def_readonly("name", &dip::SchemaDefinitionInspection::name)
        .def_readonly("metadata", &dip::SchemaDefinitionInspection::metadata)
        .def_readonly("origin", &dip::SchemaDefinitionInspection::origin)
        .def_readonly("members", &dip::SchemaDefinitionInspection::members);
    py::class_<dip::SchemaApplicationInspection>(m, "SchemaApplicationInspection")
        .def_readonly("path", &dip::SchemaApplicationInspection::path)
        .def_readonly("kind", &dip::SchemaApplicationInspection::kind)
        .def_readonly("schema_ids", &dip::SchemaApplicationInspection::schema_ids)
        .def_readonly("inherited_from_collection", &dip::SchemaApplicationInspection::inherited_from_collection)
        .def_readonly("origin", &dip::SchemaApplicationInspection::origin);
    py::class_<dip::SchemaValueAssociation>(m, "SchemaValueAssociation")
        .def_readonly("path", &dip::SchemaValueAssociation::path)
        .def_readonly("applied_schema_ids", &dip::SchemaValueAssociation::applied_schema_ids)
        .def_readonly("contributing_schema_id", &dip::SchemaValueAssociation::contributing_schema_id);
    py::class_<dip::SchemaHierarchyInspection>(m, "SchemaHierarchyInspection")
        .def_readonly("definitions_available", &dip::SchemaHierarchyInspection::definitions_available)
        .def_readonly("applications_complete", &dip::SchemaHierarchyInspection::applications_complete)
        .def_readonly("definitions", &dip::SchemaHierarchyInspection::definitions)
        .def_readonly("applications", &dip::SchemaHierarchyInspection::applications)
        .def_readonly("values", &dip::SchemaHierarchyInspection::values)
        .def_readonly("sources", &dip::SchemaHierarchyInspection::sources);
    py::enum_<dip::OverrideTargetKind>(m, "OverrideTargetKind")
        .value("Unavailable", dip::OverrideTargetKind::Unavailable)
        .value("ExistingValue", dip::OverrideTargetKind::ExistingValue)
        .value("ExistingItem", dip::OverrideTargetKind::ExistingItem)
        .value("NewItem", dip::OverrideTargetKind::NewItem);
    py::class_<dip::OverrideContract>(m, "OverrideContract")
        .def_readonly("path", &dip::OverrideContract::path)
        .def_readonly("kind", &dip::OverrideContract::kind)
        .def_readonly("reason", &dip::OverrideContract::reason)
        .def_readonly("resolved_path", &dip::OverrideContract::resolved_path)
        .def_readonly("declared_type", &dip::OverrideContract::declared_type)
        .def_readonly("current_shape", &dip::OverrideContract::current_shape)
        .def_readonly("units", &dip::OverrideContract::units)
        .def_readonly("enforced_options", &dip::OverrideContract::enforced_options)
        .def_readonly("enforced_condition", &dip::OverrideContract::enforced_condition)
        .def_readonly("item_schemas", &dip::OverrideContract::item_schemas)
        .def_readonly("snapshot_input", &dip::OverrideContract::snapshot_input);
    py::class_<dip::SemanticDescription>(m, "SemanticDescription")
        .def_readonly("path", &dip::SemanticDescription::path)
        .def_readonly("kind", &dip::SemanticDescription::kind)
        .def_readonly("declared_type", &dip::SemanticDescription::declared_type)
        .def_readonly("stored_type", &dip::SemanticDescription::stored_type)
        .def_readonly("shape", &dip::SemanticDescription::shape)
        .def_readonly("elements", &dip::SemanticDescription::elements)
        .def_readonly("value_text", &dip::SemanticDescription::value_text)
        .def_readonly("value_unavailable_reason", &dip::SemanticDescription::value_unavailable_reason)
        .def_readonly("units", &dip::SemanticDescription::units)
        .def_readonly("metadata", &dip::SemanticDescription::metadata)
        .def_readonly("tags", &dip::SemanticDescription::tags)
        .def_readonly("overridden", &dip::SemanticDescription::overridden)
        .def_readonly("declaration", &dip::SemanticDescription::declaration)
        .def_readonly("override_location", &dip::SemanticDescription::override_location)
        .def_readonly("source_text_available", &dip::SemanticDescription::source_text_available)
        .def_readonly("enforced_options", &dip::SemanticDescription::enforced_options)
        .def_readonly("enforced_condition", &dip::SemanticDescription::enforced_condition)
        .def_readonly("dependencies_recorded", &dip::SemanticDescription::dependencies_recorded)
        .def_readonly("dependencies", &dip::SemanticDescription::dependencies);

    py::class_<dip::SemanticList>(m, "SemanticList")
        .def_readonly("items", &dip::SemanticList::items)
        .def_readonly("total", &dip::SemanticList::total);

    py::enum_<dip::PreviewOverride::Kind>(m, "PreviewOverrideKind")
        .value("Text", dip::PreviewOverride::Kind::Text)
        .value("File", dip::PreviewOverride::Kind::File);
    py::class_<dip::PreviewOverride>(m, "PreviewOverride")
        .def(py::init([](dip::PreviewOverride::Kind kind, std::string input) {
            return dip::PreviewOverride{kind, std::move(input)};
        }), py::arg("kind"), py::arg("input"))
        .def_readwrite("kind", &dip::PreviewOverride::kind)
        .def_readwrite("input", &dip::PreviewOverride::input);
    py::class_<dip::PreviewResult>(m, "PreviewResult")
        .def_readonly("baseline_valid", &dip::PreviewResult::baseline_valid)
        .def_readonly("candidate_valid", &dip::PreviewResult::candidate_valid)
        .def_readonly("baseline_diagnostics", &dip::PreviewResult::baseline_diagnostics)
        .def_readonly("candidate_diagnostics", &dip::PreviewResult::candidate_diagnostics)
        .def_readonly("accepted_override_targets", &dip::PreviewResult::accepted_override_targets)
        .def_readonly("comparison", &dip::PreviewResult::comparison);

    m.def("describe", [](const dip::Environment& env, std::string_view path, std::size_t max_value_elements) {
        return dip::Inspector{env}.describe(path, max_value_elements);
    }, py::arg("env"), py::arg("path"),
          py::arg("max_value_elements") = 16);
    m.def("override_contract", [](const dip::Environment& env, std::string_view path) {
        return dip::Inspector{env}.override_contract(path);
    }, py::arg("env"), py::arg("path"));
    m.def("schema_hierarchy", [](const dip::Environment& env) {
        return dip::Inspector{env}.schema_hierarchy();
    }, py::arg("env"));
    m.def("list_descriptions", [](const dip::Environment& env, const std::string& query,
                                   const std::vector<std::string>& all,
                                   const std::vector<std::string>& any,
                                   const std::vector<std::string>& none,
                                   std::size_t limit, std::size_t max_value_elements) {
        return dip::Inspector{env}.list_descriptions(query, dip::TagFilter{all, any, none}, limit,
                                      max_value_elements);
    }, py::arg("env"), py::arg("query") = "?", py::kw_only(),
       py::arg("tags_all") = std::vector<std::string>{},
       py::arg("tags_any") = std::vector<std::string>{},
       py::arg("tags_none") = std::vector<std::string>{},
       py::arg("limit") = 100, py::arg("max_value_elements") = 0);
    m.def("preview", &dip::preview, py::arg("input"), py::arg("overrides"),
          py::arg("record_dependency_graph") = false,
          py::arg("options") = dip::ComparisonOptions{});
}
} // namespace snt::bind::python
