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
        py::class_<puq::UnitSystem> usb(m, "UnitSystemBase");
        usb.def(py::init<const puq::SystemType>());
        usb.def("change", &puq::UnitSystem::change);
        usb.def("close", &puq::UnitSystem::close);

        py::class_<UnitSystem, puq::UnitSystem> us(m, "UnitSystem");
        us.def(py::init<const puq::SystemType>());

        us.def("enter", &UnitSystem::enter);
        us.def("exit", &UnitSystem::exit);
        us.def("__enter__", &UnitSystem::enter);
        us.def("__exit__", &UnitSystem::exit);

        us.def("__repr__", &puq::UnitSystem::to_string);
        us.def("__str__", &puq::UnitSystem::to_string);
        us.def("to_string", &puq::UnitSystem::to_string);
    }

} // namespace snt::bind::python
