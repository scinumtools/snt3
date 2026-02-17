#include <pybind11/pybind11.h>
#include <snt/puq/unit_format.h>
#include <snt/puq/quantity.h>
#include <snt/puq/calc/calculator.h>

namespace py = pybind11;
using namespace snt;

void init_puq_system(py::module_&);
void init_puq_formats(py::module_&);
void init_puq_lists(py::module_&);
void init_puq_quantity(py::module_&);
void init_puq_unit_system(py::module_&);

void init_puq(py::module_& m) {

  // Exposing unit systems
  auto systems = m.def_submodule("systems", "Unit systems");
  init_puq_system(systems);

  // Exposing dimension formats
  auto formats = m.def_submodule("formats", "Dimension formats");
  init_puq_formats(formats);

  // Exposing unit format
  py::class_<snt::puq::UnitFormat>(m, "UnitFormat")
      .def(py::init<>()) // exposes default constructor
      ;

  // Exposing lists
  auto lists = m.def_submodule("lists", "Unit system lists");
  init_puq_lists(lists);

  // Exposing UnitSystem
  init_puq_unit_system(m);

  // Exposing calculator object
  m.def("Calculator", [](const std::string& e) -> puq::Quantity {
    auto calc = puq::Calculator();
    auto atom = calc.solve(e);
    return atom.value; }, py::arg("expression"));

  // Exposing Quantity object
  init_puq_quantity(m);
}
