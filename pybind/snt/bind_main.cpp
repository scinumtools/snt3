#include <pybind11/numpy.h>
#include <pybind11/pybind11.h>
#include <snt/string_format.h>

namespace py = pybind11;
using namespace snt;

void init_snt(py::module_& m) {

  py::class_<snt::StringFormatType>(m, "StringFormatType")
    .def(py::init<>())  // default constructor
    .def_readwrite("specifier", &snt::StringFormatType::specifier)
    .def_readwrite("valuePrecision", &snt::StringFormatType::valuePrecision)
    .def_readwrite("errorPrecision", &snt::StringFormatType::errorPrecision)
    .def_readwrite("thresholdScientific", &snt::StringFormatType::thresholdScientific)
    .def_readwrite("paddingZeros", &snt::StringFormatType::paddingZeros)
    .def_readwrite("paddingSize", &snt::StringFormatType::paddingSize)
    .def_readwrite("stringQuotes", &snt::StringFormatType::stringQuotes)
    // Optional: provide a nice __repr__ for debugging in Python
    .def("__repr__", [](const snt::StringFormatType &s) {
      return "<StringFormatType specifier='" + std::string(1, s.specifier) +
	"', valuePrecision=" + std::to_string(s.valuePrecision) +
	", errorPrecision=" + std::to_string(s.errorPrecision) +
	", thresholdScientific=" + std::to_string(s.thresholdScientific) +
	", paddingZeros=" + std::string(s.paddingZeros ? "True" : "False") +
	", paddingSize=" + std::to_string(s.paddingSize) +
	", stringQuotes=" + std::string(s.stringQuotes ? "True" : "False") + ">";
    });
  
}
