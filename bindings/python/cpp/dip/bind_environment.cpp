#include <codecvt>
#include <locale>
#include <pybind11/numpy.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <snt/dip/environment.h>
#include <snt/dip/lists/list_node.h>
#include <snt/dip/nodes/node_value.h>

namespace py = pybind11;
using namespace snt;

void init_environment(py::module_& m) {

    auto nl = py::class_<dip::NodeList<dip::ValueNode>>(m, "NodeList");
    nl.def(py::init<>());
    nl.def(
        "__getitem__", [](const dip::NodeList<dip::ValueNode>& self, size_t i) { return self.at(i); }, py::arg("node")
    );

    auto env = py::class_<dip::Environment>(m, "Environment");
    env.def(py::init<>());
    env.def_property_readonly("nodes", [](const dip::Environment& e) { return &e.nodes; });
    env.def_property_readonly("size", [](const dip::Environment& e) { return e.nodes.size(); });

    env.def(
        "request_nodes",
        [](const dip::Environment& e, const std::string& path, const std::vector<std::string>& tags) {
            return e.request_nodes(path, dip::RequestType::Reference, tags);
        },
        py::arg("path"),
        py::arg("tags") = std::vector<std::string>{}
    );

    env.def(
        "request_value",
        [](const dip::Environment& e, const std::string& path, const std::string& to_unit) {
            val::BaseValue::PointerType value = e.request_value(path, dip::RequestType::Reference, to_unit);
        },
        py::arg("path"),
        py::arg("to_units") = ""
    );

    // env.def("request_code", &dip::Environment::request_code, py::arg("source_name"));
}
