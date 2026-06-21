#include <pybind11/numpy.h>
#include <pybind11/pybind11.h>
#include <snt/dip/dip.h>
#include <snt/dip/environment.h>

namespace py = pybind11;
using namespace snt;

void init_value_base(py::module_& m);

void init_val(py::module_& m) {

    init_value_base(m);
};
