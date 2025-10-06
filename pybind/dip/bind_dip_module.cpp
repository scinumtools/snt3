#include <pybind11/pybind11.h>
#include <snt/dip.h>

namespace py = pybind11;
using namespace snt;

void init_dip(py::module_& m) {

  auto rt = py::enum_<dip::RequestType>(m, "RequestType");
  rt.value("Function", dip::RequestType::Function);
  rt.value("Reference", dip::RequestType::Reference);
  rt.export_values();

  auto env = py::class_<dip::Environment>(m, "Environment");
  env.def(py::init<>());
  env.def("request_code", &dip::Environment::request_code, py::arg("source_name"));
  env.def("request_value", &dip::Environment::request_value, py::arg("request"), py::arg("rtype"), py::arg("to_units") = "");
  env.def("request_nodes", &dip::Environment::request_nodes, py::arg("request"), py::arg("rtype"));
  env.def("get_value", &dip::Environment::get_value, py::arg("index"));

  auto dip = py::class_<dip::DIP>(m, "DIP");
  dip.def(py::init<>());
  dip.def("add_string", &dip::DIP::add_string, py::arg("source_code"));
  dip.def("add_file", &dip::DIP::add_file, py::arg("source_file"), py::arg("source_name") = "", py::arg("absolute") = true);
  dip.def("add_source", &dip::DIP::add_source, py::arg("source_name"), py::arg("source_file"));
  dip.def("add_unit", &dip::DIP::add_unit, py::arg("name"), py::arg("value"), py::arg("unit") = "");
  //.def("add_value_function", &dip::DIP::add_value_function, py::arg("name"), py::arg(""))
  //.def("add_node_function", &dip::DIP::add_node_function, py::arg("name"), py::arg(""))
  dip.def("parse", &dip::DIP::parse);
  // dip.def("parse_docs", &dip::DIP::parse_docs);
  dip.def("to_string", &dip::DIP::to_string);
}
