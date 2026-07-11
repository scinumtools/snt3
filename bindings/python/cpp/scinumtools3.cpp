#include <iostream>
#include <pybind11/pybind11.h>
namespace py = pybind11;

namespace snt::bind::python {

#ifdef ENABLE_SNT_PYBIND
    void init_snt(py::module_&);
#endif
#ifdef ENABLE_VAL_PYBIND
    void init_val(py::module_&);
#endif
#ifdef ENABLE_PUQ_PYBIND
    void init_puq(py::module_&);
#endif
#ifdef ENABLE_DIP_PYBIND
    void init_dip(py::module_&);
#endif
#ifdef ENABLE_API_PYBIND
    void init_api(py::module_&);
#endif

    PYBIND11_MODULE(_core, m) {

        m.doc() = "Scientific Numerical Tools v3";

        m.attr("__version__") = CODE_VERSION;

#ifdef ENABLE_SNT_PYBIND
        {
            auto smod = m.def_submodule("snt", "Scinumtools common structures and settings");
            init_snt(smod);
        }
#endif

#ifdef ENABLE_VAL_PYBIND
        {
            auto smod = m.def_submodule("val", "Array values");
            init_val(smod);
        }
#endif

#ifdef ENABLE_PUQ_PYBIND
        {
            auto smod = m.def_submodule("puq", "Physical Units & Quantities");
            init_puq(smod);
        }
#endif

#ifdef ENABLE_DIP_PYBIND
        {
            auto smod = m.def_submodule("dip", "Dimensional Input Parameters");
            init_dip(smod);
        }
#endif

#ifdef ENABLE_API_PYBIND
        {
            auto smod = m.def_submodule("api", "Application programming interface");
            init_api(smod);
        }
#endif
    }

} // namespace snt::bind::python
