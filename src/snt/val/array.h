#ifndef VAL_ARRAY_H
#define VAL_ARRAY_H

#include <cstdlib>
#include <limits>
#include <string>
#include <vector>

namespace snt::val {

  // Define common array types
  namespace Array {
    struct RangeStruct {
      size_t dmin;
      size_t dmax;
      bool operator==(const RangeStruct& other) const {
        return (dmin == other.dmin) && (dmax == other.dmax);
      };
    };
    typedef std::vector<std::string> StringType; // holds raw string values
    typedef std::vector<RangeStruct> RangeType;  // array dimension ranges
    typedef std::vector<size_t> ShapeType;       // array shape
    constexpr size_t max_range = std::numeric_limits<size_t>::max();
  } // namespace Array

} // namespace snt::val

#endif // VAL_ARRAY_H
