#import <snt/puq/systems/systems.h>

namespace snt::puq {

  const std::vector<std::string> UnitPrefixOrder = {
      "Y", "Z", "E", "P", "T", "G", "M", "k", "h", "da",
      "d", "c", "m", "u", "n", "p", "f", "a", "z", "y"};

  const std::unordered_map<std::string, UnitPrefixStruct> UnitPrefixList = {
      {"Y", {1.0e24, "1e24", "yotta"}},
      {"Z", {1.0e21, "1e21", "zetta"}},
      {"E", {1.0e18, "1e18", "exa"}},
      {"P", {1.0e15, "1e15", "peta"}},
      {"T", {1.0e12, "1e12", "tera"}},
      {"G", {1.0e9, "1e9", "giga"}},
      {"M", {1.0e6, "1e6", "mega"}},
      {"k", {1.0e3, "1e3", "kilo"}},
      {"h", {1.0e2, "1e2", "hecto"}},
      {"da", {1.0e1, "1e1", "deka"}},
      {"d", {1.0e-1, "1e-1", "deci"}},
      {"c", {1.0e-2, "1e-2", "centi"}},
      {"m", {1.0e-3, "1e-3", "milli"}},
      {"u", {1.0e-6, "1e-6", "micro"}},
      {"n", {1.0e-9, "1e-9", "nano"}},
      {"p", {1.0e-12, "1e-12", "pico"}},
      {"f", {1.0e-15, "1e-15", "femto"}},
      {"a", {1.0e-18, "1e-18", "atto"}},
      {"z", {1.0e-21, "1e-21", "zepto"}},
      {"y", {1.0e-24, "1e-24", "yocto"}},
  };

} // namespace snt::puq
