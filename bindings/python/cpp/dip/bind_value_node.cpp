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
    val.def_readonly("name", &dip::ValueNode::name);
    val.def_property_readonly(
        "units",
        [](const dip::ValueNode& vnode) -> const puq::Quantity* { return vnode.units ? &(*vnode.units) : nullptr; },
        py::return_value_policy::reference_internal
    );
    val.def_property_readonly("value", [](const dip::ValueNode& vnode) -> py::object {
        std::vector<size_t> shape = vnode.value->get_shape();
        std::vector<ssize_t> strides(shape.size());

        // Create a capsule to ensure lifetime safety
        auto capsule = py::capsule(vnode.value.get(), [](void*) {
            // Do nothing — environment owns the memory
        });

        // Compute strides correctly based on element type size
        auto compute_strides = [&](ssize_t elem_size) {
            std::vector<ssize_t> s(shape.size());
            ssize_t stride = elem_size;
            for (ssize_t j = static_cast<ssize_t>(shape.size()) - 1; j >= 0; --j) {
                s[j] = stride;
                stride *= static_cast<ssize_t>(shape[j]);
            }
            return s;
        };

        switch (vnode.dtype) {
        case dip::NodeDtype::Boolean: {
            // TODO: in the future bool should be stored as uint8_t because bool is not 8 bit,
            // but 1 bit
            val::ArrayValue<uint8_t>* val = dynamic_cast<val::ArrayValue<uint8_t>*>(vnode.value.get());
            if (!val)
                throw std::runtime_error("Type mismatch");
            auto strides = compute_strides(sizeof(uint8_t));
            return py::array_t<uint8_t>(shape, strides, val->get_data(), capsule);
        }
        case dip::NodeDtype::Integer: {
            val::ArrayValue<int64_t>* val = dynamic_cast<val::ArrayValue<int64_t>*>(vnode.value.get());
            if (!val)
                throw std::runtime_error("Type mismatch");
            auto strides = compute_strides(sizeof(int64_t));
            py::array arr(py::dtype::of<int32_t>(), shape, strides, val->get_data(), capsule);
            arr.attr("flags").attr("writeable") = false;
            return arr;
        }
        case dip::NodeDtype::Float: {
            val::ArrayValue<double>* val = dynamic_cast<val::ArrayValue<double>*>(vnode.value.get());
            if (!val)
                throw std::runtime_error("Type mismatch");
            auto strides = compute_strides(sizeof(double));
            return py::array_t<double>(shape, strides, val->get_data(), capsule);
        }
        case dip::NodeDtype::String: {
            val::ArrayValue<std::string>* val = dynamic_cast<val::ArrayValue<std::string>*>(vnode.value.get());
            py::list list;
            // NOTE: For string-array conversion I could not find an alternative
            //       implementation that would work both on clang/macos and gcc/linux
            //       so for now we stick with this one.
            for (size_t i = 0; i < val->get_size(); i++)
                list.append(py::str(val->get_value(i)));
            py::tuple t(shape.size());
            for (size_t i = 0; i < shape.size(); ++i)
                t[i] = py::cast(shape[i]);
            py::module np = py::module::import("numpy");
            py::object arr = np.attr("array")(list);
            py::object reshaped = arr.attr("reshape")(t);
            return reshaped.cast<py::array>();
        }
        default:
            throw py::type_error("Unknown dtype in Environment");
        }
    });
}
