#include "../bindings/python/cpp/val/bind_value_base.h"

#include <codecvt>
#include <locale>
#include <pybind11/numpy.h>
#include <pybind11/pybind11.h>
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
                    std::optional<puq::Quantity> quantity;

                    if (!units.is_none())
                        quantity = puq::Quantity(units.cast<std::string>());

                    // Scalar bool
                    if (py::isinstance<py::bool_>(value)) {
                        return std::make_shared<dip::BooleanNode>(
                            path,
                            std::make_unique<val::ArrayValue<uint8_t>>(
                                value.cast<bool>() ? 1 : 0, core::DataType::Boolean
                            )
                        );
                    }

                    // Scalar integer
                    if (py::isinstance<py::int_>(value)) {
                        return std::make_shared<dip::IntegerNode>(
                            path,
                            std::make_unique<val::ArrayValue<int64_t>>(
                                value.cast<int64_t>(), core::DataType::Integer64
                            ),
                            std::move(quantity)
                        );
                    }

                    // Scalar float
                    if (py::isinstance<py::float_>(value)) {
                        return std::make_shared<dip::FloatNode>(
                            path,
                            std::make_unique<val::ArrayValue<double>>(value.cast<double>(), core::DataType::Float64),
                            std::move(quantity)
                        );
                    }

                    // Scalar string
                    if (py::isinstance<py::str>(value)) {
                        return std::make_shared<dip::StringNode>(
                            path,
                            std::make_unique<val::ArrayValue<std::string>>(
                                value.cast<std::string>(), core::DataType::String
                            )
                        );
                    }

                    // 1-D list
                    if (py::isinstance<py::list>(value)) {
                        py::list list = value.cast<py::list>();

                        if (list.empty()) {
                            throw dip::PybindException(
                                "ValueNode list cannot be empty",
                                "A ValueNode list must contain at least one value.",
                                "Add at least one value to the list.",
                                __FILE__,
                                __LINE__
                            );
                        }

                        py::handle first = list[0];

                        // List of bool
                        if (py::isinstance<py::bool_>(first)) {
                            std::vector<uint8_t> data;
                            data.reserve(list.size());
                            for (py::handle item : list) {
                                if (!py::isinstance<py::bool_>(item)) {
                                    throw dip::PybindException(
                                        "Mixed types in ValueNode list",
                                        "All elements of a boolean ValueNode list must have type bool.",
                                        "Ensure that every element in the list is a bool value.",
                                        __FILE__,
                                        __LINE__
                                    );
                                }
                                data.push_back(item.cast<bool>() ? 1 : 0);
                            }
                            return std::make_shared<dip::BooleanNode>(
                                path, std::make_unique<val::ArrayValue<uint8_t>>(data, core::DataType::Boolean)
                            );
                        }

                        // List of int
                        if (py::isinstance<py::int_>(first)) {
                            std::vector<int64_t> data;
                            data.reserve(list.size());
                            for (py::handle item : list) {
                                // bool must not be accepted as int
                                if (!py::isinstance<py::int_>(item) || py::isinstance<py::bool_>(item)) {
                                    throw dip::PybindException(
                                        "Mixed types in ValueNode list",
                                        "All elements of an integer ValueNode list must have type int; bool values are "
                                        "not accepted as integers.",
                                        "Ensure that every element in the list is an int value and does not have type "
                                        "bool.",
                                        __FILE__,
                                        __LINE__
                                    );
                                }
                                data.push_back(item.cast<int64_t>());
                            }
                            return std::make_shared<dip::IntegerNode>(
                                path,
                                std::make_unique<val::ArrayValue<int64_t>>(data, core::DataType::Integer64),
                                std::move(quantity)
                            );
                        }

                        // List of float
                        if (py::isinstance<py::float_>(first)) {
                            std::vector<double> data;
                            data.reserve(list.size());
                            for (py::handle item : list) {
                                if (!py::isinstance<py::float_>(item)) {
                                    throw dip::PybindException(
                                        "Mixed types in ValueNode list",
                                        "All elements of a floating-point ValueNode list must have type float.",
                                        "Ensure that every element in the list is a float value.",
                                        __FILE__,
                                        __LINE__
                                    );
                                }

                                data.push_back(item.cast<double>());
                            }
                            return std::make_shared<dip::FloatNode>(
                                path,
                                std::make_unique<val::ArrayValue<double>>(data, core::DataType::Float64),
                                std::move(quantity)
                            );
                        }

                        // List of string
                        if (py::isinstance<py::str>(first)) {
                            std::vector<std::string> data;
                            data.reserve(list.size());
                            for (py::handle item : list) {
                                if (!py::isinstance<py::str>(item)) {
                                    throw dip::PybindException(
                                        "Mixed types in ValueNode list",
                                        "All elements of a string ValueNode list must have type str.",
                                        "Ensure that every element in the list is a str value.",
                                        __FILE__,
                                        __LINE__
                                    );
                                }
                                data.push_back(item.cast<std::string>());
                            }
                            return std::make_shared<dip::StringNode>(
                                path, std::make_unique<val::ArrayValue<std::string>>(data, core::DataType::String)
                            );
                        }
                        throw dip::PybindException(
                            "Invalid ValueNode list type",
                            "ValueNode list elements must have type bool, int, float, or str.",
                            "Use a list containing only bool, int, float, or str values.",
                            __FILE__,
                            __LINE__
                        );
                    }
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

        val.def("to_string", &dip::ValueNode::to_string, py::arg("format") = core::StringFormatType());

        val.def("to_numpy", [](const dip::ValueNode& vnode) -> py::object { return to_numpy_value(vnode.value); });
    }

} // namespace snt::bind::python
