#include <pybind11/numpy.h>
#include <pybind11/pybind11.h>
#include <snt/core/string_format.h>

namespace py = pybind11;
using namespace snt;

void init_snt(py::module_& m) {

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
}
