#include "../bindings/python/cpp/val/bind_to_value.h"

#include <codecvt>
#include <locale>
#include <pybind11/numpy.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/stl/filesystem.h>
#include <snt/dip/cursor.h>
#include <snt/dip/environment.h>
#include <snt/dip/lists/list_node.h>
#include <snt/dip/nodes/node_value.h>

namespace py = pybind11;

namespace snt::bind::python {

    void init_environment(py::module_& m) {

        auto source_info =
            py::class_<dip::SourceInfo>(m, "SourceInfo", "Durable identity and content fingerprint for a DIPL source.");
        source_info.def_readonly("name", &dip::SourceInfo::name);
        source_info.def_readonly("path", &dip::SourceInfo::path);
        source_info.def_readonly("parent_name", &dip::SourceInfo::parent_name);
        source_info.def_readonly("parent_line", &dip::SourceInfo::parent_line);
        source_info.def_readonly("hash_algorithm", &dip::SourceInfo::hash_algorithm);
        source_info.def_readonly("hash", &dip::SourceInfo::hash);

        auto export_format =
            py::enum_<dip::ExportFormat>(m, "ExportFormat", "Export format for generated parameter lists.");
        export_format.value("CPP", dip::ExportFormat::CPP);
        export_format.value("C", dip::ExportFormat::C);
        export_format.value("FORTRAN", dip::ExportFormat::FORTRAN);
        export_format.value("RUST", dip::ExportFormat::RUST);
        export_format.value("R", dip::ExportFormat::R);
        export_format.value("JULIA", dip::ExportFormat::JULIA);
        export_format.value("JSON", dip::ExportFormat::JSON);
        export_format.value("TOML", dip::ExportFormat::TOML);
        export_format.value("YAML", dip::ExportFormat::YAML);
        export_format.export_values();

        auto nl = py::class_<dip::NodeList<dip::ValueNode>>(m, "NodeList", "Sequence of evaluated DIPL value nodes.");
        nl.def(py::init<>(), "Create an empty node list.");
        nl.def(
            "__getitem__",
            [](const dip::NodeList<dip::ValueNode>& self, size_t i) { return self.at(i); },
            py::arg("node"),
            "Return the node at an index."
        );
        nl.def("size", &dip::NodeList<dip::ValueNode>::size, "Return the number of nodes.");

        auto env = py::class_<dip::Environment>(
            m, "Environment", "Evaluation environment containing DIPL sources, units, functions, and nodes."
        );
        env.def(py::init<>(), "Create an empty evaluation environment.");
        env.def_property_readonly(
            "nodes", [](const dip::Environment& e) { return &e.nodes; }, "Evaluated top-level nodes."
        );
        env.def_property_readonly(
            "size", [](const dip::Environment& e) { return e.nodes.size(); }, "Number of top-level nodes."
        );
        env.def_property_readonly(
            "source_manifest",
            &dip::Environment::get_source_manifest,
            "Source identities and SHA-256 fingerprints available for this environment."
        );

        env.def("load", &dip::Environment::load, py::arg("file"), "Load an environment from an HDF5 file.");
        env.def("save", &dip::Environment::save, py::arg("file"), "Save the environment to an HDF5 file.");
        env.def(
            "generate",
            &dip::Environment::generate,
            py::arg("format"),
            py::arg("file"),
            "Generate a static parameter list."
        );

        env.def(
            "select",
            [](const dip::Environment& e, const std::string& path, const std::vector<std::string>& all,
               const std::vector<std::string>& any, const std::vector<std::string>& none) {
                return e.select(path, dip::TagFilter{all, any, none});
            },
            py::arg("path") = "?",
            py::kw_only(),
            py::arg("tags_all") = std::vector<std::string>{},
            py::arg("tags_any") = std::vector<std::string>{},
            py::arg("tags_none") = std::vector<std::string>{},
            "Select node snapshots with full paths in environment order. Empty filters impose no restriction; "
            "all filters are combined with AND. Subtrees include their value-bearing root and collection members. "
            "Tags are not inherited. No matches returns an empty list."
        );

        env.def(
            "request_group",
            [](const dip::Environment& e, const std::string& path, const std::vector<std::string>& tags) {
                return e.request_group(path, dip::RequestType::Reference, tags);
            },
            py::arg("path"),
            py::arg("tags") = std::vector<std::string>{},
            "Return a group cursor at a DIPL path."
        );

        env.def(
            "request_value",
            [](
                const dip::Environment& e, const std::string& path, const std::string& to_unit, const bool as_numpy
            ) -> py::object {
                val::BaseValue::PointerType value = e.request_value(path, dip::RequestType::Reference, to_unit);
                if (as_numpy)
                    return to_numpy_value(value);
                else
                    return to_python_value(value);
            },
            py::arg("path"),
            py::arg("to_units") = "",
            py::arg("as_numpy") = false,
            "Return the value at a path, optionally converted to a NumPy array."
        );

        env.def("__getitem__", &dip::Environment::operator[], py::arg("path"), "Return a Cursor for a DIPL path.");

        // env.def("request_code", &dip::Environment::request_code, py::arg("source_name"));

        env.def("__str__", &dip::Environment::to_string);
        env.def("__repr__", &dip::Environment::to_string);
        env.def("to_string", &dip::Environment::to_string);
    }

} // namespace snt::bind::python
