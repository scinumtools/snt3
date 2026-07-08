#ifndef BINDIGS_PYTHON_DIP_VALUE_BASE_H
#define BINDIGS_PYTHON_DIP_VALUE_BASE_H

#include <pybind11/numpy.h>
#include <pybind11/pybind11.h>
#include <snt/val/value_base.h>
#include <snt/val/values_number.h>
#include <snt/val/values_string.h>

namespace py = pybind11;
using namespace snt;

template <typename T, typename Converter>
py::object make_nested_list(
    const T* data, const std::vector<size_t>& shape, size_t dim, size_t& index, Converter convert
) {
    if (dim == shape.size() - 1) {
        py::list list;
        for (size_t i = 0; i < shape[dim]; ++i)
            list.append(convert(data[index++]));
        return std::move(list);
    }

    py::list list;
    for (size_t i = 0; i < shape[dim]; ++i)
        list.append(make_nested_list(data, shape, dim + 1, index, convert));

    return std::move(list);
}

py::object to_python_value(const val::BaseValue::PointerType& value);

py::object to_numpy_value(const val::BaseValue::PointerType& value);

#endif // BINDIGS_PYTHON_DIP_VALUE_BASE_H
