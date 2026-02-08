#include <pybind11/numpy.h>
#include <pybind11/operators.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <snt/puq.h>
#include <variant>

namespace py = pybind11;
using namespace snt;

py::array_t<puq::MagnitudeFloat> array_to_numpy(const val::ArrayValue<puq::MagnitudeFloat>& varray) {
  py::array_t<puq::MagnitudeFloat> numpy(varray.get_shape());
  py::buffer_info buf_info = numpy.request();
  puq::MagnitudeFloat* ptr = static_cast<puq::MagnitudeFloat*>(buf_info.ptr);
  std::memcpy(ptr, varray.get_values().data(), varray.get_size() * sizeof(puq::MagnitudeFloat));
  return numpy;
}

std::variant<puq::MagnitudeFloat, std::vector<puq::MagnitudeFloat>, py::array_t<puq::MagnitudeFloat>> quantity_value(puq::Quantity q, bool numpy) {
  val::ArrayValue<puq::MagnitudeFloat> varray(q.value.magnitude.value.get());
  if (numpy) {
    return array_to_numpy(varray);
  } else if (q.value.magnitude.value->get_size() == 1) {
    return varray.get_value(0);
  } else {
    return varray.get_values();
  }
}

std::variant<puq::MagnitudeFloat, std::vector<puq::MagnitudeFloat>, py::array_t<puq::MagnitudeFloat>> quantity_get_value(const puq::Quantity& q, size_t index) {
  val::ArrayValue<puq::MagnitudeFloat>* otherT = dynamic_cast<val::ArrayValue<puq::MagnitudeFloat>*>(q.value.magnitude.value.get());
  return otherT->get_value(index);
}

std::variant<puq::MagnitudeFloat, std::vector<puq::MagnitudeFloat>, py::array_t<puq::MagnitudeFloat>> quantity_error(const puq::Quantity& q, bool numpy) {
  val::ArrayValue<puq::MagnitudeFloat> varray(q.value.magnitude.error.get());
  if (numpy) {
    return array_to_numpy(varray);
  } else if (varray.get_size() == 1) {
    return varray.get_value(0);
  } else {
    return varray.get_values();
  }
}

val::BaseValue::PointerType parseBaseValue(const py::object& other) {
  std::vector<puq::MagnitudeFloat> vec;
  if (py::isinstance<py::int_>(other)) {
    vec = {static_cast<puq::MagnitudeFloat>(other.cast<int>())};
  } else if (py::isinstance<py::list>(other)) {
    vec = other.cast<std::vector<puq::MagnitudeFloat>>();
  } else if (py::isinstance<py::array>(other)) {
    py::array_t<puq::MagnitudeFloat, py::array::c_style | py::array::forcecast> arr(other);
    auto buf = arr.unchecked<1>();
    vec.resize(buf.shape(0));
    for (ssize_t i = 0; i < buf.shape(0); ++i)
      vec[i] = buf(i);
  } else {
    return nullptr;
  }
  return std::make_unique<val::ArrayValue<puq::MagnitudeFloat>>(vec);
}

val::BaseValue::PointerType buffer_to_array(py::buffer_info& info) {
  std::vector<puq::MagnitudeFloat> a(info.size);

  if (info.format != py::format_descriptor<puq::MagnitudeFloat>::format())
    throw std::runtime_error("Incompatible format: expected a double array!");

  std::copy(static_cast<puq::MagnitudeFloat*>(info.ptr),
            static_cast<puq::MagnitudeFloat*>(info.ptr) + info.size,
            a.begin());

  val::Array::ShapeType s(info.shape.size());
  std::copy(info.shape.begin(), info.shape.end(), s.begin());
  return std::make_unique<val::ArrayValue<puq::MagnitudeFloat>>(a, s);
}

void init_puq_quantity(py::module_& m) {

  auto q = py::class_<puq::Quantity>(m, "Quantity", py::buffer_protocol());

  q.def(py::init<std::string>())
      .def(py::init<std::string, puq::SystemType>())
      .def(py::init<puq::MagnitudeFloat>())
      .def(py::init<puq::MagnitudeFloat, std::string>())
      .def(py::init<puq::MagnitudeFloat, std::string, puq::SystemType>())
      .def(py::init<puq::MagnitudeFloat, puq::MagnitudeFloat>())
      .def(py::init<puq::MagnitudeFloat, puq::MagnitudeFloat, std::string>())
      .def(py::init<puq::MagnitudeFloat, puq::MagnitudeFloat, std::string, puq::SystemType>())
      //.def(py::init<std::vector<double>>())
      //.def(py::init<std::vector<double>, std::string>())
      //.def(py::init<std::vector<double>, std::string, puq::SystemType>())
      //.def(py::init<std::vector<double>, std::vector<double>>())
      //.def(py::init<std::vector<double>, std::vector<double>, std::string>())
      //.def(py::init<std::vector<double>, std::vector<double>, std::string, puq::SystemType>())
      ;

  // Set higher priority than NumpyArrays
  q.attr("__array_priority__") = 1000;

  /**
   * @brief Initialise Quantity with a value from numpy arrays
   */
  q.def(py::init([](const py::array_t<puq::MagnitudeFloat, py::array::c_style | py::array::forcecast>& v,
                    const std::string& s, puq::SystemType sys) {
          py::buffer_info info = v.request();
          val::BaseValue::PointerType value = buffer_to_array(info);
#if defined(MAGNITUDE_VALUE)
          return puq::Quantity(std::move(value), s, sys);
#endif
        }),
        py::arg("value"), py::arg("unit") = "", py::arg("system") = puq::SystemType::NONE);

  /**
   * @brief Initialise Quantity with a value/error from numpy arrays
   */
  q.def(py::init([](const py::array_t<puq::MagnitudeFloat, py::array::c_style | py::array::forcecast>& v,
                    const py::array_t<puq::MagnitudeFloat, py::array::c_style | py::array::forcecast>& e,
                    const std::string& s, puq::SystemType sys) {
          py::buffer_info v_info = v.request();
          val::BaseValue::PointerType value = buffer_to_array(v_info);
          py::buffer_info e_info = e.request();
          val::BaseValue::PointerType error = buffer_to_array(e_info);
#if defined(MAGNITUDE_VALUE)
          return puq::Quantity(std::move(value), std::move(error), s, sys);
#endif
        }),
        py::arg("value"), py::arg("error"), py::arg("unit") = "", py::arg("system") = puq::SystemType::NONE);

  /**
   * @brief Convert Quantity into a numpy array
   */
  q.def("to_numpy", [](const puq::Quantity& q) {
    val::ArrayValue<puq::MagnitudeFloat>* otherT = dynamic_cast<val::ArrayValue<puq::MagnitudeFloat>*>(q.value.magnitude.value.get());
    std::vector<size_t> shape = otherT->get_shape();
    std::vector<ssize_t> strides(shape.size());
    ssize_t stride = sizeof(otherT);
    for (ssize_t i = static_cast<ssize_t>(shape.size()) - 1; i >= 0; --i) {
      strides[i] = stride;
      stride *= static_cast<ssize_t>(shape[i]);
    }
    return py::array_t<puq::MagnitudeFloat>(shape, strides, otherT->get_data());
  });
  //  q.def("to_numpy", [](const puq::Quantity &q) -> py::buffer_info {
  // #if defined(MAGNITUDE_VALUE)
  //      val::ArrayValue<puq::MagnitudeFloat>* otherT = dynamic_cast<val::ArrayValue<puq::MagnitudeFloat>*>(q.value.magnitude.value.get());
  //      return py::buffer_info(
  //			     otherT->get_data(),
  //			     sizeof(puq::MagnitudeFloat),
  //			     py::format_descriptor<puq::MagnitudeFloat>::format(),
  //			     1,
  //			     otherT->get_shape(),
  //			     {sizeof(puq::MagnitudeFloat)},
  //			     false
  //			     );
  // #endif
  //   });

  q.def("convert", py::overload_cast<std::string, puq::SystemType, const std::string&>(&puq::Quantity::convert, py::const_),
        py::arg("expression"), py::arg("system") = puq::SystemType::NONE, py::arg("quantity") = "")
      .def("convert", py::overload_cast<const puq::Format::Base&, puq::SystemType>(&puq::Quantity::convert, py::const_),
           py::arg("dformat"), py::arg("system") = puq::SystemType::NONE)
      .def("unit_system", &puq::Quantity::unit_system)
      .def("rebase_prefixes", &puq::Quantity::rebase_prefixes)
      .def("rebase_dimensions", &puq::Quantity::rebase_dimensions)
      .def("to_string", &puq::Quantity::to_string, py::arg("format") = snt::puq::UnitFormat())
      .def("size", &puq::Quantity::size)
      .def("shape", &puq::Quantity::shape);

  q.def("value", &quantity_value, py::arg("numpy") = false);
  q.def("error", &quantity_error, py::arg("numpy") = false);

  q.def("__repr__", &puq::Quantity::to_string, py::arg("format") = snt::puq::UnitFormat())
      .def("__str__", &puq::Quantity::to_string, py::arg("format") = snt::puq::UnitFormat())
      //.def("__getitem__", &quantity_get_value, py::arg("index"))
      //.def("__len__", &puq::Quantity::size)
      .def(py::self + py::self)
      .def(py::self - py::self)
      .def(py::self * py::self)
      .def(py::self / py::self)
      //.def(puq::MagnitudeFloat() + py::self)
      //.def(puq::MagnitudeFloat() - py::self)
      //.def(puq::MagnitudeFloat() * py::self)
      //.def(puq::MagnitudeFloat() / py::self)
      //.def(py::self + puq::MagnitudeFloat())
      //.def(py::self - puq::MagnitudeFloat())
      //.def(py::self * puq::MagnitudeFloat())
      //.def(py::self / puq::MagnitudeFloat())
      ;

  /**
   * Addition of quantities
   */
  q.def("__add__", [](const puq::Quantity& quant, const py::object& other) -> py::object {
       val::BaseValue::PointerType varray = parseBaseValue(other);
       if (varray==nullptr)
	 py::reinterpret_borrow<py::object>(Py_NotImplemented);
       return py::cast(quant + std::move(varray));
  }, py::is_operator());
  q.def("__radd__", [](const puq::Quantity& quant, const py::object& other) -> py::object {
     val::BaseValue::PointerType varray = parseBaseValue(other);
     if (varray==nullptr)
       py::reinterpret_borrow<py::object>(Py_NotImplemented);
     return py::cast(std::move(varray) + quant);
  }, py::is_operator());

  /**
   * Subtraction of quantities
   */
  q.def("__sub__", [](const puq::Quantity& quant, const py::object& other) -> py::object {
       val::BaseValue::PointerType varray = parseBaseValue(other);
       if (varray==nullptr)
	 py::reinterpret_borrow<py::object>(Py_NotImplemented);
       return py::cast(quant - std::move(varray));
  }, py::is_operator());
  q.def("__rsub__", [](const puq::Quantity& quant, const py::object& other) -> py::object {
     val::BaseValue::PointerType varray = parseBaseValue(other);
     if (varray==nullptr)
       py::reinterpret_borrow<py::object>(Py_NotImplemented);
     return py::cast(std::move(varray) - quant);
  }, py::is_operator());

  /**
   * Multiplication of quantities
   */
  q.def("__mul__", [](const puq::Quantity& quant, const py::object& other) -> py::object {
       val::BaseValue::PointerType varray = parseBaseValue(other);
       if (varray==nullptr)
	 py::reinterpret_borrow<py::object>(Py_NotImplemented);
       return py::cast(quant * std::move(varray));
  }, py::is_operator());
  q.def("__rmul__", [](const puq::Quantity& quant, const py::object& other) -> py::object {
     val::BaseValue::PointerType varray = parseBaseValue(other);
     if (varray==nullptr)
       py::reinterpret_borrow<py::object>(Py_NotImplemented);
     return py::cast(std::move(varray) * quant);
  }, py::is_operator());

  /**
   * Division of quantities
   */
  q.def("__truediv__", [](const puq::Quantity& quant, const py::object& other) -> py::object {
       val::BaseValue::PointerType varray = parseBaseValue(other);
       if (varray==nullptr)
	 py::reinterpret_borrow<py::object>(Py_NotImplemented);
       return py::cast(quant / std::move(varray));
  }, py::is_operator());
  q.def("__rtruediv__", [](const puq::Quantity& quant, const py::object& other) -> py::object {
     val::BaseValue::PointerType varray = parseBaseValue(other);
     if (varray==nullptr)
       py::reinterpret_borrow<py::object>(Py_NotImplemented);
     return py::cast(std::move(varray) / quant);
  }, py::is_operator());

  // q.def(std::vector<double>() + py::self)
  //.def(std::vector<double>() - py::self)
  //.def(std::vector<double>() * py::self)
  //.def(std::vector<double>() / py::self)
  //.def(py::self + std::vector<double>())
  //.def(py::self - std::vector<double>())
  //.def(py::self * std::vector<double>())
  //.def(py::self / std::vector<double>())

  q.def("__eq__", [](const snt::puq::Quantity& self, const snt::puq::Quantity& other) -> py::object {
    return py::bool_(self==other);
  }, py::is_operator());
  
  q.def("__neq__", [](const snt::puq::Quantity& self, const snt::puq::Quantity& other) -> py::object {
    return py::bool_(self!=other);
  }, py::is_operator());
  
  q.def("__array__", [](const snt::puq::Quantity& q, const py::object& dtype) {
    throw std::runtime_error("Convert explicitly: use .to_numpy() or .magnitude");
  });
}
