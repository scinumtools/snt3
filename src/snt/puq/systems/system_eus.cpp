#import "systems.h"

namespace snt::puq {
  namespace SystemData {

    inline UnitListType GetEUSUnits() {
      UnitListType units = {
        // units of length
        {"twip", {Utype::LIN, "17.6388888e-6*m", "twip", false, {}}},
        {"th", {Utype::LIN, "25.4e-6*m", "thou", false, {}}},
        {"mil", {Utype::LIN, "th", "mil", false, {}}},
        {"in", {Utype::LIN, "0.0254*m", "inch", false, {}}},
        {"''", {Utype::LIN, "in", "inch", false, {}}},
        {"li", {Utype::LIN, "0.201168*m", "link", false, {}}},
        {"ft", {Utype::LIN, "0.3048*m", "foot", false, {}}},
        {"'", {Utype::LIN, "ft", "foot", false, {}}},
        {"yd", {Utype::LIN, "0.9144*m", "yard", false, {}}},
        {"rd", {Utype::LIN, "5.0292*m", "rod/pole/perch", false, {}}},
        {"ch", {Utype::LIN, "20.1168*m", "chain", false, {}}},
        {"fur", {Utype::LIN, "201.168*m", "furlong", false, {}}},
        {"mi", {Utype::LIN, "1609.344*m", "mile", false, {}}},
        {"lea", {Utype::LIN, "4828.032*m", "league", false, {}}},
        {"le", {Utype::LIN, "lea", "league", false, {}}},

        // international nautical units
        {"ftm", {Utype::LIN, "1.8288*m", "fathom", false, {}}},
        {"cb", {Utype::LIN, "219.456*m", "cable", false, {}}},
        {"nmi", {Utype::LIN, "1852*m", "nautical mile", false, {}}},
        {"NM", {Utype::LIN, "nmi", "nautical mile", false, {}}},

        // units of area
        {"ac", {Utype::LIN, "4046.8564224*m2", "acre", false, {}}},

        // units of mass
        {"gr", {Utype::LIN, "64.79891e-3*g", "grain", false, {}}},
        {"dr", {Utype::LIN, "1.771845195*g", "drachm/dram", false, {}}},
        {"oz", {Utype::LIN, "28.349523125*g", "ounce", false, {}}},
        {"lb", {Utype::LIN, "453.59237*g", "pund", false, {}}},
        {"cwt", {Utype::LIN, "50.80234544e3*g", "long hundredweight", false, {}}},
        {"ton", {Utype::LIN, "1.0160469088e6*g", "long ton", false, {}}},
        {"slug", {Utype::LIN, "14.59390294e3*g", "slug", false, {}}},

        // units of time
        {"min", {Utype::LIN, "60*s", "minute", false, {}}},
        {"h", {Utype::LIN, "60*min", "hour", false, {}}},
        {"day", {Utype::LIN, "24*h", "day", false, {}}},
        {"yr_t", {Utype::LIN, "365.24219*day", "tropical year", true, {"k", "m", "G"}}},
        {"yr_j", {Utype::LIN, "365.25*day", "Julian year", true, {"k", "m", "G"}}},
        {"yr_g", {Utype::LIN, "365.2425*day", "Gregorian year", true, {"k", "m", "G"}}},
        {"yr", {Utype::LIN, "yr_j", "year", true, {"k", "m", "G"}}},

        // units of temperature
        {"degR", {UT_LIN_TMP, "5/9*K", "degree Rankine", false, {}}},

        // units of velocity
        {"mph", {Utype::LIN, "mi/h", "miles per hour", false, {}}}
      };
      
      if constexpr (Config::use_units_temperature) {
	units.insert({
	  {"Cel", {Utype::TMP, "K", "degree Celsius", false, {}}},
	  {"degF", {Utype::TMP, "K", "degree Fahrenheit", false, {}}},
        });
      }
      
      return units;
    };

    UnitListType _EUS_UNITS = GetEUSUnits();
    
    const QuantityListType _EUS_QUANTITIES = {
        {"l", {"ft"}},
        {"m", {"lb"}},
        {"t", {"s"}},
        {"Iv", {"cd"}},
        {"n", {"mol"}},
        {"the", {"rad"}},
        {"V", {"gal"}},
        {"A", {"ac"}},
        {"v", {"mph"}},
    };

    SystemDataType IU = {
        "IU", "Imperial units",
        _BASE_UNITS + _EUS_UNITS + UnitListType({

                                       // units of length
                                       {"bc", {Utype::LIN, "8.4667e-3*m", "barleycorn", false, {}}},
                                       {"hh", {Utype::LIN, "0.1016*m", "hand", false, {}}},

                                       // units of area
                                       {"sqrd", {Utype::LIN, "25.29285264*m2", "sq. rod/po./per.", false, {}}},
                                       {"ro", {Utype::LIN, "1011.7141056*m2", "rood", false, {}}},
                                       {"sqmi", {Utype::LIN, "2589988.110336*m2", "square mile", false, {}}},

                                       // units of volume
                                       {"minim", {Utype::LIN, "59.1938802083e-9*m3", "minim", false, {}}},
                                       {"fls", {Utype::LIN, "1.18387760416e-6*m3", "fluid scruple", false, {}}},
                                       {"fldr", {Utype::LIN, "3.5516328125e-6*m3", "fluid drachm", false, {}}},
                                       {"floz", {Utype::LIN, "28.4130625e-6*m3", "fluid ounce", false, {}}},
                                       {"gi", {Utype::LIN, "142.0653125e-6*m3", "gill", false, {}}},
                                       {"pt", {Utype::LIN, "568.26125e-6*m3", "pint", false, {}}},
                                       {"qt", {Utype::LIN, "1136.5225e-6*m3", "quart", false, {}}},
                                       {"gal", {Utype::LIN, "4.54609e-3*m3", "gallon", false, {}}},
                                       {"bblo", {Utype::LIN, "0.16365924*m3", "oil barrel", false, {}}},

                                       // units of mass
                                       {"st", {Utype::LIN, "6.35029318e3*g", "stone", false, {}}},
                                       {"qr", {Utype::LIN, "12.70058636e3*g", "quarter", false, {}}},

                                   }),
        _EUS_QUANTITIES + QuantityListType({}),
        DimensionMapType({
#include "dmaps/dmap_IU.h"
        })};

    SystemDataType US = {
        "US", "United States customary units",
        _BASE_UNITS + _EUS_UNITS + UnitListType({

                                       // units of length
                                       {"p", {Utype::LIN, "352.778e-6*m", "point", false, {}}},
                                       {"P", {Utype::LIN, "4.233e-3*m", "pica", false, {}}},

                                       // units of area
                                       {"sqft", {Utype::LIN, "0.09290341*m2", "sq. survey foot", false, {}}},
                                       {"sqch", {Utype::LIN, "404.68564224*m2", "square chain", false, {}}},
                                       {"section", {Utype::LIN, "2.589998e6*m2", "section", false, {}}},
                                       {"twp", {Utype::LIN, "93.23993e6*m2", "survey township", false, {}}},

                                       // cubic units of volume
                                       {"cuin", {Utype::LIN, "16.387064e-6*m3", "cubic inch", false, {}}},
                                       {"cuft", {Utype::LIN, "28.316846592e-3*m3", "cubic foot", false, {}}},
                                       {"cuyd", {Utype::LIN, "0.764554857984*m3", "cubic yard", false, {}}},
                                       {"acft", {Utype::LIN, "1233.482*m3", "acre-foot", false, {}}},
                                       // fluid units of volume
                                       {"minim", {Utype::LIN, "61.611519921875e-9*m3", "minim", false, {}}},
                                       {"fldr", {Utype::LIN, "3.6966911953125e-6*m3", "fluid drachm", false, {}}},
                                       {"tsp", {Utype::LIN, "4.92892159375e-6*m3", "teaspoon", false, {}}},
                                       {"tbsp", {Utype::LIN, "14.78676478125e-6*m3", "tablespoon", false, {}}},
                                       {"floz", {Utype::LIN, "29.5735295625e-6*m3", "fluid ounce", false, {}}},
                                       {"jig", {Utype::LIN, "44.36029434375e-6*m3", "jig", false, {}}},
                                       {"gi", {Utype::LIN, "118.29411825e-6*m3", "gill", false, {}}},
                                       {"c", {Utype::LIN, "236.5882365e-6*m3", "cup", false, {}}},
                                       {"pt", {Utype::LIN, "0.473176473e-3*m3", "pint", false, {}}},
                                       {"qt", {Utype::LIN, "0.946352946e-3*m3", "quart", false, {}}},
                                       {"pot", {Utype::LIN, "1.892705892e-3*m3", "pottle", false, {}}},
                                       {"gal", {Utype::LIN, "3.785411784e-3*m3", "gallon", false, {}}},
                                       {"bbl", {Utype::LIN, "119.240471196e-3*m3", "liquid barrel", false, {}}},
                                       {"bblo", {Utype::LIN, "158.987294928e-3*m3", "oil barrel", false, {}}},
                                       {"hhd", {Utype::LIN, "238.480942392e-3*m3", "hogshead", false, {}}},
                                       // dry units of volume
                                       {"ptd", {Utype::LIN, "0.55061047135749e-3*m3", "dry pint", false, {}}},
                                       {"qtd", {Utype::LIN, "1.1012209427149e-3*m3", "dry quart", false, {}}},
                                       {"gald", {Utype::LIN, "4.4048837708599e-3*m3", "dry gallon", false, {}}},
                                       {"pk", {Utype::LIN, "8.8097675417199e-3*m3", "peck", false, {}}},
                                       {"bu", {Utype::LIN, "35.239070166879e-3*m3", "bushel", false, {}}},
                                       {"bbll", {Utype::LIN, "115.62712358400e-3*m3", "liquid barrel", false, {}}},

                                       // units of mass
                                       {"cwts", {Utype::LIN, "45.359237e3*g", "short hundredwe.", false, {}}},
                                       {"tons", {Utype::LIN, "907.18474e3*g", "short ton", false, {}}},
                                       {"dwt", {Utype::LIN, "1.55517384*g", "pennyweight", false, {}}},
                                       {"ozt", {Utype::LIN, "31.1034768*g", "troy ounce", false, {}}},
                                       {"lbt", {Utype::LIN, "373.241721*g", "troy pound", false, {}}},

                                   }),
        _EUS_QUANTITIES + QuantityListType({}),
        DimensionMapType({
#include "dmaps/dmap_US.h"
        })};

  } // namespace SystemData
} // namespace snt::puq
