#ifndef PUQ_DIMENSIONS_H
#define PUQ_DIMENSIONS_H

#include "../magnitude.h"
#include "../settings.h"
#include "../systems/systems.h"

namespace snt::puq {

  class Dimensions {
  public:
    MAGNITUDE_TYPE numerical;
    BaseDimensions physical;
    std::vector<std::string> symbols;
    Utype utype;
    Dimensions();
    Dimensions(const MAGNITUDE_TYPE& n);
    Dimensions(const MAGNITUDE_TYPE& n, const BaseDimensions& p) : utype(Utype::NUL), numerical(n), physical(p) {};
#ifdef MAGNITUDE_ERRORS
    Dimensions(const MAGNITUDE_PRECISION& m, const MAGNITUDE_PRECISION& e);
    Dimensions(const MAGNITUDE_PRECISION& m, const MAGNITUDE_PRECISION& e, const BaseDimensions& p) : utype(Utype::NUL), numerical(m, e), physical(p) {};
#endif
    std::string to_string(const UnitFormat& format = UnitFormat()) const;
    friend std::ostream& operator<<(std::ostream& os, const Dimensions& d);
    bool operator==(const Dimensions& d) const;
    bool operator!=(const Dimensions& d) const;
  };

} // namespace snt::puq

#endif // PUQ_DIMENSIONS_H
