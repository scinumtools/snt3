#include <pybind11/numpy.h>
#include <pybind11/pybind11.h>
#include <snt/val.h>
#include <snt/puq.h>
#include <snt/dip.h>
#include <snt/string_format.h>
#include <codecvt>
#include <locale>

namespace py = pybind11;
using namespace snt;

void init_value_node(py::module_& m) {
  
  auto val = py::class_<dip::ValueNode, std::shared_ptr<dip::ValueNode>>(m, "ValueNode");
  val.def("__str__", &dip::ValueNode::to_string, py::arg("format") = snt::StringFormatType());
  val.def_readonly("name", &dip::ValueNode::name);
  val.def_property_readonly("units", [](const dip::ValueNode &vnode) -> puq::Quantity* {
    return vnode.units ? vnode.units.get() : nullptr;
  }, py::return_value_policy::reference_internal);
  val.def_property_readonly("value", [](const dip::ValueNode &vnode) -> py::object {

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
      // TODO: in the future bool should be stored as uint8_t because bool is not 8 bit, but 1 bit
      val::ArrayValue<bool>* val =
        dynamic_cast<val::ArrayValue<bool>*>(vnode.value.get());
      if (!val) throw std::runtime_error("Type mismatch");    
      std::vector<uint8_t> tmp(val->get_size());
      for (size_t i = 0; i < tmp.size(); ++i)
        tmp[i] = (*val).get_value(i) ? 1 : 0;
      // Copy into a NumPy-owned buffer
      py::array_t<uint8_t> arr(shape);
      std::memcpy(arr.mutable_data(), tmp.data(), tmp.size());
      return arr;
    }
    case dip::NodeDtype::Integer: {
      val::ArrayValue<int>* val = dynamic_cast<val::ArrayValue<int>*>(vnode.value.get());
      if (!val) throw std::runtime_error("Type mismatch");      
      auto strides = compute_strides(sizeof(int));
      return py::array_t<int>(shape, strides, val->get_data(), capsule);
    }
    case dip::NodeDtype::Float: {
      val::ArrayValue<double>* val = dynamic_cast<val::ArrayValue<double>*>(vnode.value.get());
      if (!val) throw std::runtime_error("Type mismatch");      
      auto strides = compute_strides(sizeof(double));
      return py::array_t<double>(shape, strides, val->get_data(), capsule);
    }
    case dip::NodeDtype::String: {
      val::ArrayValue<std::string>* val = dynamic_cast<val::ArrayValue<std::string>*>(vnode.value.get());
      py::list list;
      // NOTE: For string-array conversion I could not find an alternative
      //       implementation that would work both on clang/macos and gcc/linux
      //       so for now we stick with this one.
      for (size_t i=0; i<val->get_size(); i++)
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
