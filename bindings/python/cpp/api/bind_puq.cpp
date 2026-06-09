#include "snt/api/puq_convert.h"
#include "snt/api/puq_eval.h"
#include "snt/api/puq_info.h"
#include "snt/api/puq_list.h"

#include <pybind11/pybind11.h>

namespace py = pybind11;
using namespace snt;

void init_api_puq(py::module_& m) {

    {
        auto command = py::class_<api::PUQConvert>(m, "PUQConvert");

        command.def(py::init<const std::string&, const std::string&>(), py::arg("expr1"), py::arg("expr2"));

        command.def("argument_input_system", &api::PUQConvert::argument_input_system, py::arg("system"));

        command.def("argument_output_system", &api::PUQConvert::argument_output_system, py::arg("system"));

        command.def("argument_output_quantity", &api::PUQConvert::argument_output_quantity, py::arg("quantity"));

        command.def("execute", &api::PUQConvert::execute);
    }

    {
        auto command = py::class_<api::PUQEval>(m, "PUQEval");

        command.def(py::init<const std::string&>(), py::arg("expr"));

        command.def("argument_input_system", &api::PUQEval::argument_input_system, py::arg("isystem"));

        command.def("argument_output_system", &api::PUQEval::argument_output_system, py::arg("osystem"));

        command.def("argument_output_units", &api::PUQEval::argument_output_units, py::arg("units"));

        command.def("argument_output_quantity", &api::PUQEval::argument_output_quantity, py::arg("quantity"));

        command.def("execute", &api::PUQEval::execute);
    }

    {
        auto command = py::class_<api::PUQInfo>(m, "PUQInfo");

        command.def(py::init<const std::string&>(), py::arg("expr"));

        command.def("argument_input_system", &api::PUQInfo::argument_input_system, py::arg("isystem"));

        command.def("execute", &api::PUQInfo::execute);
    }

    {
        auto command = py::class_<api::PUQList>(m, "PUQList");

        command.def(py::init<const std::string&>(), py::arg("li") = "");

        command.def("argument_system", &api::PUQList::argument_system, py::arg("system"));

        command.def("execute", &api::PUQList::execute);
    }
}
