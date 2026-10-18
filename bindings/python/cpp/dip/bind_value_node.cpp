#include "../../../../src/snt/dip/nodes/metadata_fields.h"
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

        auto metadata = py::class_<dip::ValueMetadata>(
            m, "ValueMetadata", "Citation, licensing, and descriptive metadata attached to a DIPL value or schema."
        );
        metadata.def(py::init<>(), "Create empty DIPL value or schema metadata.");
#define SNT_PY_METADATA(TYPE, FIELD, NAME) metadata.def_readonly(NAME, &dip::ValueMetadata::FIELD);
        SNT_DIP_METADATA_SCALAR_FIELDS(SNT_PY_METADATA)
        SNT_DIP_METADATA_LIST_FIELDS(SNT_PY_METADATA)
#undef SNT_PY_METADATA

        auto vdata = py::class_<dip::ValueNodeData>(
            m, "ValueNodeData", "Evaluated value data returned by DIPL functions and nodes."
        );
        vdata.def(
            py::init([](const py::object& value, const py::object& units) {
                dip::ValueNodeData data;
                data.value = from_python(value);
                if (!units.is_none()) {
                    data.units = puq::Quantity(units.cast<std::string>());
                }
                return data;
            }),
            py::arg("value"),
            py::arg("units") = py::none(),
            "Create value data from a Python value and optional units."
        );

        auto val = py::class_<dip::ValueNode, std::shared_ptr<dip::ValueNode>>(
            m, "ValueNode", "A DIPL parameter node with value, type, units, and metadata."
        );

        val.def(
            py::init(
                [](const std::string& path,
                   const py::object& value,
                   const py::object& units) -> std::shared_ptr<dip::ValueNode> {
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
            py::arg("units") = py::none(),
            "Create a DIPL value node.\n\nArgs:\n    path: Fully qualified node path.\n"
            "    value: Python bool, int, float, str, or list value.\n"
            "    units: Optional unit expression for a numeric value."
        );

        val.def(
            "__str__",
            &dip::ValueNode::to_string,
            py::arg("format") = core::StringFormatType(),
            "Format this node as text."
        );

        val.def_property_readonly(
            "name",
            [](const dip::ValueNode& self) -> const std::string& { return self.path.name; },
            "Name of the parameter node."
        );

        val.def_property_readonly(
            "units",
            [](const dip::ValueNode& vnode) -> const puq::Quantity* { return vnode.units ? &(*vnode.units) : nullptr; },
            py::return_value_policy::reference_internal,
            "Quantity units attached to the node, or None."
        );

        val.def_property_readonly(
            "value",
            [](const dip::ValueNode& vnode) -> py::object { return to_python_value(vnode.value); },
            "Python value stored by the node."
        );

        val.def_property_readonly(
            "node_type",
            [](const dip::ValueNode& vnode) { return dip::NodeDtypeNames.at(vnode.dtype); },
            "DIPL node type, such as `integer`, `string`, or `link`."
        );
        val.def_property_readonly(
            "link_source",
            [](const dip::ValueNode& vnode) -> py::object {
                return vnode.dtype == dip::NodeDtype::Link ? py::cast(vnode.link_source) : py::none();
            },
            "Named source for a link, or None for non-links and local links."
        );
        val.def_property_readonly(
            "link_target",
            [](const dip::ValueNode& vnode) -> py::object {
                return vnode.dtype == dip::NodeDtype::Link ? py::cast(vnode.link_target) : py::none();
            },
            "Target path for a link, or None for non-links."
        );

        val.def_property_readonly(
            "tags",
            [](const dip::ValueNode& vnode) { return vnode.tags; },
            "Copy of the node's explicitly assigned tags. Editing this Python list does not change the environment."
        );

        val.def_property_readonly(
            "schema_id",
            [](const dip::ValueNode& vnode) { return vnode.schema_id; },
            "Trace ID of the schema that supplied this node, or an empty string."
        );
        val.def_property_readonly(
            "override",
            [](const dip::ValueNode& vnode) { return vnode.override; },
            "Whether an explicit $override supplied the effective value."
        );

        val.def_property_readonly(
            "metadata",
            [](const dip::ValueNode& vnode) -> const dip::ValueMetadata& { return vnode.metadata; },
            py::return_value_policy::reference_internal,
            "Documentation and provenance metadata attached to this value node."
        );

        val.def_property_readonly(
            "shape", [](const dip::ValueNode& vnode) {
                return vnode.value ? vnode.value->get_shape() : val::Array::ShapeType{};
            }, "Shape of the stored value; an empty shape is returned for nodes without values."
        );

        val.def_property_readonly(
            "dtype",
            [](const dip::ValueNode& vnode) {
                return vnode.value ? vnode.value->get_dtype() : core::DataType::None;
            },
            "SNT data type of the stored value."
        );

        val.def(
            "to_string",
            &dip::ValueNode::to_string,
            py::arg("format") = core::StringFormatType(),
            "Format this node as text."
        );

        val.def(
            "to_numpy",
            [](const dip::ValueNode& vnode) -> py::object {
                return vnode.value ? to_numpy_value(vnode.value) : py::none();
            },
            "Return the stored value as a NumPy array."
        );
    }

} // namespace snt::bind::python
