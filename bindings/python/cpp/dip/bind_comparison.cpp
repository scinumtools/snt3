#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/stl/filesystem.h>
#include <snt/dip/comparison.h>
#include <snt/dip/environment.h>

namespace py = pybind11;

namespace snt::bind::python {
    void init_comparison(py::module_& m) {
        py::enum_<dip::ComparisonScope>(m, "ComparisonScope")
            .value("Effective", dip::ComparisonScope::Effective)
            .value("Full", dip::ComparisonScope::Full);
        py::enum_<dip::DifferenceKind>(m, "DifferenceKind")
            .value("Added", dip::DifferenceKind::Added)
            .value("Removed", dip::DifferenceKind::Removed)
            .value("Changed", dip::DifferenceKind::Changed);
        py::class_<dip::ComparisonOptions>(m, "ComparisonOptions")
            .def(py::init<>())
            .def_readwrite("scope", &dip::ComparisonOptions::scope)
            .def_readwrite("max_array_examples", &dip::ComparisonOptions::max_array_examples);
        py::class_<dip::Difference>(m, "Difference")
            .def_readonly("path", &dip::Difference::path)
            .def_readonly("category", &dip::Difference::category)
            .def_readonly("kind", &dip::Difference::kind)
            .def_readonly("fields", &dip::Difference::fields)
            .def_readonly("before", &dip::Difference::before)
            .def_readonly("after", &dip::Difference::after)
            .def_readonly("changed_elements", &dip::Difference::changed_elements)
            .def_readonly("example_indices", &dip::Difference::example_indices);
        py::class_<dip::ComparisonResult>(m, "ComparisonResult")
            .def_readonly("scope", &dip::ComparisonResult::scope)
            .def_readonly("added", &dip::ComparisonResult::added)
            .def_readonly("removed", &dip::ComparisonResult::removed)
            .def_readonly("changed", &dip::ComparisonResult::changed)
            .def_readonly("differences", &dip::ComparisonResult::differences)
            .def_property_readonly("equal", &dip::ComparisonResult::equal);
        m.def(
            "compare",
            &dip::compare,
            py::arg("before"),
            py::arg("after"),
            py::arg("options") = dip::ComparisonOptions{},
            "Compare two loaded DIP environments."
        );
        m.def(
            "compare_diph5",
            &dip::compare_diph5,
            py::arg("before"),
            py::arg("after"),
            py::arg("options") = dip::ComparisonOptions{},
            "Load and compare two DIPH5 files."
        );
        m.def(
            "render_comparison",
            &dip::render_comparison,
            py::arg("result"),
            py::arg("max_details") = 50,
            "Render a bounded plain-text comparison."
        );
    }
} // namespace snt::bind::python
