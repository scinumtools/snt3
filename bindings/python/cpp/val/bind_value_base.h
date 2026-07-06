#ifndef BINDIGS_PYTHON_DIP_VALUE_BASE_H
#define BINDIGS_PYTHON_DIP_VALUE_BASE_H

#include <pybind11/numpy.h>
#include <pybind11/pybind11.h>
#include <snt/val/value_base.h>
#include <snt/val/values_number.h>
#include <snt/val/values_string.h>

namespace py = pybind11;
using namespace snt;

py::object to_python_value(const val::BaseValue::PointerType& value);

py::object to_numpy_value(const val::BaseValue::PointerType& value);

#endif // BINDIGS_PYTHON_DIP_VALUE_BASE_H
