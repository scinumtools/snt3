#ifndef PUQ_SYSTEMS_H
#define PUQ_SYSTEMS_H

#include "../magnitude.h"
#include "../settings.h"

#include <map>
#include <set>
#include <sstream>
#include <stack>
#include <string>

namespace snt::puq {

  /*
   *  Unit types
   */

  enum class Utype : std::uint8_t {
    NUL = 0b00000000, // empty flag
    BAS = 0b00000001, // base units
    LIN = 0b00000010, // standard unit (linear conversion)
    LOG = 0b00000100, // logarithmic unit
    TMP = 0b00001000, // temperature unit
    CST = 0b00010000, // constant
    CSB = 0b00100000, // base constant
    QUA = 0b01000000, // quantity
  };

  inline Utype operator|(Utype lhs, Utype rhs) {
    return static_cast<Utype>(
        static_cast<std::underlying_type_t<Utype>>(lhs) |
        static_cast<std::underlying_type_t<Utype>>(rhs));
  }

  inline Utype operator&(Utype lhs, Utype rhs) {
    return static_cast<Utype>(
        static_cast<std::underlying_type_t<Utype>>(lhs) &
        static_cast<std::underlying_type_t<Utype>>(rhs));
  }

  inline Utype operator^(Utype lhs, Utype rhs) {
    return static_cast<Utype>(
        static_cast<std::underlying_type_t<Utype>>(lhs) ^
        static_cast<std::underlying_type_t<Utype>>(rhs));
  }

  const Utype UT_LIN_BAS = Utype::LIN | Utype::BAS;
  const Utype UT_LIN_BAS_CST = Utype::LIN | Utype::BAS | Utype::CST;
  const Utype UT_LIN_BAS_TMP = Utype::LIN | Utype::BAS | Utype::TMP;
  const Utype UT_LIN_TMP = Utype::LIN | Utype::TMP;
  const Utype UT_LIN_CST = Utype::LIN | Utype::CST;
  const Utype UT_LIN_CSB = Utype::LIN | Utype::CSB;
  const Utype UT_LIN_LOG = Utype::LIN | Utype::LOG;
  const Utype UT_LIN_QUA = Utype::LIN | Utype::QUA;

  /*
   *  Prefix and unit list definitions
   */

  struct UnitPrefixStruct {
    MAGNITUDE_TYPE magnitude;
    std::string definition;
    std::string name;
  };
  extern const std::unordered_map<std::string, UnitPrefixStruct> UnitPrefixList;
  extern const std::vector<std::string> UnitPrefixOrder;

  typedef std::array<EXPONENT_TYPE, Config::num_basedim> BaseDimensions;
  typedef std::set<std::string> AllowedPrefixes;
  struct UnitStruct {
    Utype utype;
    std::string definition;
    std::string name;
    bool use_prefixes;
    AllowedPrefixes allowed_prefixes;
  };
  typedef std::unordered_map<std::string, UnitStruct> UnitListType;

  /*
   *  Quantities
   */

#include "quantities.h"

  /*
   *  Dimension map
   */

  class DimensionMapExcept : public std::exception {
  private:
    std::string message;

  public:
    DimensionMapExcept(const std::string& m) : message(m) {}
    const char* what() const noexcept override {
      return message.c_str();
    }
  };

  struct DimensionStruct {
    MagnitudeFloat magnitude;
    MagnitudeFloat error;
    BaseDimensions dimensions;
  };
  typedef std::unordered_map<std::string, DimensionStruct> DimensionMapType;

  /*
   *  System of units
   */

#ifdef EXPONENT_FRACTIONS
  typedef ExponentInt FRC[2];
#endif

  struct SystemDataType {
    std::string SystemAbbrev;
    std::string SystemName;
    UnitListType UnitList;
    QuantityListType QuantityList;
    DimensionMapType DimensionMap;
  };
  namespace SystemData {
#include "system_base.h"
    extern SystemDataType SI;
    extern SystemDataType ESU;
    extern SystemDataType EMU;
    extern SystemDataType GU;
    extern SystemDataType IU;
    extern SystemDataType US;
    extern SystemDataType AU;
    extern SystemDataType PU;
    extern SystemDataType SRU;
    extern SystemDataType GRU;
    extern SystemDataType GEO;
  } // namespace SystemData
  enum class SystemType {
    NONE,
    SI,
    ESU,
    GU,
    EMU,
    IU,
    US,
    AU,
    PU,
    SRU,
    GRU,
    GEO,
  };

} // namespace snt::puq

#endif // PUQ_SYSTEMS_H
