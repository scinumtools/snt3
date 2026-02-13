#include "substance_atom.h"

#include <sstream>

namespace snt::mat {
  
  /**
   * @brief Test if a string is an integer
   *
   * @param str String that should be tested
   * @return Returns true if input string is an integer
   */
  inline bool is_integer(const std::string& str) {
    try {
      size_t pos;
      std::stoi(str, &pos); // Convert string to int
      return pos == str.size(); // Ensure entire string was used
    } catch (std::invalid_argument&) {
      return false; // Not a number
    } catch (std::out_of_range&) {
      return false; // Number too big for int
    }
  }
  
  ElementMap SubstanceAtom::from_string(std::string s, exs::BaseSettings* set) {
    if (is_integer(s)) {
      return {{"", std::stoi(s)}};
    } else {
      return {{s, 1}};
    }
  }

  std::string SubstanceAtom::to_string() {
    std::stringstream ss;
    for (const auto& [name, prop]: value) {
      ss << name << prop;
    }
    return ss.str();
  }

  void SubstanceAtom::math_add(SubstanceAtom* other) {
    for (auto& [key,val]: other->value) {
      value[key] += val;
    }
  }

  void SubstanceAtom::math_multiply(SubstanceAtom* other) {
    if (other->value.size() == 1 && other->value.begin()->first.empty()) {
      const auto& exponent = other->value.begin()->second;
      for (auto& [key,val]: value) {
	val *= exponent;
      }
    } else {
      throw std::runtime_error("Multiplication exponent is not a scalar value");
    }
  }

}
