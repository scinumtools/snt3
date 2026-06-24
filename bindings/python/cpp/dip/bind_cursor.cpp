#include <codecvt>
#include <locale>
#include <pybind11/numpy.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <snt/dip/cursor.h>
#include <snt/dip/environment.h>

namespace py = pybind11;
using namespace snt;

void init_cursor(py::module_& m) {

    auto val = py::class_<dip::Cursor, std::shared_ptr<dip::Cursor>>(m, "Cursor");

    val.def(py::init<const dip::Environment*, std::string_view>(), py::arg("env"), py::arg("path") = "");

    val.def("__str__", &dip::Cursor::to_string);

    val.def_property_readonly("path", &dip::Cursor::get_path);

    val.def("get_group", &dip::Cursor::get_group, py::arg("path"));

    val.def("get_list", &dip::Cursor::get_list, py::arg("path"));

    val.def("get_map", &dip::Cursor::get_map, py::arg("path"));

    val.def("get_value", &dip::Cursor::get_value);

    val.def("get_path", &dip::Cursor::get_path);

    val.def("to_string", &dip::Cursor::to_string);

    val.def(
        "get_scalar",
        [](const dip::Cursor& self, size_t index) -> py::object {
            auto value = self.get_value();

            if (auto* v = dynamic_cast<val::ArrayValue<int64_t>*>(value.get()))
                return py::cast(v->get_value(index));

            if (auto* v = dynamic_cast<val::ArrayValue<double>*>(value.get()))
                return py::cast(v->get_value(index));

            if (auto* v = dynamic_cast<val::ArrayValue<std::string>*>(value.get()))
                return py::cast(v->get_value(index));

            if (auto* v = dynamic_cast<val::ArrayValue<uint8_t>*>(value.get()))
                return py::bool_(v->get_value(index));

            throw std::runtime_error("Unsupported value type");
        },
        py::arg("index") = 0
    );

    val.def("get_vector", [](const dip::Cursor& self) -> py::object {
        auto value = self.get_value();

        if (auto* v = dynamic_cast<val::ArrayValue<int64_t>*>(value.get()))
            return py::cast(v->get_values());

        if (auto* v = dynamic_cast<val::ArrayValue<double>*>(value.get()))
            return py::cast(v->get_values());

        if (auto* v = dynamic_cast<val::ArrayValue<std::string>*>(value.get()))
            return py::cast(v->get_values());

        if (auto* v = dynamic_cast<val::ArrayValue<uint8_t>*>(value.get())) {
            std::vector<bool> result;
            result.reserve(v->get_values().size());

            for (auto x : v->get_values())
                result.push_back(x != 0);

            return py::cast(result);
        }
        throw std::runtime_error("Unsupported value type");
    });
}
