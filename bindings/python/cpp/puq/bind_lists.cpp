#include <pybind11/pybind11.h>
#include <snt/puq/lists.h>

namespace py = pybind11;

namespace snt::bind::python {

    void init_puq_lists(py::module_& m) {

        m.def(
            "prefixes",
            &puq::lists::prefixes,
            py::arg("json") = false,
            py::arg("format") = puq::UnitFormat(),
            "List unit prefixes."
        );
        m.def(
            "base_units",
            &puq::lists::base_units,
            py::arg("json") = false,
            py::arg("format") = puq::UnitFormat(),
            "List base units."
        );
        m.def(
            "derived_units",
            &puq::lists::derived_units,
            py::arg("json") = false,
            py::arg("format") = puq::UnitFormat(),
            "List derived units."
        );
        m.def("logarithmic_units", &puq::lists::logarithmic_units, py::arg("json") = false, "List logarithmic units.");
        m.def("temperature_units", &puq::lists::temperature_units, py::arg("json") = false, "List temperature units.");
        m.def(
            "constants",
            &puq::lists::constants,
            py::arg("json") = false,
            py::arg("format") = puq::UnitFormat(),
            "List physical constants."
        );
        m.def(
            "quantities",
            &puq::lists::quantities,
            py::arg("json") = false,
            py::arg("format") = puq::UnitFormat(),
            "List named physical quantities."
        );
        m.def("unit_systems", &puq::lists::unit_systems, py::arg("json") = false, "List available unit systems.");
    }

} // namespace snt::bind::python
