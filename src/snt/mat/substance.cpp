#include "substance.h"

namespace snt::mat {

  Substance Substance::from_string(const std::string& expr) {
    std::cout << expr << '\n';
    char* end = nullptr;
    std::strtod(expr.c_str(), &end); // parse number and report last unparsed character
    if (end != expr.c_str() && *end == '\0') {
      return Substance(std::stod(expr));
    } else {
      Substance::PartMap em;
      em[expr] = 1;
      return Substance(em);
    }
  }

  Substance::Substance(double num):
    Part<double>(num), Set<SubstanceSolver, Element, int>("", true) {
  }
    
  Substance::Substance(const PartMap& comp, double prop, bool nat):
    Part<double>(prop), Set<SubstanceSolver, Element, int>(comp, nat) {
  }
    
  Substance::Substance(const std::string& expr, double prop, bool nat):
    Part<double>(prop), Set<SubstanceSolver, Element, int>(expr, nat) {
  }
  
  void Substance::math_add(Substance* other) {
  }

  void Substance::math_multiply(Substance* other) {
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
