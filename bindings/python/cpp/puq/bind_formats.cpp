#include <pybind11/pybind11.h>
#include <snt/puq.h>

namespace py = pybind11;
using namespace snt;

void init_puq_formats(py::module_& m) {

  auto d = py::enum_<puq::Format::Base>(m, "Dformat");
  d.value("MKS", puq::Format::Base::MKS);
  d.value("MGS", puq::Format::Base::MGS);
  d.value("CGS", puq::Format::Base::CGS);
  d.value("FPS", puq::Format::Base::FPS);
  d.export_values();
}
