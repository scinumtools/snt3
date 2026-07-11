#include <pybind11/pybind11.h>

namespace py = pybind11;

namespace snt::bind::python {

    void init_api_dip(py::module_&);

    void init_api_puq(py::module_&);

    void init_api(py::module_& m) {

        // Exposing DIP API
        auto dip = m.def_submodule("dip", "Dimensional Input Parameters");
        init_api_dip(dip);

        // Exposing PUQ API
        auto puq = m.def_submodule("puq", "Physical Units & Quantities");
        init_api_puq(puq);
    }

} // namespace snt::bind::python
