#include "../val/bind_from_value.h"
#include "../val/bind_to_value.h"

#include <codecvt>
#include <locale>
#include <pybind11/numpy.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <snt/core/string_format.h>
#include <snt/dip/exceptions.h>
#include <snt/dip/nodes/node_boolean.h>
#include <snt/dip/nodes/node_float.h>
#include <snt/dip/nodes/node_integer.h>
#include <snt/dip/nodes/node_string.h>
#include <snt/dip/nodes/node_value.h>
#include <snt/puq/quantity.h>
#include <snt/val/values_number.h>
#include <snt/val/values_string.h>

namespace py = pybind11;

namespace snt::bind::python {

    void init_value_node(py::module_& m) {

        auto val = py::class_<dip::ValueNode, std::shared_ptr<dip::ValueNode>>(m, "ValueNode");

        val.def(
            py::init(
                [](const std::string& path, py::object value, py::object units) -> std::shared_ptr<dip::ValueNode> {
                    std::shared_ptr<dip::ValueNode> node;
                    if (py::isinstance<py::list>(value)) { // 1-D list
                        node = from_python_list(path, value, units);
                    } else if (py::isinstance<py::array>(value)) { // NumPy array
                        node = from_python_array(path, value, units);
                    } else {
                        node = from_python_scalar(path, value, units);
                    }

                    if (node)
                        return node;
                    else
                        throw dip::PybindException(
                            "Invalid ValueNode type",
                            "A ValueNode value must have type bool, int, float, str, or list.",
                            "Use a bool, int, float, str, or list value.",
                            __FILE__,
                            __LINE__
                        );
                }
            ),
            py::arg("path"),
            py::arg("value"),
            py::arg("units") = py::none()
        );

        val.def("__str__", &dip::ValueNode::to_string, py::arg("format") = core::StringFormatType());

        val.def_property_readonly("name", [](const dip::ValueNode& self) -> const std::string& {
            return self.path.name;
        });

        val.def_property_readonly(
            "units",
            [](const dip::ValueNode& vnode) -> const puq::Quantity* { return vnode.units ? &(*vnode.units) : nullptr; },
            py::return_value_policy::reference_internal
        );

        val.def_property_readonly("value", [](const dip::ValueNode& vnode) -> py::object {
            return to_python_value(vnode.value);
        });

        val.def_property_readonly("shape", [](const dip::ValueNode& vnode) { return vnode.value->get_shape(); });

        val.def_property_readonly("dtype", [](const dip::ValueNode& vnode) { return vnode.value->get_dtype(); });

        val.def("to_string", &dip::ValueNode::to_string, py::arg("format") = core::StringFormatType());

        val.def("to_numpy", [](const dip::ValueNode& vnode) -> py::object { return to_numpy_value(vnode.value); });
    }

} // namespace snt::bind::python
