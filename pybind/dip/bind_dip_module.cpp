#include <pybind11/numpy.h>
#include <pybind11/pybind11.h>
#include <snt/val.h>
#include <snt/dip.h>

namespace py = pybind11;
using namespace snt;

void init_dip(py::module_& m) {

  auto rt = py::enum_<dip::RequestType>(m, "RequestType");
  rt.value("Function", dip::RequestType::Function);
  rt.value("Reference", dip::RequestType::Reference);
  rt.export_values();

  auto val = py::class_<dip::ValueNode, std::shared_ptr<dip::ValueNode>>(m, "ValueNode");
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
    case dip::NodeDtype::Boolean: {  // TODO: in the future bool should be stored as uint8_t
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
      val::ArrayValue<std::string>* val =
        dynamic_cast<val::ArrayValue<std::string>*>(vnode.value.get());
      if (!val) throw std::runtime_error("Type mismatch");      
      py::array arr(py::dtype("O"), shape);
      py::object* data = static_cast<py::object*>(arr.mutable_data());
      for (size_t i = 0; i < val->get_size(); ++i)
        data[i] = py::str((*val).get_value(i));
      return arr;
    }
    default:
      throw py::type_error("Unknown dtype in Environment");
    }
  });
  
  auto env = py::class_<dip::Environment>(m, "Environment");
  env.def(py::init<>());
  env.def("__getitem__", [](const dip::Environment &e, size_t i) {
    return e.nodes.at(i);
  }, py::arg("node"));
  env.def_property_readonly("size", [](const dip::Environment &e) {
    return e.nodes.size();
  });
  //env.def("request_code", &dip::Environment::request_code, py::arg("source_name"));
  //env.def("request_value", &dip::Environment::request_value, py::arg("request"), py::arg("rtype"), py::arg("to_units") = "");
  //env.def("request_nodes", &dip::Environment::request_nodes, py::arg("request"), py::arg("rtype"));
  //env.def("get_value", &dip::Environment::get_value, py::arg("index"));

  auto dip = py::class_<dip::DIP>(m, "DIP");
  dip.def(py::init<>());
  dip.def("add_string", &dip::DIP::add_string, py::arg("source_code"));
  dip.def("add_file", &dip::DIP::add_file, py::arg("source_file"), py::arg("source_name") = "", py::arg("absolute") = true);
  dip.def("add_source", &dip::DIP::add_source, py::arg("source_name"), py::arg("source_file"));
  dip.def("add_unit", &dip::DIP::add_unit, py::arg("name"), py::arg("value"), py::arg("unit") = "");
  //.def("add_value_function", &dip::DIP::add_value_function, py::arg("name"), py::arg(""))
  //.def("add_node_function", &dip::DIP::add_node_function, py::arg("name"), py::arg(""))
  dip.def("parse", &dip::DIP::parse);
  // dip.def("parse_docs", &dip::DIP::parse_docs);
  dip.def("to_string", &dip::DIP::to_string);

  
}
