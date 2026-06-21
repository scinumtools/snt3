#include "../bindings/python/cpp/val/bind_value_base.h"

#include <codecvt>
#include <locale>
#include <pybind11/numpy.h>
#include <pybind11/pybind11.h>
#include <snt/core/string_format.h>
#include <snt/dip/nodes/node_value.h>

namespace py = pybind11;
using namespace snt;

void init_value_node(py::module_& m) {

    auto val = py::class_<dip::ValueNode, std::shared_ptr<dip::ValueNode>>(m, "ValueNode");

    val.def("__str__", &dip::ValueNode::to_string, py::arg("format") = core::StringFormatType());

    val.def_property_readonly("name", [](const dip::ValueNode& self) -> const std::string& { return self.path.name; });

    val.def_property_readonly(
        "units",
        [](const dip::ValueNode& vnode) -> const puq::Quantity* { return vnode.units ? &(*vnode.units) : nullptr; },
        py::return_value_policy::reference_internal
    );

    val.def_property_readonly("value", [](const dip::ValueNode& vnode) -> py::object {
        return to_python_value(vnode.value);
    });
}
