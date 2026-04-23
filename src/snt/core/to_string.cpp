#include <iomanip>
#include <sstream>
#include <string>

namespace snt::core {

  std::string to_string(const double value, int precision) {
    std::stringstream ss;
    ss << std::setprecision(precision);
    ss << value << std::scientific;
    return ss.str();
  }

} // namespace snt::core
