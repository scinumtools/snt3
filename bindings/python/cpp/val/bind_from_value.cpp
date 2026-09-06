#include "../bindings/python/cpp/val/bind_from_value.h"

#include <pybind11/numpy.h>
#include <pybind11/pybind11.h>
#include <snt/dip/nodes/node_boolean.h>
#include <snt/dip/nodes/node_float.h>
#include <snt/dip/nodes/node_integer.h>
#include <snt/dip/nodes/node_string.h>
#include <snt/dip/nodes/node_value.h>
#include <snt/dip/settings.h>
#include <snt/val/exceptions.h>
#include <snt/val/value_base.h>
#include <snt/val/values_number.h>
#include <snt/val/values_string.h>

namespace py = pybind11;

namespace snt::bind::python {

    void init_from_value(py::module_& m) {}

    val::BaseValue::PointerType from_python(py::object value) {
        val::BaseValue::PointerType val;
        if (py::isinstance<py::list>(value)) { // 1-D list
            val = from_python_list(value);
        } else if (py::isinstance<py::array>(value)) { // NumPy array
            val = from_python_array(value);
        } else {
            val = from_python_scalar(value);
        }
        if (!val)
            throw dip::PybindException(
                "Invalid Python value",
                "The provided Python value could not be converted to a supported value type.",
                "Provide a valid scalar, list, or NumPy array.",
                __FILE__,
                __LINE__
            );
        return std::move(val);
    }

    val::BaseValue::PointerType from_python_scalar(py::object value) {

        // Scalar bool
        if (py::isinstance<py::bool_>(value))
            return std::make_unique<val::ArrayValue<uint8_t>>(value.cast<bool>() ? 1 : 0, core::DataType::Boolean);
        // Scalar integer
        if (py::isinstance<py::int_>(value))
            return std::make_unique<val::ArrayValue<int64_t>>(value.cast<int64_t>(), core::DataType::Integer64);
        // Scalar float
        if (py::isinstance<py::float_>(value))
            return std::make_unique<val::ArrayValue<double>>(value.cast<double>(), core::DataType::Float64);
        // Scalar string
        if (py::isinstance<py::str>(value))
            return std::make_unique<val::ArrayValue<std::string>>(value.cast<std::string>(), core::DataType::String);

        return nullptr;
    }

    val::BaseValue::PointerType from_python_list(py::object value) {
        py::list list = value.cast<py::list>();

        if (list.empty()) {
            throw dip::PybindException(
                "ValueNode list cannot be empty",
                "A ValueNode list must contain at least one value.",
                "Add at least one value to the list.",
                __FILE__,
                __LINE__
            );
        }

        py::handle first = list[0];

        // List of bool
        if (py::isinstance<py::bool_>(first)) {
            std::vector<uint8_t> data;
            data.reserve(list.size());
            for (py::handle item : list) {
                if (!py::isinstance<py::bool_>(item)) {
                    throw dip::PybindException(
                        "Mixed types in ValueNode list",
                        "All elements of a boolean ValueNode list must have type bool.",
                        "Ensure that every element in the list is a bool value.",
                        __FILE__,
                        __LINE__
                    );
                }
                data.push_back(item.cast<bool>() ? 1 : 0);
            }
            return std::make_unique<val::ArrayValue<uint8_t>>(data, core::DataType::Boolean);
        }

        // List of int
        if (py::isinstance<py::int_>(first)) {
            std::vector<int64_t> data;
            data.reserve(list.size());
            for (py::handle item : list) {
                // bool must not be accepted as int
                if (!py::isinstance<py::int_>(item) || py::isinstance<py::bool_>(item)) {
                    throw dip::PybindException(
                        "Mixed types in ValueNode list",
                        "All elements of an integer ValueNode list must have type int; bool values are "
                        "not accepted as integers.",
                        "Ensure that every element in the list is an int value and does not have type "
                        "bool.",
                        __FILE__,
                        __LINE__
                    );
                }
                data.push_back(item.cast<int64_t>());
            }
            return std::make_unique<val::ArrayValue<int64_t>>(data, core::DataType::Integer64);
        }

        // List of float
        if (py::isinstance<py::float_>(first)) {
            std::vector<double> data;
            data.reserve(list.size());
            for (py::handle item : list) {
                if (!py::isinstance<py::float_>(item)) {
                    throw dip::PybindException(
                        "Mixed types in ValueNode list",
                        "All elements of a floating-point ValueNode list must have type float.",
                        "Ensure that every element in the list is a float value.",
                        __FILE__,
                        __LINE__
                    );
                }

                data.push_back(item.cast<double>());
            }
            return std::make_unique<val::ArrayValue<double>>(data, core::DataType::Float64);
        }

        // List of string
        if (py::isinstance<py::str>(first)) {
            std::vector<std::string> data;
            data.reserve(list.size());
            for (py::handle item : list) {
                if (!py::isinstance<py::str>(item)) {
                    throw dip::PybindException(
                        "Mixed types in ValueNode list",
                        "All elements of a string ValueNode list must have type str.",
                        "Ensure that every element in the list is a str value.",
                        __FILE__,
                        __LINE__
                    );
                }
                data.push_back(item.cast<std::string>());
            }
            return std::make_unique<val::ArrayValue<std::string>>(data, core::DataType::String);
        }
        throw dip::PybindException(
            "Invalid ValueNode list type",
            "ValueNode list elements must have type bool, int, float, or str.",
            "Use a list containing only bool, int, float, or str values.",
            __FILE__,
            __LINE__
        );
        return nullptr;
    }

    val::BaseValue::PointerType from_python_array(py::object value) {
        py::array array = value.cast<py::array>();
        py::buffer_info info = array.request();

        if (array.dtype().is(py::dtype::of<bool>())) {

            // booleans
            const bool* ptr = static_cast<const bool*>(info.ptr);
            std::vector<uint8_t> data(info.size);
            for (size_t i = 0; i < static_cast<size_t>(info.size); ++i) {
                data[i] = ptr[i] ? 1 : 0;
            }
            val::Array::ShapeType shape(info.shape.begin(), info.shape.end());
            return std::make_unique<val::ArrayValue<uint8_t>>(data, shape, core::DataType::Boolean);

        } else if (array.dtype().is(py::dtype::of<int64_t>())) {

            // integers
            py::array_t<int64_t, py::array::c_style | py::array::forcecast> int_array(array);
            py::buffer_info int_info = int_array.request();
            const int64_t* ptr = static_cast<const int64_t*>(int_info.ptr);
            std::vector<int64_t> data(ptr, ptr + int_info.size);
            val::Array::ShapeType shape(int_info.shape.begin(), int_info.shape.end());
            return std::make_unique<val::ArrayValue<int64_t>>(data, shape, core::DataType::Integer64);

        } else if (array.dtype().is(py::dtype::of<double>())) {

            // floating-point values
            const double* ptr = static_cast<const double*>(info.ptr);
            std::vector<double> data(ptr, ptr + info.size);
            val::Array::ShapeType shape(info.shape.begin(), info.shape.end());
            return std::make_unique<val::ArrayValue<double>>(data, shape, core::DataType::Float64);

        } else if (array.dtype().kind() == 'U' || array.dtype().kind() == 'S') {

            // strings
            py::array flat = array.attr("reshape")(py::make_tuple(-1));
            std::vector<std::string> data;
            data.reserve(info.size);
            for (py::handle item : flat) {
                data.push_back(item.cast<std::string>());
            }
            val::Array::ShapeType shape(info.shape.begin(), info.shape.end());
            return std::make_unique<val::ArrayValue<std::string>>(data, shape, core::DataType::String);

        } else {
            throw dip::PybindException(
                "Unsupported NumPy data type",
                "The NumPy array has an unsupported data format: `" + info.format + "`.",
                "Provide a NumPy array with dtype `bool`, `int64`, or `float64`.",
                __FILE__,
                __LINE__
            );
        }

        return nullptr;
    }

} // namespace snt::bind::python
