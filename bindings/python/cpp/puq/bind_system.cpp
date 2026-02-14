#include <pybind11/pybind11.h>
#include <snt/puq.h>

namespace py = pybind11;
using namespace snt;

void init_puq_system(py::module_& m) {

  auto e = py::enum_<puq::SystemType>(m, "SystemType");
  for (auto sys : puq::SystemMap) {
    e.value(sys.second->SystemAbbrev.c_str(), sys.first);
  }
  e.export_values();
}
