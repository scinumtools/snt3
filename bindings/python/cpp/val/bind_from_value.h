#ifndef BINDIGS_PYTHON_DIP_FROM_VALUE_H
#define BINDIGS_PYTHON_DIP_FROM_VALUE_H

#include <pybind11/numpy.h>
#include <pybind11/pybind11.h>
#include <snt/dip/nodes/node_value.h>
#include <snt/val/value_base.h>
#include <snt/val/values_number.h>

namespace py = pybind11;

namespace snt::bind::python {

    /**
     * Parse a SNT array value from a python object
     * @param value Python object with a value
     * @return Pointer to a SNT array value
     */
    val::BaseValue::PointerType from_python(py::object value);

    /**
     * Parse a SNT array value from a python scalar object
     * @param value Python object with a value
     * @return Pointer to a SNT array value
     */
    val::BaseValue::PointerType from_python_scalar(py::object value);

    /**
     * Parse a SNT array value from a python list object
     * @param value Python object with a value
     * @return Pointer to a SNT array value
     */
    val::BaseValue::PointerType from_python_list(py::object value);

    /**
     * Parse a SNT array value from a numpy array object
     * @param value Python object with a value
     * @return Pointer to a SNT array value
     */
    val::BaseValue::PointerType from_python_array(py::object value);

} // namespace snt::bind::python

#endif // BINDIGS_PYTHON_DIP_FROM_VALUE_H
