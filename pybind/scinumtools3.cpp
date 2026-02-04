#include <pybind11/pybind11.h>
#include <iostream>
namespace py = pybind11;

#ifdef ENABLE_SNT_PYBIND
void init_snt(py::module_&);
#endif
#ifdef ENABLE_PUQ_PYBIND
void init_puq(py::module_&);
#endif
#ifdef ENABLE_DIP_PYBIND
void init_dip(py::module_&);
#endif

PYBIND11_MODULE(scinumtools3, m) {

  m.doc() = "Scientific Numerical Tools v3";

  m.attr("__version__") = CODE_VERSION;

#ifdef ENABLE_SNT_PYBIND
  auto snt = m.def_submodule("snt", "Scinumtools common structures and settings");
  init_snt(snt);
#endif

#ifdef ENABLE_PUQ_PYBIND
  auto puq = m.def_submodule("puq", "Physical Units & Quantities");
  init_puq(puq);
#endif

#ifdef ENABLE_DIP_PYBIND
  auto dip = m.def_submodule("dip", "Dimensional Input Parameters");
  init_dip(dip);
#endif

}
