#include <pybind11/pybind11.h>
namespace py = pybind11;

void init_snt(py::module_&);
void init_dip(py::module_&);
void init_puq(py::module_&);

PYBIND11_MODULE(scinumtools3, m) {

  m.doc() = "Scientific Numerical Tools v3";

  m.attr("__version__") = CODE_VERSION;
  
  auto snt = m.def_submodule("snt", "Scinumtools common structures and settings");
  init_snt(snt);

  auto dip = m.def_submodule("dip", "Dimensional Input Parameters");
  init_dip(dip);

  auto puq = m.def_submodule("puq", "Physical Units & Quantities");
  init_puq(puq);
}
