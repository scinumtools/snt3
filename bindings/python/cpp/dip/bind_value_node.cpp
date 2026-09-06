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

        auto vdata = py::class_<dip::ValueNodeData>(m, "ValueNodeData");
        vdata.def(
            py::init([](py::object value, py::object units) {
                dip::ValueNodeData data;
                data.value = from_python(value);
                if (!units.is_none()) {
                    data.units = puq::Quantity(units.cast<std::string>());
                }
                return data;
            }),
            py::arg("value"),
            py::arg("units") = py::none()
        );

        auto val = py::class_<dip::ValueNode, std::shared_ptr<dip::ValueNode>>(m, "ValueNode");

        val.def(
            py::init(
                [](const std::string& path, py::object value, py::object units) -> std::shared_ptr<dip::ValueNode> {
                    // prepare value
                    val::BaseValue::PointerType val = from_python(value);
                    // prepare quantity
                    std::optional<puq::Quantity> quantity;
                    if (!units.is_none()) {
                        quantity = puq::Quantity(units.cast<std::string>());
                        if (quantity.has_value() && (val->get_dtype() == core::DataType::Boolean ||
                                                     val->get_dtype() == core::DataType::String)) {
                            throw dip::PybindException(
                                "Units not supported for value type",
                                "A value of data type `" + core::DataTypeNames[val->get_dtype()] +
                                    "` cannot have units. Units are only supported for integer "
                                    "and floating-point values.",
                                "Remove the units or provide a numeric value.",
                                __FILE__,
                                __LINE__
                            );
                        }
                    }
                    // prepare a value node
                    std::shared_ptr<dip::ValueNode> node;
                    switch (val->get_dtype()) {
                    case core::DataType::Boolean:
                        return std::make_shared<dip::BooleanNode>(path, std::move(val));
                    case core::DataType::Integer64:
                        return std::make_shared<dip::IntegerNode>(path, std::move(val), std::move(quantity));
                    case core::DataType::Float64:
                        return std::make_shared<dip::FloatNode>(path, std::move(val), std::move(quantity));
                    case core::DataType::String:
                        return std::make_shared<dip::StringNode>(path, std::move(val));
                    default:
                        throw dip::PybindException(
                            "Invalid ValueNode type",
                            "A ValueNode value must have type bool, int, float, str, or list.",
                            "Use a bool, int, float, str, or list value.",
                            __FILE__,
                            __LINE__
                        );
                    }
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
