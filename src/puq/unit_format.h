#ifndef PUQ_UNIT_FORMAT_H
#define PUQ_UNIT_FORMAT_H

#include <iostream>
#include <optional>
#include <string>
#include <unordered_map>

#include "settings.h"

namespace puq {

  template <typename T, typename... Args>
  std::optional<T> get_option(const Args&... args) {
    std::optional<T> result;
    ([&](const auto& arg) {
      if constexpr (std::is_same_v<std::decay_t<decltype(arg)>, T>) {
        result = arg;
      }
    }(args),
     ...);
    return result;
  };

  namespace Format {
    enum class System { SHOW,
                        HIDE };
    enum class Math { ASCII,
                      UNICODE,
                      HTML,
                      MATH };
    enum class Display { BOTH,
                         MAGNITUDE,
                         UNITS };
    enum class Base { UNITS,
                      MGS,
                      MKS,
                      CGS,
                      FPS };
    enum class Error { SHOW,
                       HIDE };
    typedef int Precision;
  } // namespace Format

  class UnitFormat {
    static const std::unordered_map<char, std::string> superscript_map;
    static const std::unordered_map<std::string, std::string> symbol_map;

  public:
    Format::System system;
    Format::Precision precision;
    Format::Math math;
    Format::Display part;
    Format::Base base;
    Format::Error error;

    UnitFormat(const auto&... args) {
      math = get_option<Format::Math>(args...).value_or(Format::Math::ASCII);
      precision = get_option<Format::Precision>(args...).value_or(std::cout.precision());
      system = get_option<Format::System>(args...).value_or(Format::System::HIDE);
      part = get_option<Format::Display>(args...).value_or(Format::Display::BOTH);
      base = get_option<Format::Base>(args...).value_or(Format::Base::UNITS);
      error = get_option<Format::Error>(args...).value_or(Format::Error::SHOW);
    };

    UnitFormat merge(const auto&... args) const {
      UnitFormat uf(get_option<Format::Math>(args...).value_or(math),
                    get_option<Format::Precision>(args...).value_or(precision),
                    get_option<Format::System>(args...).value_or(system),
                    get_option<Format::Display>(args...).value_or(part),
                    get_option<Format::Base>(args...).value_or(base),
                    get_option<Format::Error>(args...).value_or(error));
      return uf;
    }

    static bool preprocess_system(std::string& expression, const std::string& abbrev);
    static void preprocess_symbols(std::string& expression);
    std::string multiply_symbol() const;
    std::string format_exponents(std::string expression) const;
    std::string format_order(std::string expression) const;
    std::string format_system(std::string expression, const std::string& abbrev) const;
    bool display_magnitude() const;
    bool display_units() const;
    bool display_error() const;
  };

} // namespace puq

#endif // PUQ_UNIT_FORMAT_H
