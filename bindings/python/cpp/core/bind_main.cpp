#include <pybind11/numpy.h>
#include <pybind11/pybind11.h>
#include <snt/core/string_format.h>

namespace py = pybind11;

namespace snt::bind::python {

    void init_core(py::module_& m) {

        py::class_<core::StringFormatType>(m, "StringFormatType")
            .def(py::init<>()) // default constructor
            .def_readwrite("specifier", &core::StringFormatType::specifier)
            .def_readwrite("valuePrecision", &core::StringFormatType::valuePrecision)
            .def_readwrite("uncertaintyPrecision", &core::StringFormatType::uncertaintyPrecision)
            .def_readwrite("thresholdScientific", &core::StringFormatType::thresholdScientific)
            .def_readwrite("paddingZeros", &core::StringFormatType::paddingZeros)
            .def_readwrite("paddingSize", &core::StringFormatType::paddingSize)
            .def_readwrite("stringQuotes", &core::StringFormatType::stringQuotes)
            // Optional: provide a nice __repr__ for debugging in Python
            .def("__repr__", [](const core::StringFormatType& s) {
                return "<StringFormatType specifier='" + std::string(1, s.specifier) +
                       "', valuePrecision=" + std::to_string(s.valuePrecision) +
                       ", uncertaintyPrecision=" + std::to_string(s.uncertaintyPrecision) +
                       ", thresholdScientific=" + std::to_string(s.thresholdScientific) +
                       ", paddingZeros=" + std::string(s.paddingZeros ? "True" : "False") +
                       ", paddingSize=" + std::to_string(s.paddingSize) +
                       ", stringQuotes=" + std::string(s.stringQuotes ? "True" : "False") + ">";
            });

        py::enum_<core::DataType>(m, "DataType")
            .value("None", core::DataType::None)
            .value("Boolean", core::DataType::Boolean)
            .value("String", core::DataType::String)
            .value("Character", core::DataType::Character)
            .value("Integer16", core::DataType::Integer16)
            .value("Integer32", core::DataType::Integer32)
            .value("Integer64", core::DataType::Integer64)
            .value("IntegerX", core::DataType::IntegerX)
            .value("Integer16_U", core::DataType::Integer16_U)
            .value("Integer32_U", core::DataType::Integer32_U)
            .value("Integer64_U", core::DataType::Integer64_U)
            .value("Float32", core::DataType::Float32)
            .value("Float64", core::DataType::Float64)
            .value("Float128", core::DataType::Float128)
            .value("FloatX", core::DataType::FloatX)
            .value("Integer", core::DataType::Integer)
            .value("Integer_S", core::DataType::Integer_S)
            .value("Integer_U", core::DataType::Integer_U)
            .value("Float", core::DataType::Float);
    }

} // namespace snt::bind::python
