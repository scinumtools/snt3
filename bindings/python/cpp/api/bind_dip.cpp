#include "snt/api/dip_parse.h"

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

namespace py = pybind11;
using namespace snt;

void init_api_dip(py::module_& m) {

    auto command = py::class_<api::DIPParse>(m, "DIPParse");

    command.def(py::init<>());

    command.def("argument_file", &api::DIPParse::argument_file, py::arg("file_name"));

    command.def(
        "argument_string",
        py::overload_cast<const std::vector<std::string>&>(&api::DIPParse::argument_string),
        py::arg("list")
    );

    command.def(
        "argument_string", py::overload_cast<const std::string&>(&api::DIPParse::argument_string), py::arg("code")
    );

    command.def("argument_request", &api::DIPParse::argument_request, py::arg("path"));

    command.def("argument_tags", &api::DIPParse::argument_tags, py::arg("tags"));

    command.def("argument_print", &api::DIPParse::argument_print);

    command.def("execute", &api::DIPParse::execute);
}
