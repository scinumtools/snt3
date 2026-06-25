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
    std::vector<size_t> shape = value->get_shape();
    std::vector<py::ssize_t> strides(shape.size());

    // Create a capsule to ensure lifetime safety
    auto capsule = py::capsule(value.get(), [](void*) {
        // Do nothing — environment owns the memory
    });

    // Compute strides correctly based on element type size
    auto compute_strides = [&](py::ssize_t elem_size) {
        std::vector<py::ssize_t> s(shape.size());
        py::ssize_t stride = elem_size;
        for (py::ssize_t j = static_cast<py::ssize_t>(shape.size()) - 1; j >= 0; --j) {
            s[j] = stride;
            stride *= static_cast<py::ssize_t>(shape[j]);
        }
        return s;
    };

    switch (value->get_dtype()) {
    case core::DataType::Boolean: {
        // TODO: in the future bool should be stored as uint8_t because bool is not 8 bit,
        // but 1 bit
        val::ArrayValue<uint8_t>* val = dynamic_cast<val::ArrayValue<uint8_t>*>(value.get());
        if (!val)
            throw std::runtime_error("Type mismatch");
        auto strides = compute_strides(sizeof(uint8_t));
        return py::array_t<uint8_t>(shape, strides, val->get_data(), capsule);
    }
    case core::DataType::Integer16:
    case core::DataType::Integer32:
    case core::DataType::Integer64:
    case core::DataType::Integer: {
        val::ArrayValue<int64_t>* val = dynamic_cast<val::ArrayValue<int64_t>*>(value.get());
        if (!val)
            throw std::runtime_error("Type mismatch");
        auto strides = compute_strides(sizeof(int64_t));
        py::array arr(py::dtype::of<int32_t>(), shape, strides, val->get_data(), capsule);
        arr.attr("flags").attr("writeable") = false;
        return arr;
    }
    case core::DataType::Float32:
    case core::DataType::Float64:
    case core::DataType::Float: {
        val::ArrayValue<double>* val = dynamic_cast<val::ArrayValue<double>*>(value.get());
        if (!val)
            throw std::runtime_error("Type mismatch");
        auto strides = compute_strides(sizeof(double));
        return py::array_t<double>(shape, strides, val->get_data(), capsule);
    }
    case core::DataType::String: {
        val::ArrayValue<std::string>* val = dynamic_cast<val::ArrayValue<std::string>*>(value.get());
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
    default:
        throw py::type_error("Unknown dtype in Environment");
    }
}
