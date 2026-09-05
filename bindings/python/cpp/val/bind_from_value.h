#ifndef BINDIGS_PYTHON_DIP_FROM_VALUE_H
#define BINDIGS_PYTHON_DIP_FROM_VALUE_H

#include <pybind11/numpy.h>
#include <pybind11/pybind11.h>
#include <snt/dip/nodes/node_value.h>
#include <snt/val/value_base.h>
#include <snt/val/values_number.h>

namespace py = pybind11;

namespace snt::bind::python {

    val::BaseValue::PointerType from_python_scalar(const std::string& path, py::object value, py::object units);

    val::BaseValue::PointerType from_python_list(const std::string& path, py::object value, py::object units);

    val::BaseValue::PointerType from_python_array(const std::string& path, py::object value, py::object units);

} // namespace snt::bind::python

#endif // BINDIGS_PYTHON_DIP_FROM_VALUE_H
