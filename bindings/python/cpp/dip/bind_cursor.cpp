#include "../bindings/python/cpp/val/bind_to_value.h"

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

        auto provenance = py::class_<dip::Provenance>(
            m, "Provenance", "Read-only DIPL source and citation provenance for a resolved value."
        );
        provenance.def_readonly("source_name", &dip::Provenance::source_name);
        provenance.def_readonly("source_line", &dip::Provenance::source_line);
        provenance.def_readonly("source_code", &dip::Provenance::source_code);
        provenance.def_readonly("metadata", &dip::Provenance::metadata);
        provenance.def_readonly("source", &dip::Provenance::source);

        auto k = py::enum_<dip::Path::Kind>(m, "PathKind", "Kind of path within a DIPL node hierarchy.");
        k.value("None", dip::Path::Kind::None);
        k.value("Empty", dip::Path::Kind::Empty);
        k.value("Group", dip::Path::Kind::Group);
        k.value("Map", dip::Path::Kind::Map);
        k.value("List", dip::Path::Kind::List);
        k.value("Item", dip::Path::Kind::Item);

        auto val = py::class_<dip::Cursor, std::shared_ptr<dip::Cursor>>(
            m, "Cursor", "Cursor for traversing and querying evaluated DIPL nodes."
        );

        val.def(
            py::init<const dip::Environment*, std::string_view>(),
            py::arg("env"),
            py::arg("path") = "",
            "Create a cursor at a DIPL path."
        );

        val.def(
            "__getitem__",
            py::overload_cast<std::string_view>(&dip::Cursor::operator[], py::const_),
            py::arg("name"),
            "Select a child by name."
        );

        val.def(
            "__getitem__",
            py::overload_cast<std::size_t>(&dip::Cursor::operator[], py::const_),
            py::arg("index"),
            "Select a child by index."
        );

        val.def(
            "__contains__",
            [](const dip::Cursor& self, const std::string& item) { return self.has_item(item); },
            py::arg("item"),
            "Return whether a named child exists."
        );

        val.def("elements", &dip::Cursor::elements, "Return child elements.");

        val.def(
            "items",
            [](const dip::Cursor& c) {
                py::list result;
                for (auto&& [key, value] : c.items()) {
                    result.append(py::make_tuple(key, value));
                }
                return result;
            },
            "Return child names and values as pairs."
        );

        val.def("has_item", &dip::Cursor::has_item, py::arg("name"), "Return whether a named child exists.");

        val.def_property_readonly("path", &dip::Cursor::get_path, "Path of this cursor in the DIPL environment.");

        val.def_property_readonly(
            "shape", [](const dip::Cursor& self) { return self.get_shape(); }, "Shape of the value at this path."
        );

        val.def_property_readonly(
            "value",
            [](const dip::Cursor& self) -> py::object { return to_python_value(self.get_value()); },
            "Python value stored at this path (scalar, string, list, or NumPy-compatible array)."
        );

        val.def_property_readonly(
            "units",
            [](const dip::Cursor& self) -> py::object {
                auto units = self.get_units();
                if (!units)
                    return py::none();
                return py::cast(*units);
            },
            "Unit string for a dimensional value, or None when the value is unitless."
        );

        val.def_property_readonly(
            "provenance", &dip::Cursor::get_provenance, "Source and citation provenance for the value at this path."
        );

        val.def_property_readonly(
            "metadata",
            [](const dip::Cursor& self) { return self.get_provenance().metadata; },
            "Documentation and provenance metadata for the value at this path."
        );

        val.def_property_readonly("kind", &dip::Cursor::get_kind, "Kind of DIPL path represented by this cursor.");

        val.def(
            "to_numpy",
            [](const dip::Cursor& self) -> py::object { return to_numpy_value(self.get_value()); },
            "Return the cursor value as a NumPy array."
        );

        val.def("__str__", &dip::Cursor::to_string, "Format the cursor as text.");
        val.def("__repr__", &dip::Cursor::to_string, "Format the cursor as text.");
        val.def("to_string", &dip::Cursor::to_string, "Format the cursor as text.");
    }

} // namespace snt::bind::python
