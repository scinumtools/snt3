#include <pybind11/pybind11.h>
#include <snt/puq.h>

namespace py = pybind11;
using namespace snt;

void init_puq_lists(py::module_& m) {

  m.def("prefixes", &puq::lists::prefixes, py::arg("json") = false, py::arg("format") = snt::puq::UnitFormat());
  m.def("base_units", &puq::lists::base_units, py::arg("json") = false, py::arg("format") = snt::puq::UnitFormat());
  m.def("derived_units", &puq::lists::derived_units, py::arg("json") = false, py::arg("format") = snt::puq::UnitFormat());
  m.def("logarithmic_units", &puq::lists::logarithmic_units, py::arg("json") = false);
  m.def("temperature_units", &puq::lists::temperature_units, py::arg("json") = false);
  m.def("constants", &puq::lists::constants, py::arg("json") = false, py::arg("format") = snt::puq::UnitFormat());
  m.def("quantities", &puq::lists::quantities, py::arg("json") = false, py::arg("format") = snt::puq::UnitFormat());
  m.def("unit_systems", &puq::lists::unit_systems, py::arg("json") = false);
}
