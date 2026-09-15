#include <pybind11/pybind11.h>
#include <snt/puq/systems/unit_system.h>

namespace py = pybind11;

namespace snt::bind::python {

    // Adding Python context manager methods to UnitSystem
    class UnitSystem : public puq::UnitSystem {
        using puq::UnitSystem::UnitSystem;

      public:
        UnitSystem& enter() { return *this; }
        void exit(const py::object& exc_type, const py::object& exc_value, const py::object& traceback) { close(); }
    };

    void init_puq_unit_system(py::module_& m) {

        // Expose UnitSystem
        py::class_<puq::UnitSystem> usb(m, "UnitSystemBase", "Base context manager for selecting a unit system.");
        usb.def(py::init<const puq::SystemType>(), py::arg("system"), "Select a unit system.");
        usb.def("change", &puq::UnitSystem::change, py::arg("system"), "Change the active unit system.");
        usb.def("close", &puq::UnitSystem::close, "Restore the previously active unit system.");

        py::class_<UnitSystem, puq::UnitSystem> us(m, "UnitSystem", "Context manager for changing the active unit system.");
        us.def(py::init<const puq::SystemType>(), py::arg("system"), "Create a unit-system context manager.");

        us.def("enter", &UnitSystem::enter, "Enter the unit-system context.");
        us.def("exit", &UnitSystem::exit, "Leave the unit-system context and restore the previous system.");
        us.def("__enter__", &UnitSystem::enter);
        us.def("__exit__", &UnitSystem::exit);

        us.def("__repr__", &puq::UnitSystem::to_string);
        us.def("__str__", &puq::UnitSystem::to_string);
        us.def("to_string", &puq::UnitSystem::to_string, "Return a textual description of the active unit system.");
    }

} // namespace snt::bind::python
