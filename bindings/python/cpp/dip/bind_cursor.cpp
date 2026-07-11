#include "../bindings/python/cpp/val/bind_value_base.h"

#include <codecvt>
#include <locale>
#include <pybind11/numpy.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <snt/dip/cursor.h>
#include <snt/dip/environment.h>

namespace py = pybind11;

namespace snt::bind::python {

    namespace {

        std::vector<py::ssize_t> numpy_shape(const std::vector<size_t>& shape) {
            std::vector<py::ssize_t> result;
            result.reserve(shape.size());

            for (size_t d : shape) {
                result.push_back(static_cast<py::ssize_t>(d));
            }

            return result;
        }

        template <typename T>
        py::array_t<T> make_numpy_array(const std::vector<size_t>& shape, const std::vector<T>& values) {
            return py::array_t<T>(numpy_shape(shape), values.data());
        }

    } // namespace

    void init_cursor(py::module_& m) {

        auto val = py::class_<dip::Cursor, std::shared_ptr<dip::Cursor>>(m, "Cursor");

        val.def(py::init<const dip::Environment*, std::string_view>(), py::arg("env"), py::arg("path") = "");

        val.def("__str__", &dip::Cursor::to_string);

        val.def("get_group", &dip::Cursor::get_group, py::arg("path"));

        val.def("get_list", &dip::Cursor::get_list, py::arg("path"));

        val.def("get_map", &dip::Cursor::get_map, py::arg("path"));

        val.def_property_readonly("path", &dip::Cursor::get_path);

        val.def_property_readonly("shape", [](const dip::Cursor& self) { return self.get_shape(); });

        val.def_property_readonly("value", [](const dip::Cursor& self) -> py::object {
            return to_python_value(self.get_value());
        });

        val.def("to_numpy", [](const dip::Cursor& self) -> py::object { return to_numpy_value(self.get_value()); });

        val.def("to_string", &dip::Cursor::to_string);
    }

} // namespace snt::bind::python
