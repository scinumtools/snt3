#ifndef PUQ_SYSTEMS_H
#define PUQ_SYSTEMS_H

#include <array>
#include <map>
#include <set>
#include <snt/puq/result.h>
#include <snt/puq/settings.h>
#include <sstream>
#include <stack>
#include <string>

namespace snt::puq {

    /*
     *  Unit types
     */

    enum class Utype : std::uint8_t {
        NUL = 0b00000000, ///< empty flag
        BAS = 0b00000001, ///< base units
        LIN = 0b00000010, ///< standard unit (linear conversion)
        LOG = 0b00000100, ///< logarithmic unit
        TMP = 0b00001000, ///< temperature unit
        CST = 0b00010000, ///< constant
        CSB = 0b00100000, ///< base constant
        QUA = 0b01000000, ///< quantity
        CUS = 0b10000000, ///< custom unit
    };

    inline Utype operator|(Utype lhs, Utype rhs) {
        return static_cast<Utype>(
            static_cast<std::underlying_type_t<Utype>>(lhs) | static_cast<std::underlying_type_t<Utype>>(rhs)
        );
    }

    inline Utype operator&(Utype lhs, Utype rhs) {
        return static_cast<Utype>(
            static_cast<std::underlying_type_t<Utype>>(lhs) & static_cast<std::underlying_type_t<Utype>>(rhs)
        );
    }

    inline Utype operator^(Utype lhs, Utype rhs) {
        return static_cast<Utype>(
            static_cast<std::underlying_type_t<Utype>>(lhs) ^ static_cast<std::underlying_type_t<Utype>>(rhs)
        );
    }

    const Utype UT_LIN_BAS = Utype::LIN | Utype::BAS;
    const Utype UT_LIN_BAS_CST = Utype::LIN | Utype::BAS | Utype::CST;
    const Utype UT_LIN_BAS_TMP = Utype::LIN | Utype::BAS | Utype::TMP;
    const Utype UT_LIN_TMP = Utype::LIN | Utype::TMP;
    const Utype UT_LIN_CST = Utype::LIN | Utype::CST;
    const Utype UT_LIN_CSB = Utype::LIN | Utype::CSB;
    const Utype UT_LIN_LOG = Utype::LIN | Utype::LOG;
    const Utype UT_LIN_QUA = Utype::LIN | Utype::QUA;
    const Utype UT_LIN_CUS = Utype::LIN | Utype::CUS;

    /*
     *  Prefix and unit list definitions
     */

    struct UnitPrefixStruct {
        Result result;
        std::string definition;
        std::string name;
    };

    extern const std::unordered_map<std::string, UnitPrefixStruct> UnitPrefixList;
    extern const std::vector<std::string> UnitPrefixOrder;

    using BaseDimensions = std::array<ExponentVariant, Config::num_basedim>;
    using AllowedPrefixes = std::set<std::string>;

    struct UnitStruct {
        Utype utype;
        std::string definition;
        std::string name;
        bool use_prefixes;
        AllowedPrefixes allowed_prefixes;
    };
    using UnitListType = std::unordered_map<std::string, UnitStruct>;

    /*
     *  Quantities
     */

#include <snt/puq/systems/quantities.h>

    /*
     *  Dimension map
     */

    class DimensionMapExcept : public std::exception {
      private:
        std::string message;

      public:
        DimensionMapExcept(const std::string& m) : message(m) {}
        const char* what() const noexcept override { return message.c_str(); }
    };

    struct DimensionStruct {
        double estimate;
        double uncertainty;
        BaseDimensions dimensions;
    };
    using DimensionMapType = std::unordered_map<std::string, DimensionStruct>;

    /*
     *  System of units
     */

    struct SystemDataType {
        std::string SystemAbbrev;
        std::string SystemName;
        UnitListType UnitList;
        QuantityListType QuantityList;
        DimensionMapType DimensionMap;
    };

    namespace SystemData {
#include <snt/puq/systems/system_base.h>
        extern SystemDataType SI;  ///< International system of units
        extern SystemDataType ESU; ///< Electrostatic CGS system of units
        extern SystemDataType EMU; ///< Electromagnetic CGS system of units
        extern SystemDataType GU;  ///< Gauss CGS sytem of units
        extern SystemDataType IU;  ///< Imperial units
        extern SystemDataType US;  ///< United States customary system of units
        extern SystemDataType AU;  ///< Atomic system of units
        extern SystemDataType PU;  ///< Planck system of units
        extern SystemDataType SRU; ///< Units for special relativity
        extern SystemDataType GRU; ///< Units for general relativity
        extern SystemDataType GEO; ///< Geometrized units
    } // namespace SystemData

    enum class SystemType {
        SI,    ///< International system of units
        ESU,   ///< Electrostatic CGS system of units
        GU,    ///< Gauss CGS sytem of units
        EMU,   ///< Electromagnetic CGS system of units
        IU,    ///< Imperial units
        US,    ///< United States customary system of units
        AU,    ///< Atomic system of units
        PU,    ///< Planck system of units
        SRU,   ///< Units for special relativity
        GRU,   ///< Units for general relativity
        GEO,   ///< Geometrized units
        COUNT, ///< Number of units systems
        NONE   ///< No unit system selected
    };

} // namespace snt::puq

#endif // PUQ_SYSTEMS_H
