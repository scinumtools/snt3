#include "../bindings/python/cpp/val/bind_value_base.h"

#include <pybind11/numpy.h>
#include <pybind11/pybind11.h>
#include <snt/val/value_base.h>
#include <snt/val/values_number.h>
#include <snt/val/values_string.h>

namespace py = pybind11;
using namespace snt;

void init_value_base(py::module_& m) {}

py::object to_python_value(const val::BaseValue::PointerType& value) {
    if (value == nullptr)
        return py::none();

    std::vector<size_t> shape = value->get_shape();
    std::vector<py::ssize_t> strides(shape.size());

    switch (value->get_dtype()) {
    case core::DataType::Boolean: {
        val::ArrayValue<uint8_t>* val = dynamic_cast<val::ArrayValue<uint8_t>*>(value.get());
        if (!val) {
            throw std::runtime_error("Type mismatch: uint8_t");
        } else if (val->get_size() == 0) { // return none value
            return py::none();
        } else if (val->get_size() == 1) { // return as a scalar
            return py::bool_(static_cast<bool>(val->get_value(0)));
        } else { // return as a list
            size_t index = 0;
            return make_nested_list(val->get_data(), shape, 0, index, [](uint8_t v) {
                return py::bool_(static_cast<bool>(v));
            });
        }
    }
    case core::DataType::Integer16:
    case core::DataType::Integer32:
    case core::DataType::Integer64:
    case core::DataType::Integer16_U:
    case core::DataType::Integer32_U:
    case core::DataType::Integer64_U:
    case core::DataType::Integer: {
        val::ArrayValue<int64_t>* val = dynamic_cast<val::ArrayValue<int64_t>*>(value.get());
        if (!val) {
            throw std::runtime_error("Type mismatch: int64_t");
        } else if (val->get_size() == 0) { // return none value
            return py::none();
        } else if (val->get_size() == 1) { // return as a scalar
            return py::int_(val->get_value(0));
        } else { // return as a list
            size_t index = 0;
            return make_nested_list(val->get_data(), shape, 0, index, [](int64_t v) { return py::int_(v); });
        }
    }
    case core::DataType::Float32:
    case core::DataType::Float64:
    case core::DataType::Float: {
        val::ArrayValue<double>* val = dynamic_cast<val::ArrayValue<double>*>(value.get());
        if (!val) {
            throw std::runtime_error("Type mismatch: double");
        } else if (val->get_size() == 0) { // return none value
            return py::none();
        } else if (val->get_size() == 1) { // return as a scalar
            return py::float_(val->get_value(0));
        } else { // return as a list
            size_t index = 0;
            return make_nested_list(val->get_data(), shape, 0, index, [](double v) { return py::float_(v); });
        }
    }
    case core::DataType::String: {
        val::ArrayValue<std::string>* val = dynamic_cast<val::ArrayValue<std::string>*>(value.get());
        if (!val) {
            throw std::runtime_error("Type mismatch: string");
        } else if (val->get_size() == 0) { // return none value
            return py::none();
        } else if (val->get_size() == 1) { // return as a scalar
            return py::str(val->get_value(0));
        } else { // return as a list
            size_t index = 0;
            return make_nested_list(val->get_data(), shape, 0, index, [](std::string v) { return py::str(v); });
        }
    }
    default:
        throw py::type_error("Unknown dtype in Environment");
    }
}

py::object to_numpy_value(const val::BaseValue::PointerType& value) {
    if (value == nullptr)
        return py::none();

    std::vector<size_t> shape = value->get_shape();
    std::vector<py::ssize_t> strides(shape.size());

    switch (value->get_dtype()) {
    case core::DataType::Boolean: {
        val::ArrayValue<uint8_t>* val = dynamic_cast<val::ArrayValue<uint8_t>*>(value.get());
        if (!val)
            throw std::runtime_error("Type mismatch");
        if (val->get_size() == 0) { // empty array
            return py::array_t<bool>(0);
        } else if (val->get_size() == 1) { // scalar array
            auto result = py::array_t<bool>(shape);
            *result.mutable_data() = static_cast<bool>(val->get_value(0));
            return result;
        } else { // normal array
            auto result = py::array_t<bool>(shape);
            std::transform(val->get_data(), val->get_data() + val->get_size(), result.mutable_data(), [](uint8_t x) {
                return static_cast<bool>(x);
            });
            return result;
        }
    }
    case core::DataType::Integer16:
    case core::DataType::Integer32:
    case core::DataType::Integer64:
    case core::DataType::Integer16_U:
    case core::DataType::Integer32_U:
    case core::DataType::Integer64_U:
    case core::DataType::Integer: {
        val::ArrayValue<int64_t>* val = dynamic_cast<val::ArrayValue<int64_t>*>(value.get());
        if (!val) {
            throw std::runtime_error("Type mismatch");
        } else if (val->get_size() == 0) { // return none value
            return py::array_t<int64_t>(0);
        } else if (val->get_size() == 1) { // scalar array
            auto result = py::array_t<int64_t>({});
            *result.mutable_data() = val->get_value(0);
            return result;
        } else { // return numpy array
            auto result = py::array_t<int64_t>(shape);
            std::copy(val->get_data(), val->get_data() + val->get_size(), result.mutable_data());
            return result;
        }
    }
    case core::DataType::Float32:
    case core::DataType::Float64:
    case core::DataType::Float: {
        val::ArrayValue<double>* val = dynamic_cast<val::ArrayValue<double>*>(value.get());
        if (!val) {
            throw std::runtime_error("Type mismatch");
        } else if (val->get_size() == 0) { // return none value
            return py::array_t<double>(0);
        } else if (val->get_size() == 1) { // scalar array
            auto result = py::array_t<double>(shape);
            *result.mutable_data() = val->get_value(0);
            return result;
        } else { // return numpy array
            auto result = py::array_t<double>(shape);
            std::copy(val->get_data(), val->get_data() + val->get_size(), result.mutable_data());
            return result;
        }
    }
    case core::DataType::String: {
        val::ArrayValue<std::string>* val = dynamic_cast<val::ArrayValue<std::string>*>(value.get());
        if (!val) {
            throw std::runtime_error("Type mismatch");
        } else if (val->get_size() == 0) { // return none value
            py::module_ np = py::module_::import("numpy");
            return np.attr("array")(py::list());
        } else if (val->get_size() == 1) { // scalar array
            py::module_ np = py::module_::import("numpy");
            return np.attr("array")(py::str(val->get_value(0)));
        } else { // return numpy array
            py::list list;
            // NOTE: For string-array conversion I could not find an alternative
            //       implementation that would work both on clang/macos and gcc/linux
            //       so for now we stick with this one.
            for (size_t i = 0; i < val->get_size(); i++)
                list.append(py::str(val->get_value(i)));
            py::tuple t(shape.size());
            for (size_t i = 0; i < shape.size(); ++i)
                t[i] = py::cast(shape[i]);
            py::module np = py::module::import("numpy");
            py::object arr = np.attr("array")(list);
            py::object reshaped = arr.attr("reshape")(t);
            return reshaped.cast<py::array>();
        }
    }
    default:
        throw py::type_error("Unknown dtype in Environment");
    }
}
