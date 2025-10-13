#include <pybind11/numpy.h>
#include <pybind11/pybind11.h>
#include <snt/val.h>
#include <snt/dip.h>
#include <codecvt>
#include <locale>

namespace py = pybind11;
using namespace snt;

void init_environment(py::module_& m) {

  auto env = py::class_<dip::Environment>(m, "Environment");
  env.def(py::init<>());
  env.def("__getitem__", [](const dip::Environment &e, size_t i) {
    return e.nodes.at(i);
  }, py::arg("node"));
  env.def_property_readonly("size", [](const dip::Environment &e) {
    return e.nodes.size();
  });
  env.def("request", [](const dip::Environment &e, const std::string& path) {
    return e.request_value(path);
  });
  //env.def("request_code", &dip::Environment::request_code, py::arg("source_name"));
  //env.def("request_value", &dip::Environment::request_value, py::arg("request"), py::arg("rtype"), py::arg("to_units") = "");
  //env.def("request_nodes", &dip::Environment::request_nodes, py::arg("request"), py::arg("rtype"));
  //env.def("get_value", &dip::Environment::get_value, py::arg("index"));

}
