#include "snt/api/puq_convert.h"
#include "snt/api/puq_eval.h"
#include "snt/api/puq_info.h"
#include "snt/api/puq_list.h"

#include <pybind11/pybind11.h>

namespace py = pybind11;

namespace snt::bind::python {

    void init_api_puq(py::module_& m) {

        {
            auto command = py::class_<api::PUQConvert>(m, "PUQConvert", "Command object for converting a PUQ quantity between units.");

            command.def(py::init<const std::string&, const std::string&>(), py::arg("expr1"), py::arg("expr2"), "Create a quantity conversion command.");

            command.def("argument_input_system", &api::PUQConvert::argument_input_system, py::arg("system"), "Set the input unit system.");

            command.def("argument_output_system", &api::PUQConvert::argument_output_system, py::arg("system"), "Set the output unit system.");

            command.def("argument_output_quantity", &api::PUQConvert::argument_output_quantity, py::arg("quantity"), "Set the physical quantity used for conversion.");

            command.def("execute", &api::PUQConvert::execute, "Convert and return the requested quantity.");
        }

        {
            auto command = py::class_<api::PUQEval>(m, "PUQEval", "Command object for evaluating a PUQ expression.");

            command.def(py::init<const std::string&>(), py::arg("expr"), "Create an expression evaluation command.");

            command.def("argument_input_system", &api::PUQEval::argument_input_system, py::arg("isystem"), "Set the input unit system.");

            command.def("argument_output_system", &api::PUQEval::argument_output_system, py::arg("osystem"), "Set the output unit system.");

            command.def("argument_output_units", &api::PUQEval::argument_output_units, py::arg("units"), "Set output units.");

            command.def("argument_output_quantity", &api::PUQEval::argument_output_quantity, py::arg("quantity"), "Set the output physical quantity.");

            command.def("execute", &api::PUQEval::execute, "Evaluate and return the PUQ expression.");
        }

        {
            auto command = py::class_<api::PUQInfo>(m, "PUQInfo", "Command object for querying PUQ expression information.");

            command.def(py::init<const std::string&>(), py::arg("expr"), "Create an expression information command.");

            command.def("argument_input_system", &api::PUQInfo::argument_input_system, py::arg("isystem"), "Set the input unit system.");

            command.def("execute", &api::PUQInfo::execute, "Return information about the PUQ expression.");
        }

        {
            auto command = py::class_<api::PUQList>(m, "PUQList", "Command object for listing available PUQ definitions.");

            command.def(py::init<const std::string&>(), py::arg("li") = "", "Create a unit-list command.");

            command.def("argument_system", &api::PUQList::argument_system, py::arg("system"), "Set the unit system for the listing.");

            command.def("execute", &api::PUQList::execute, "Return the requested PUQ definition list.");
        }
    }

} // namespace snt::bind::python
