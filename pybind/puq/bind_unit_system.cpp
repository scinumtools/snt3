#include <pybind11/pybind11.h>
#include <snt/puq.h>

namespace py = pybind11;
using namespace snt;

// Adding Python context manager methods to UnitSystem
class UnitSystem : public puq::UnitSystem {
  using puq::UnitSystem::UnitSystem;

public:
  UnitSystem& enter() { return *this; }
  void exit(const py::object& exc_type, const py::object& exc_value, const py::object& traceback) { close(); }
};

void init_puq_unit_system(py::module_& m) {

  // Expose UnitSystem
  py::class_<puq::UnitSystem>(m, "UnitSystemBase")
      .def(py::init<const puq::SystemType>())
      .def("change", &puq::UnitSystem::change)
      .def("close", &puq::UnitSystem::close);

  py::class_<UnitSystem, puq::UnitSystem>(m, "UnitSystem")
      .def(py::init<const puq::SystemType>())
      .def("enter", &UnitSystem::enter)
      .def("exit", &UnitSystem::exit)
      .def("__enter__", &UnitSystem::enter)
      .def("__exit__", &UnitSystem::exit);
}
