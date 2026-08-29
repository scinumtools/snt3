#include "../bindings/python/cpp/val/bind_value_base.h"

#include <codecvt>
#include <locale>
#include <pybind11/numpy.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <snt/dip/cursor.h>
#include <snt/dip/environment.h>
#include <snt/dip/exceptions.h>

namespace py = pybind11;

namespace snt::bind::python {

    static PyObject* pybind_exception = nullptr;

    void init_exceptions(py::module_& m) {

        py::class_<dip::ExceptionInfo>(m, "ExceptionInfo")
            .def_readonly("message", &dip::ExceptionInfo::message)
            .def_readonly("details", &dip::ExceptionInfo::details)
            .def_readonly("suggestion", &dip::ExceptionInfo::suggestion);

        pybind_exception = py::register_exception<dip::PybindException>(m, "PybindException").ptr();
        py::register_exception_translator([](std::exception_ptr p) {
            try {
                std::rethrow_exception(p);
            } catch (const dip::PybindException& e) {
                py::object type = py::reinterpret_borrow<py::object>(pybind_exception);
                py::object instance = type(e.what());
                instance.attr("message") = e.info().message;
                instance.attr("details") = e.info().details;
                instance.attr("suggestion") = e.info().suggestion;
                PyErr_SetObject(type.ptr(), instance.ptr());
            }
        });
    }

} // namespace snt::bind::python
