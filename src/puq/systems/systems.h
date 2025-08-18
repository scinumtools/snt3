#ifndef PUQ_SYSTEMS_H
#define PUQ_SYSTEMS_H

#include "../magnitude.h"
#include "../settings.h"

#include <map>
#include <set>
#include <sstream>
#include <stack>
#include <string>

namespace puq {

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

  typedef std::array<EXPONENT_TYPE, NUM_BASEDIM> BaseDimensions;
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
    DimensionMapExcept(std::string m) : message(m) {}
    const char* what() const noexcept override {
      return message.c_str();
    }
  };

  struct DimensionStruct {
    MAGNITUDE_PRECISION magnitude;
    MAGNITUDE_PRECISION error;
    BaseDimensions dimensions;
  };
  typedef std::unordered_map<std::string, DimensionStruct> DimensionMapType;

  /*
   *  System of units
   */

#ifdef EXPONENT_FRACTIONS
  typedef EXPONENT_INT_PRECISION FRC[2];
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
#ifdef UNIT_SYSTEM_CGS
    extern SystemDataType ESU;
    extern SystemDataType EMU;
    extern SystemDataType GU;
#endif
#ifdef UNIT_SYSTEM_EUS
    extern SystemDataType IU;
    extern SystemDataType US;
#endif
#ifdef UNIT_SYSTEM_NUS
    extern SystemDataType AU;
    extern SystemDataType PU;
    extern SystemDataType SRU;
    extern SystemDataType GRU;
    extern SystemDataType GEO;
#endif
  } // namespace SystemData
  enum class SystemType {
    NONE,
    SI,
#ifdef UNIT_SYSTEM_CGS
    ESU,
    GU,
    EMU,
#endif
#ifdef UNIT_SYSTEM_EUS
    IU,
    US,
#endif
#ifdef UNIT_SYSTEM_NUS
    AU,
    PU,
    SRU,
    GRU,
    GEO,
#endif
  };
  extern std::unordered_map<SystemType, SystemDataType*> SystemMap;

  class UnitSystemExcept : public std::exception {
  private:
    std::string message;

  public:
    UnitSystemExcept(const std::string m) : message(m) {}
    UnitSystemExcept(const SystemType st1, const SystemType st2) {
      auto it1 = SystemMap.find(st1);
      auto it2 = SystemMap.find(st2);
      if (it1 == SystemMap.end() || it2 == SystemMap.end()) {
        message = "Unknown system type";
      } else {
        std::stringstream ss;
        ss << "Incompatible unit systems: ";
        ss << it1->second->SystemAbbrev;
        ss << " != ";
        ss << it2->second->SystemAbbrev;
        ss << std::endl;
        message = ss.str();
      }
    }
    const char* what() const noexcept override {
      return message.c_str();
    }
  };

  class UnitSystem {
    bool closed;
    static std::stack<SystemDataType*> systemStack;

  public:
    static SystemDataType* Data;
    static SystemType System;
    UnitSystem(const SystemType st = SystemType::NONE);
    ~UnitSystem();
    void change(const SystemType st);
    void close();
  };

} // namespace puq

#endif // PUQ_SYSTEMS_H
