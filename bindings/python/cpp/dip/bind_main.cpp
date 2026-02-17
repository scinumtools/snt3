#include <pybind11/numpy.h>
#include <pybind11/pybind11.h>
#include <snt/dip/environment.h>
#include <snt/dip/dip.h>
#include <codecvt>
#include <locale>

namespace py = pybind11;
using namespace snt;

void init_environment(py::module_& m);
void init_value_node(py::module_& m);

void init_dip(py::module_& m) {

  auto rt = py::enum_<dip::RequestType>(m, "RequestType");
  rt.value("Function", dip::RequestType::Function);
  rt.value("Reference", dip::RequestType::Reference);
  rt.export_values();

  init_value_node(m);
  init_environment(m);
  
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
