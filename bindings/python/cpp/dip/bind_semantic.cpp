#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/stl/filesystem.h>

#include <snt/dip/inspect/semantic.h>
#include <snt/dip/preview.h>

namespace py = pybind11;

namespace snt::bind::python {
void init_semantic(py::module_& m) {
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

    m.def("describe", &dip::describe, py::arg("env"), py::arg("path"),
          py::arg("max_value_elements") = 16);
    m.def("list_descriptions", [](const dip::Environment& env, const std::string& query,
                                   const std::vector<std::string>& all,
                                   const std::vector<std::string>& any,
                                   const std::vector<std::string>& none,
                                   std::size_t limit, std::size_t max_value_elements) {
        return dip::list_descriptions(env, query, dip::TagFilter{all, any, none}, limit,
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
