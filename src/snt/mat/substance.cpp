#include "substance.h"

namespace snt::mat {

  Substance::Substance(const std::string& expr, double prop, bool nat):
    Component<double>(prop),
    Composite<SubstanceSolver, Element>(expr, nat) {
  }
  
  void Substance::math_add(Substance* other) {
  }

  void Substance::math_multiply(Substance* other) {
  }
  
}
