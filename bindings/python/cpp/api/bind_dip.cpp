#include "snt/api/dip_parse.h"

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

namespace py = pybind11;

namespace snt::bind::python {

    void init_api_dip(py::module_& m) {

        auto command = py::class_<api::DIPParse>(m, "DIPParse", "Command object for parsing and querying DIPL input.");

        command.def(py::init<>(), "Create an empty DIPL command.");

        command.def(
            "argument_add",
            &api::DIPParse::argument_add,
            py::arg("add_type"),
            py::arg("add_value"),
            R"doc(Add a DIPfile project, file, inline code, named source, custom unit, or named schema.
schema_string and schema_file take a name and unwrapped body or file path.
override_string takes an unwrapped body; override_file takes its file path. Both may accompany a project.
A DIPfile may include schemas[] entries.

Args:
    add_type: project, file, string, override_string, override_file, source, unit, schema_string, or schema_file.
    add_value: One value for project/file/string/override_string/override_file; [name, value] for the others.
)doc"
        );

        command.def(
            "argument_request", &api::DIPParse::argument_request, py::arg("path"),
            "Select a DIPL node path.\n\nArgs:\n    path: Reference query to print or return as a scalar."
        );

        command.def(
            "argument_load",
            &api::DIPParse::argument_load,
            py::arg("file"),
            "Load a DIPH5 environment on execute(), instead of DIPL inputs.\n\n"
            "Args:\n    file: Input DIPH5 path."
        );
        command.def(
            "argument_save",
            &api::DIPParse::argument_save,
            py::arg("file"),
            "Save the full environment on execute(), overwriting the file; output filters do not limit the saved nodes.\n\n"
            "Args:\n    file: Output DIPH5 path."
        );
        command.def(
            "argument_generate",
            &api::DIPParse::argument_generate,
            py::arg("format"),
            py::arg("file"),
            "Generate static parameters on execute().\n\nArgs:\n"
            "    format: cpp, c, fortran, rust, julia, json, or yaml.\n"
            "    file: Output path; an existing file is overwritten."
        );

        command.def(
            "argument_tags",
            &api::DIPParse::argument_tags,
            py::arg("tags"),
            "Restrict printed output to nodes carrying at least one listed tag.\n\n"
            "Args:\n    tags: Explicit node tags to match with any-tag semantics."
        );

        command.def("argument_print", &api::DIPParse::argument_print, "Request named, formatted output.");

        command.def(
            "argument_value",
            &api::DIPParse::argument_value,
            py::arg("type") = "",
            "Request one defined, unitless scalar.\n\nArgs:\n"
            "    type: Optional required type: bool, integer, float, or string."
        );

        command.def("execute", &api::DIPParse::execute, "Execute the configured DIPL query.");
    }

} // namespace snt::bind::python
