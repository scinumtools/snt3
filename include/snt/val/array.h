#ifndef VAL_ARRAY_H
#define VAL_ARRAY_H

#include <cstdlib>
#include <limits>
#include <string>
#include <vector>

namespace snt::val {

    /**
     * @namespace Array
     * @brief Define common array types
     */
    namespace Array {
        /**
         * @struct RangeStruct
         * @brief Array dimension range
         */
        struct RangeStruct {
            size_t dmin; ///<  Dimension range minimum
            size_t dmax; ///<  Dimension range maximum

            /**
             * @brief Compare equalit of two ranges
             *
             * @param other Other range that will be compared
             */
            bool operator==(const RangeStruct& other) const { return (dmin == other.dmin) && (dmax == other.dmax); };
        };

        using StringType = std::vector<std::string>;                     ///< Holds raw string values
        using RangeType = std::vector<RangeStruct>;                      ///< Array dimension ranges
        using ShapeType = std::vector<size_t>;                           ///< Array shape
        constexpr size_t max_range = std::numeric_limits<size_t>::max(); ///< Maximum size value indicates open range
    } // namespace Array

} // namespace snt::val

#endif // VAL_ARRAY_H
