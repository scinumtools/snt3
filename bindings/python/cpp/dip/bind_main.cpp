#include "../val/bind_from_value.h"

#include <codecvt>
#include <locale>
#include <pybind11/numpy.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/stl/filesystem.h>
#include <snt/dip/dip.h>
#include <snt/dip/environment.h>

namespace py = pybind11;

namespace snt::bind::python {

    void init_environment(py::module_& m);
    void init_value_node(py::module_& m);
    void init_cursor(py::module_& m);
    void init_inspection(py::module_& m);
    void init_exceptions(py::module_& m);
    void init_adapter(py::module_& m);
    void init_comparison(py::module_& m);

    // Adding Python context manager methods to DIP
    dip::DIP& dip_enter(dip::DIP& self) {
        return self;
    };
    void dip_exit(
        dip::DIP& self, const py::object& exc_type, const py::object& exc_value, const py::object& traceback
    ) {};

    void init_dip(py::module_& m) {

        auto rt = py::enum_<dip::RequestType>(m, "RequestType", "How a DIPL environment resolves a requested node.");
        rt.value("Function", dip::RequestType::Function);
        rt.value("Reference", dip::RequestType::Reference);
        rt.export_values();

        init_value_node(m);
        init_environment(m);
        init_cursor(m);
        init_inspection(m);
        init_exceptions(m);
        init_adapter(m);
        init_comparison(m);

        auto dip = py::class_<dip::DIP>(m, "DIP", "Parser and evaluator for DIPL source definitions.");
        dip.def(py::init<>(), "Create an empty DIPL parser.");
        dip.def(
            "add_string", &dip::DIP::add_string, py::arg("source_code"),
            "Add DIPL source text to the parser.\n\nArgs:\n    source_code: Complete DIPL text to parse."
        );
        dip.def(
            "add_schema_string", &dip::DIP::add_schema_string,
            py::arg("name"), py::arg("source_code"),
            R"doc(Register a named schema body without a $schema wrapper.
Leading ? metadata describe the schema definition; metadata on a member
follow that member into instances.

Args:
    name: Unique schema name.
    source_code: Schema body starting at indentation zero.
)doc"
        );
        dip.def(
            "add_schema_file", &dip::DIP::add_schema_file,
            py::arg("name"), py::arg("source_file"),
            R"doc(Register a named schema body from a file, with the same rules as add_schema_string().

Args:
    name: Unique schema name.
    source_file: Path to a file containing the unwrapped schema body.
)doc"
        );
        dip.def(
            "add_override_string", &dip::DIP::add_override_string, py::arg("source_code"),
            "Collect unwrapped value modifications before evaluating nodes. Empty bodies make no changes.\n\n"
            "Args:\n    source_code: DIPL modifications with dotted paths or nested path prefixes."
        );
        dip.def(
            "add_override_file", &dip::DIP::add_override_file, py::arg("source_file"),
            "Register an unwrapped override body from a file before evaluating nodes. Empty files make no changes.\n\n"
            "Args:\n    source_file: Path to an unwrapped body of modifications and optional nested path prefixes."
        );
        dip.def(
            "add_file",
            &dip::DIP::add_file,
            py::arg("source_file"),
            py::arg("source_name") = "",
            py::arg("absolute") = true,
            R"doc(Add a DIPL source file to the parser.

Args:
    source_file: Path to the DIPL file.
    source_name: Optional registered source name; generated when empty.
    absolute: Accepted for compatibility; currently has no effect.
)doc"
        );
        dip.def(
            "add_source",
            &dip::DIP::add_source,
            py::arg("source_name"),
            py::arg("source_file"),
            "Register a named DIPL source file.\n\nArgs:\n    source_name: Name used in references.\n"
            "    source_file: Path to the source file."
        );
        dip.def(
            "add_unit", &dip::DIP::add_unit, py::arg("name"), py::arg("unit"),
            "Register a custom unit definition.\n\nArgs:\n    name: Unit name.\n"
            "    unit: PUEL expression defining the unit."
        );
        dip.def(
            "add_project",
            &dip::DIP::add_project,
            py::arg("project_file"),
            R"doc(Add a DIPfile with units[], sources[], schemas[], overrides[], and ordered code[] entries.
Schema entries have a name and exactly one file or string body. Override
entries have a file containing value modifications without a $override wrapper.
Relative paths resolve from the DIPfile directory.

Args:
    project_file: Path to the DIPfile manifest.
)doc"
        );

        dip.def(
            "add_function_value",
            [](dip::DIP& self, const std::string& name, const py::function& func) {
                self.add_function_value(name, [func](const dip::Environment& env) {
                    py::gil_scoped_acquire gil;
                    return func(env).cast<dip::ValueNodeData>();
                });
            },
            py::arg("name"), py::arg("callback"),
            "Register a Python callback that returns value data for a DIPL function.\n\n"
            "Args:\n    name: Function name used in DIPL.\n"
            "    callback: Callable receiving an Environment and returning ValueNodeData."
        );

        dip.def(
            "add_function_nodes",
            [](dip::DIP& self, const std::string& name, const py::function& func) {
                self.add_function_nodes(name, [func](const dip::Environment& env) {
                    py::gil_scoped_acquire gil;
                    return func(env).cast<dip::ValueNode::ListType>();
                });
            },
            py::arg("name"), py::arg("callback"),
            "Register a Python callback that returns nodes for a DIPL function.\n\n"
            "Args:\n    name: Function name used in DIPL.\n"
            "    callback: Callable receiving an Environment and returning value nodes."
        );

        dip.def("parse", &dip::DIP::parse, py::arg("record_dependency_graph") = false,
                "Parse and evaluate DIPL inputs; retain a dependency graph when requested (default: False).");

        dip.def("enter", &dip_enter);
        dip.def("exit", &dip_exit);
        dip.def("__enter__", &dip_enter);
        dip.def("__exit__", &dip_exit);

        dip.def("__str__", &dip::DIP::to_string);
        dip.def("__repr__", &dip::DIP::to_string);
        dip.def("to_string", &dip::DIP::to_string);
    }

} // namespace snt::bind::python
