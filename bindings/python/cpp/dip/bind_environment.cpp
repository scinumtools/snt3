#include <pybind11/numpy.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <snt/val.h>
#include <snt/dip.h>
#include <codecvt>
#include <locale>

namespace py = pybind11;
using namespace snt;

void init_environment(py::module_& m) {

  auto nl = py::class_<dip::NodeList<dip::ValueNode>>(m, "NodeList");
  nl.def(py::init<>());
  nl.def("__getitem__", [](const dip::NodeList<dip::ValueNode> &self, size_t i) {
    return self.at(i);
  }, py::arg("node"));
  
  auto env = py::class_<dip::Environment>(m, "Environment");
  env.def(py::init<>());
  env.def_property_readonly("nodes", [](const dip::Environment &e) {
    return &e.nodes;
  });
  env.def_property_readonly("size", [](const dip::Environment &e) {
    return e.nodes.size();
  });
  env.def("request", [](const dip::Environment &e, const std::string& path) {
    return e.request_nodes("?"+path);
  });
  //env.def("request_code", &dip::Environment::request_code, py::arg("source_name"));
  //env.def("request_value", &dip::Environment::request_value, py::arg("request"), py::arg("rtype"), py::arg("to_units") = "");
  //env.def("request_nodes", &dip::Environment::request_nodes, py::arg("request"), py::arg("rtype"));
  //env.def("get_value", &dip::Environment::get_value, py::arg("index"));

}
