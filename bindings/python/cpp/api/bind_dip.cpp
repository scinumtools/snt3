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
            "Add a file or inline DIPL source."
        );

        command.def("argument_request", &api::DIPParse::argument_request, py::arg("path"), "Select a DIPL node path.");

        command.def(
            "argument_tags",
            &api::DIPParse::argument_tags,
            py::arg("tags"),
            "Restrict output to nodes carrying these tags."
        );

        command.def("argument_print", &api::DIPParse::argument_print, "Request named, formatted output.");

        command.def("execute", &api::DIPParse::execute, "Execute the configured DIPL query.");
    }

} // namespace snt::bind::python
