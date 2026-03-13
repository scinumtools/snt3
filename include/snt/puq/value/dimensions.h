#ifndef PUQ_DIMENSIONS_H
#define PUQ_DIMENSIONS_H

#include <snt/puq/magnitude.h>
#include <snt/puq/settings.h>
#include <snt/puq/systems/systems.h>

namespace snt::puq {

  class Dimensions {
  public:
    Magnitude numerical;
    BaseDimensions physical;
    std::vector<std::string> symbols;
    Utype utype;
    Dimensions();
    Dimensions(const Magnitude& n);
    Dimensions(const Magnitude& n, const BaseDimensions& p) : utype(Utype::NUL), numerical(n), physical(p) {};
    Dimensions(const double& m, const double& e);
    Dimensions(const double& m, const double& e, const BaseDimensions& p) : utype(Utype::NUL), numerical(m, e), physical(p) {};
    std::string to_string(const UnitFormat& format = UnitFormat()) const;
    friend std::ostream& operator<<(std::ostream& os, const Dimensions& d);
    bool operator==(const Dimensions& d) const;
    bool operator!=(const Dimensions& d) const;
  };

} // namespace snt::puq

#endif // PUQ_DIMENSIONS_H
