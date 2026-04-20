#ifndef PUQ_UNIT_ATOM_H
#define PUQ_UNIT_ATOM_H

#include <snt/exs/atom_base.h>
#include <snt/exs/settings.h>
#include <snt/puq/measurement.h>

namespace snt::puq {

  class AtomParsingExcept : public std::exception {
  private:
    std::string message;

  public:
    AtomParsingExcept(const std::string& m) : message(m) {}
    const char* what() const noexcept override {
      return message.c_str();
    }
  };

  class UnitAtom : public exs::AtomBase<UnitAtom, Measurement> {
  public:
    UnitAtom(UnitAtom& a) : AtomBase(a) {};
    UnitAtom(Measurement v) : AtomBase(v) {};
    static Measurement from_string(const std::string& s, exs::BaseSettings* set = nullptr);
    std::string to_string();
    void math_power(ExponentVariant& e);
    void math_power(UnitAtom* other);
    void math_multiply(UnitAtom* other);
    void math_divide(UnitAtom* other);
  };

} // namespace snt::puq

#endif // PUQ_UNIT_ATOM_H
