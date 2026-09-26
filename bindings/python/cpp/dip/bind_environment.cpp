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

        auto schema = py::class_<dip::EnvSchema>(
            m, "SchemaDefinition", "Registered DIPL schema definition; its metadata are not copied to instances."
        );
        schema.def_readonly("name", &dip::EnvSchema::name);
        schema.def_readonly("id", &dip::EnvSchema::id);
        schema.def_property_readonly(
            "metadata", [](const dip::EnvSchema& s) -> const dip::ValueMetadata& { return s.metadata; },
            py::return_value_policy::reference_internal,
            "Metadata describing this reusable definition, not its applying group, collection, item, or values."
        );

        auto env = py::class_<dip::Environment>(
            m, "Environment", "Evaluation environment containing DIPL sources, units, schemas, functions, and nodes."
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
        env.def_property_readonly(
            "schemas", [](const dip::Environment& e) { return e.schemas.entries(); },
            "Registered schema definitions keyed by name. The mapping is a snapshot and remains valid "
            "independently of this environment. DIPH5 loading does not restore reusable schemas."
        );

        env.def(
            "load", &dip::Environment::load, py::arg("file"),
            "Load evaluated nodes from DIPH5. Schema definitions and their metadata are not reconstructed.\n\n"
            "Args:\n    file: Path to the DIPH5 file."
        );
        env.def(
            "save", &dip::Environment::save, py::arg("file"),
            "Save evaluated nodes to DIPH5. Schema identities are recorded, but definitions and their metadata are not.\n\n"
            "Args:\n    file: Output DIPH5 path; an existing file is overwritten."
        );
        env.def(
            "generate",
            &dip::Environment::generate,
            py::arg("format"),
            py::arg("file"),
            "Generate a static parameter list.\n\nArgs:\n    format: ExportFormat member selecting the output format.\n"
            "    file: Output path; an existing file is overwritten."
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
            R"doc(Select independent node snapshots with full paths, in environment order.
Subtrees include value-bearing roots and collection members; each node
appears once. Tags are explicit and not inherited. Filters combine with AND;
empty filters impose no restriction. Selection does not modify the environment.
No matches returns an empty list.

Args:
    path: Query path; ? selects all, ?path. a subtree, and ?path an exact node.
    tags_all: Require every listed tag.
    tags_any: Require at least one listed tag.
    tags_none: Exclude nodes with any listed tag.
)doc"
        );

        env.def(
            "request_group",
            [](const dip::Environment& e, const std::string& path, const std::vector<std::string>& tags) {
                return e.request_group(path, dip::RequestType::Reference, tags);
            },
            py::arg("path"),
            py::arg("tags") = std::vector<std::string>{},
            R"doc(Return node snapshots with paths relative to the requested root.
A nonempty tags list matches any listed tag; an empty list imposes no
restriction. Raise an error if no nodes match.

Args:
    path: Reference query path.
    tags: Tags of which at least one must occur on each returned node.
)doc"
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
            R"doc(Return the value at a path, optionally converted to a NumPy array.

Args:
    path: Reference query path.
    to_units: Requested output units; empty keeps the original units.
    as_numpy: Return a NumPy array when true.
)doc"
        );

        env.def(
            "__getitem__", &dip::Environment::operator[], py::arg("path"),
            "Return a Cursor for a known DIPL path.\n\nArgs:\n    path: Path to inspect."
        );

        // env.def("request_code", &dip::Environment::request_code, py::arg("source_name"));

        env.def("__str__", &dip::Environment::to_string);
        env.def("__repr__", &dip::Environment::to_string);
        env.def("to_string", &dip::Environment::to_string);
    }

} // namespace snt::bind::python
