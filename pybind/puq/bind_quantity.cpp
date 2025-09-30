#include <snt/puq.h>

#include <pybind11/pybind11.h>
#include <pybind11/numpy.h>
#include <pybind11/stl.h>
#include <pybind11/operators.h>
#include <variant>

namespace py = pybind11;
using namespace snt;

// Adding Python context manager methods to UnitSystem
class UnitSystem: public puq::UnitSystem {
  using puq::UnitSystem::UnitSystem;
public:
  UnitSystem& enter() {return *this;}
  void exit(const py::object& exc_type, const py::object& exc_value, const py::object& traceback) {close();}
};

#if defined(MAGNITUDE_ARRAY)

py::array_t<MAGNITUDE_PRECISION> array_to_numpy(puq::Array array) {
  py::array_t<MAGNITUDE_PRECISION> numpy(array.value.size());
  py::buffer_info buf_info = numpy.request();
  MAGNITUDE_PRECISION* ptr = static_cast<MAGNITUDE_PRECISION*>(buf_info.ptr);
  std::memcpy(ptr, array.value.data(), array.value.size() * sizeof(MAGNITUDE_PRECISION));
  numpy.resize(array.shape());
  return numpy;
}

#elif defined(MAGNITUDE_VALUE)

py::array_t<MAGNITUDE_PRECISION> array_to_numpy(const val::ArrayValue<MAGNITUDE_PRECISION>& varray) {
  py::array_t<MAGNITUDE_PRECISION> numpy(varray.get_shape());
  py::buffer_info buf_info = numpy.request();
  MAGNITUDE_PRECISION* ptr = static_cast<MAGNITUDE_PRECISION*>(buf_info.ptr);
  std::memcpy(ptr, varray.get_values().data(), varray.get_size() * sizeof(MAGNITUDE_PRECISION));
  return numpy;
}

#endif

std::variant<MAGNITUDE_PRECISION, std::vector<MAGNITUDE_PRECISION>, py::array_t<MAGNITUDE_PRECISION>> quantity_value(puq::Quantity q, bool numpy) {
#if defined(MAGNITUDE_ARRAY)
  if (numpy) {
    return array_to_numpy(q.value.magnitude.value);
  } else if (q.value.magnitude.value.size()==1) {
    return q.value.magnitude.value[0];
  } else {
    return q.value.magnitude.value.value;
  }
#elif defined(MAGNITUDE_VALUE)
  val::ArrayValue<MAGNITUDE_PRECISION> varray(q.value.magnitude.value.get());
  if (numpy) {
    return array_to_numpy(varray);
  } else if (q.value.magnitude.value->get_size()==1) {
    return varray.get_value(0);
  } else {
    return varray.get_values();
  }
#endif
}

std::variant<MAGNITUDE_PRECISION, std::vector<MAGNITUDE_PRECISION>, py::array_t<MAGNITUDE_PRECISION>> quantity_get_value(puq::Quantity q, size_t index) {
  val::ArrayValue<MAGNITUDE_PRECISION>* otherT = dynamic_cast<val::ArrayValue<MAGNITUDE_PRECISION>*>(q.value.magnitude.value.get());
  return otherT->get_value(index);
}

std::variant<MAGNITUDE_PRECISION, std::vector<MAGNITUDE_PRECISION>, py::array_t<MAGNITUDE_PRECISION>> quantity_error(puq::Quantity q, bool numpy) {
#if defined(MAGNITUDE_ARRAY)
  if (numpy) {
    return array_to_numpy(q.value.magnitude.error);
  } else if (q.value.magnitude.value.size()==1) {
    return q.value.magnitude.error[0];
  } else {
    return q.value.magnitude.error.value;
  }
#elif defined(MAGNITUDE_VALUE)
  val::ArrayValue<MAGNITUDE_PRECISION> varray(q.value.magnitude.error.get());
  if (numpy) {
    return array_to_numpy(varray);
  } else if (varray.get_size()==1) {
    return varray.get_value(0);
  } else {
    return varray.get_values();
  }
#endif
}

#if defined(MAGNITUDE_ARRAY)

puq::Array buffer_to_array(py::buffer_info& info) {
  puq::ArrayValue a(info.size);
  
  if (info.format != py::format_descriptor<MAGNITUDE_PRECISION>::format())
    throw std::runtime_error("Incompatible format: expected a double array!");
  
  std::copy(static_cast<MAGNITUDE_PRECISION*>(info.ptr),
	    static_cast<MAGNITUDE_PRECISION*>(info.ptr)+info.size,
	    a.begin());

  puq::ArrayShape s(info.shape.size());
  std::copy(info.shape.begin(), info.shape.end(), s.begin());
  return puq::Array(a, s);
}

#elif defined(MAGNITUDE_VALUE)

val::BaseValue::PointerType buffer_to_array(py::buffer_info& info) {
  std::vector<MAGNITUDE_PRECISION> a(info.size);
  
  if (info.format != py::format_descriptor<MAGNITUDE_PRECISION>::format())
    throw std::runtime_error("Incompatible format: expected a double array!");
  
  std::copy(static_cast<MAGNITUDE_PRECISION*>(info.ptr),
	    static_cast<MAGNITUDE_PRECISION*>(info.ptr)+info.size,
	    a.begin());

  val::Array::ShapeType s(info.shape.size());
  std::copy(info.shape.begin(), info.shape.end(), s.begin());
  return std::make_unique<val::ArrayValue<MAGNITUDE_PRECISION>>(a, s);
}

#endif

val::BaseValue::PointerType parseBaseValue(const py::object& other) {
  std::vector<MAGNITUDE_PRECISION> vec;
  if (py::isinstance<py::int_>(other)) {
    vec = {static_cast<MAGNITUDE_PRECISION>(other.cast<int>())};
  } else if (py::isinstance<py::list>(other)) {
    vec = other.cast<std::vector<MAGNITUDE_PRECISION>>();
  } else if (py::isinstance<py::array>(other)) {
    py::array_t<MAGNITUDE_PRECISION, py::array::c_style | py::array::forcecast> arr(other);
    auto buf = arr.unchecked<1>();
    vec.resize(buf.shape(0));
    for (ssize_t i = 0; i < buf.shape(0); ++i)
      vec[i] = buf(i);
  } else {
    return nullptr;
  }
  return std::make_unique<val::ArrayValue<MAGNITUDE_PRECISION>>(vec);
}

PYBIND11_MODULE(scinumtools3, m) {

  m.doc() = "Scientific Numerical Tools v3"; 

  m.attr("__version__") = CODE_VERSION;

  auto puq = m.def_submodule("puq", "Physical Units and Quantities");
  
  // Exposing all unit systems
  auto sys = puq.def_submodule("systems", "Unit systems");
  auto e = py::enum_<puq::SystemType>(sys, "SystemType");
  for (auto sys: puq::SystemMap) {
    e.value(sys.second->SystemAbbrev.c_str(), sys.first);
  }
  e.export_values();

  // Exposing dimension formats
  auto df = puq.def_submodule("formats", "Dimension formats");
  auto d = py::enum_<puq::Format::Base>(df, "Dformat");
  d.value("MKS", puq::Format::Base::MKS);
  d.value("MGS", puq::Format::Base::MGS);
  d.value("CGS", puq::Format::Base::CGS);
  d.value("FPS", puq::Format::Base::FPS);
  d.export_values();

  // Exposing unit format
  py::class_<snt::puq::UnitFormat>(puq, "UnitFormat")
    .def(py::init<>())  // exposes default constructor
    ;
  
  // Exposing lists
  auto lists = puq.def_submodule("lists", "Unit system lists");
  lists.def("prefixes",          &puq::lists::prefixes,          py::arg("json") = false, py::arg("format") = snt::puq::UnitFormat());
  lists.def("base_units",        &puq::lists::base_units,        py::arg("json") = false, py::arg("format") = snt::puq::UnitFormat());
  lists.def("derived_units",     &puq::lists::derived_units,     py::arg("json") = false, py::arg("format") = snt::puq::UnitFormat());
  lists.def("logarithmic_units", &puq::lists::logarithmic_units, py::arg("json") = false);
  lists.def("temperature_units", &puq::lists::temperature_units, py::arg("json") = false);
  lists.def("constants",         &puq::lists::constants,         py::arg("json") = false, py::arg("format") = snt::puq::UnitFormat());
  lists.def("quantities",        &puq::lists::quantities,        py::arg("json") = false, py::arg("format") = snt::puq::UnitFormat());
  lists.def("unit_systems",      &puq::lists::unit_systems,      py::arg("json") = false);

  // Expose UnitSystem
  py::class_<puq::UnitSystem>(puq, "UnitSystemBase")
    .def(py::init<const puq::SystemType>())
    .def("change", &puq::UnitSystem::change)      
    .def("close", &puq::UnitSystem::close)      
    ;

  py::class_<UnitSystem, puq::UnitSystem>(puq, "UnitSystem")
    .def(py::init<const puq::SystemType>())
    .def("enter", &UnitSystem::enter)
    .def("exit", &UnitSystem::exit)  
    .def("__enter__", &UnitSystem::enter)
    .def("__exit__", &UnitSystem::exit)
    ;

  // Exposing calculator object
  puq.def("Calculator", [](std::string e) -> puq::Quantity {
    auto calc = puq::Calculator();
    auto atom = calc.solve(e);
    return atom.value;
  }, py::arg("expression"));
    
  // Exposing quantity object
  auto q = py::class_<puq::Quantity>(puq, "Quantity", py::buffer_protocol());
  
  q.def(py::init<std::string>())
    .def(py::init<std::string, puq::SystemType>())
    .def(py::init<MAGNITUDE_PRECISION>())
    .def(py::init<MAGNITUDE_PRECISION, std::string>())
    .def(py::init<MAGNITUDE_PRECISION, std::string, puq::SystemType>())
    .def(py::init<MAGNITUDE_PRECISION, MAGNITUDE_PRECISION>())
    .def(py::init<MAGNITUDE_PRECISION, MAGNITUDE_PRECISION, std::string>())
    .def(py::init<MAGNITUDE_PRECISION, MAGNITUDE_PRECISION, std::string, puq::SystemType>())
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
  q.def(py::init([](py::array_t<MAGNITUDE_PRECISION, py::array::c_style | py::array::forcecast> v,
		     std::string s, puq::SystemType sys) {
      py::buffer_info info = v.request();
      val::BaseValue::PointerType value = buffer_to_array(info);
#if defined(MAGNITUDE_ARRAY)
      return puq::Quantity(value, s, sys);
#elif defined(MAGNITUDE_VALUE)
      return puq::Quantity(std::move(value), s, sys);
#endif
  }), py::arg("value"), py::arg("unit")="", py::arg("system")=puq::SystemType::NONE);

  /**
   * @brief Initialise Quantity with a value/error from numpy arrays
   */
  q.def(py::init([](py::array_t<MAGNITUDE_PRECISION, py::array::c_style | py::array::forcecast> v,
		     py::array_t<MAGNITUDE_PRECISION, py::array::c_style | py::array::forcecast> e,
		     std::string s, puq::SystemType sys) {
      py::buffer_info v_info = v.request();
      val::BaseValue::PointerType value = buffer_to_array(v_info);
      py::buffer_info e_info = e.request();
      val::BaseValue::PointerType error = buffer_to_array(e_info);
#if defined(MAGNITUDE_ARRAY)
      return puq::Quantity(value, error, s, sys);
#elif defined(MAGNITUDE_VALUE)
      return puq::Quantity(std::move(value), std::move(error), s, sys);
#endif
  }), py::arg("value"), py::arg("error"), py::arg("unit")="", py::arg("system")=puq::SystemType::NONE);

  /**
   * @brief Convert Quantity into a numpy array
   */
  q.def("to_numpy", [](const puq::Quantity& q) {
    val::ArrayValue<MAGNITUDE_PRECISION>* otherT = dynamic_cast<val::ArrayValue<MAGNITUDE_PRECISION>*>(q.value.magnitude.value.get());
    std::vector<size_t> shape = otherT->get_shape();
    std::vector<ssize_t> strides(shape.size());
    ssize_t stride = sizeof(otherT);
    for (ssize_t i = shape.size() - 1; i >= 0; --i) {
        strides[i] = stride;
        stride *= shape[i];
    }
    return py::array_t<MAGNITUDE_PRECISION>(shape, strides, otherT->get_data());
  });
//  q.def("to_numpy", [](const puq::Quantity &q) -> py::buffer_info {
//#if defined(MAGNITUDE_ARRAY)
//      return py::buffer_info(
//			     q.value.magnitude.value.value.data(),
//			     sizeof(MAGNITUDE_PRECISION),
//			     py::format_descriptor<MAGNITUDE_PRECISION>::format(),
//			     1,
//			     q.value.magnitude.value.shape(),
//			     {sizeof(MAGNITUDE_PRECISION)}
//			     );
//#elif defined(MAGNITUDE_VALUE)
//      val::ArrayValue<MAGNITUDE_PRECISION>* otherT = dynamic_cast<val::ArrayValue<MAGNITUDE_PRECISION>*>(q.value.magnitude.value.get());
//      return py::buffer_info(
//			     otherT->get_data(), 
//			     sizeof(MAGNITUDE_PRECISION),
//			     py::format_descriptor<MAGNITUDE_PRECISION>::format(),
//			     1,
//			     otherT->get_shape(),
//			     {sizeof(MAGNITUDE_PRECISION)},
//			     false
//			     );      
//#endif
//   });
   
   q.def("convert", py::overload_cast<std::string, puq::SystemType, const std::string&>(&puq::Quantity::convert, py::const_),
	 py::arg("expression"), py::arg("system")=puq::SystemType::NONE, py::arg("quantity")="")
     .def("convert", py::overload_cast<const puq::Format::Base&, puq::SystemType>(&puq::Quantity::convert, py::const_),
	  py::arg("dformat"), py::arg("system")=puq::SystemType::NONE) 
     .def("unit_system", &puq::Quantity::unit_system)
     .def("rebase_prefixes", &puq::Quantity::rebase_prefixes)
     .def("rebase_dimensions", &puq::Quantity::rebase_dimensions)
     .def("to_string", &puq::Quantity::to_string, py::arg("format") = snt::puq::UnitFormat())
     .def("size", &puq::Quantity::size)
     .def("shape", &puq::Quantity::shape);
   
   q.def("value", &quantity_value, py::arg("numpy")=false);
   q.def("error", &quantity_error, py::arg("numpy")=false);
   
   q.def("__repr__", &puq::Quantity::to_string, py::arg("format") = snt::puq::UnitFormat())
     .def("__str__", &puq::Quantity::to_string, py::arg("format") = snt::puq::UnitFormat())
     //.def("__getitem__", &quantity_get_value, py::arg("index"))  
     //.def("__len__", &puq::Quantity::size)
     .def(py::self + py::self)
     .def(py::self - py::self)
     .def(py::self * py::self)
     .def(py::self / py::self)
     //.def(MAGNITUDE_PRECISION() + py::self)
     //.def(MAGNITUDE_PRECISION() - py::self)
     //.def(MAGNITUDE_PRECISION() * py::self)
     //.def(MAGNITUDE_PRECISION() / py::self)
     //.def(py::self + MAGNITUDE_PRECISION())
     //.def(py::self - MAGNITUDE_PRECISION())
     //.def(py::self * MAGNITUDE_PRECISION())
     //.def(py::self / MAGNITUDE_PRECISION())
     ;
   
   /**
    * Addition of quantities
    */
   q.def("__add__", [](const puq::Quantity& quant, py::object other) -> py::object {
       val::BaseValue::PointerType varray = parseBaseValue(other);
       if (varray==nullptr)
	 py::reinterpret_borrow<py::object>(Py_NotImplemented);
       return py::cast(quant + std::move(varray));
     }, py::is_operator());
   q.def("__radd__", [](const puq::Quantity& quant, py::object other) -> py::object {
     val::BaseValue::PointerType varray = parseBaseValue(other);
     if (varray==nullptr)
       py::reinterpret_borrow<py::object>(Py_NotImplemented);
     return py::cast(std::move(varray) + quant);
   }, py::is_operator());

   /**
    * Subtraction of quantities
    */
   q.def("__sub__", [](const puq::Quantity& quant, py::object other) -> py::object {
       val::BaseValue::PointerType varray = parseBaseValue(other);
       if (varray==nullptr)
	 py::reinterpret_borrow<py::object>(Py_NotImplemented);
       return py::cast(quant - std::move(varray));
     }, py::is_operator());
   q.def("__rsub__", [](const puq::Quantity& quant, py::object other) -> py::object {
     val::BaseValue::PointerType varray = parseBaseValue(other);
     if (varray==nullptr)
       py::reinterpret_borrow<py::object>(Py_NotImplemented);
     return py::cast(std::move(varray) - quant);
   }, py::is_operator());

   /**
    * Multiplication of quantities
    */
   q.def("__mul__", [](const puq::Quantity& quant, py::object other) -> py::object {
       val::BaseValue::PointerType varray = parseBaseValue(other);
       if (varray==nullptr)
	 py::reinterpret_borrow<py::object>(Py_NotImplemented);
       return py::cast(quant * std::move(varray));
     }, py::is_operator());
   q.def("__rmul__", [](const puq::Quantity& quant, py::object other) -> py::object {
     val::BaseValue::PointerType varray = parseBaseValue(other);
     if (varray==nullptr)
       py::reinterpret_borrow<py::object>(Py_NotImplemented);
     return py::cast(std::move(varray) * quant);
   }, py::is_operator());

   /**
    * Division of quantities
    */
   q.def("__truediv__", [](const puq::Quantity& quant, py::object other) -> py::object {
       val::BaseValue::PointerType varray = parseBaseValue(other);
       if (varray==nullptr)
	 py::reinterpret_borrow<py::object>(Py_NotImplemented);
       return py::cast(quant / std::move(varray));
     }, py::is_operator());
   q.def("__rtruediv__", [](const puq::Quantity& quant, py::object other) -> py::object {
     val::BaseValue::PointerType varray = parseBaseValue(other);
     if (varray==nullptr)
       py::reinterpret_borrow<py::object>(Py_NotImplemented);
     return py::cast(std::move(varray) / quant);
   }, py::is_operator());

   
    //q.def(std::vector<double>() + py::self)
    //.def(std::vector<double>() - py::self)
    //.def(std::vector<double>() * py::self)
    //.def(std::vector<double>() / py::self)
    //.def(py::self + std::vector<double>())
    //.def(py::self - std::vector<double>())
    //.def(py::self * std::vector<double>())
    //.def(py::self / std::vector<double>())

  q.def("__array__", [](const snt::puq::Quantity& q, py::object dtype) {
    throw std::runtime_error("Convert explicitly: use .to_numpy() or .magnitude");
  });

}
