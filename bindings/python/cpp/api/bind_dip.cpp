#include "snt/api/dip_parse.h"

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

namespace py = pybind11;

namespace snt::bind::python {

    void init_api_dip(py::module_& m) {

        auto command = py::class_<api::DIPParse>(m, "DIPParse");

        command.def(py::init<>());

        command.def("argument_input", &api::DIPParse::argument_input, py::arg("input_type"), py::arg("input_value"));

        command.def("argument_request", &api::DIPParse::argument_request, py::arg("path"));

        command.def("argument_tags", &api::DIPParse::argument_tags, py::arg("tags"));

        command.def("argument_print", &api::DIPParse::argument_print);

        command.def("execute", &api::DIPParse::execute);
    }

} // namespace snt::bind::python
