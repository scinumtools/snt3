#include "substance.h"

namespace snt::mat {

  Substance::Substance(double num):
    Part<double>(num), Set<Element, int>({}, true) {
  }
    
  Substance::Substance(const ElementMap& comp, double prop, bool nat):
    Part<double>(prop), Set<Element, int>(comp, nat) {
  }
    
  Substance::Substance(const std::string& expr, double prop, bool nat):
    Part<double>(prop), Set<Element, int>(SubstanceSolver().solve(expr), nat) {
  }
  
  std::string Substance::to_string() const {
    std::stringstream ss;
    ss << "Substance(";
    ss << "mass=" << matterProps.mass << " ";
    ss << "Z="    << ' ' << " ";
    ss << "N="    << ' ' << " ";
    ss << "e="    << ' ' << " ";
    ss << ")";
    return ss.str();
  };
  
}
