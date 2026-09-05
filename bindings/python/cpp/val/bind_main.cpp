#include <pybind11/numpy.h>
#include <pybind11/pybind11.h>
#include <snt/dip/dip.h>
#include <snt/dip/environment.h>

namespace py = pybind11;

namespace snt::bind::python {

    void init_to_value(py::module_& m);

    void init_from_value(py::module_& m);

    void init_val(py::module_& m) {

        init_to_value(m);
    };

} // namespace snt::bind::python
