#ifndef PUQ_DIMENSIONS_H
#define PUQ_DIMENSIONS_H

#include <snt/puq/result.h>
#include <snt/puq/settings.h>
#include <snt/puq/systems/systems.h>

namespace snt::puq {

    class Dimensions {
      public:
        Result numerical;
        BaseDimensions physical;
        std::vector<std::string> symbols;
        Utype utype;
        Dimensions();
        Dimensions(const Result& n);
        Dimensions(const Result& n, const BaseDimensions& p) : utype(Utype::NUL), numerical(n), physical(p) {};
        Dimensions(const double m, const double e);
        Dimensions(const double m, const double e, const BaseDimensions& p)
            : utype(Utype::NUL), numerical(m, e), physical(p) {};
        /** Format the object as text.
         * @param format Formatting options controlling the textual representation.
         */
        std::string to_string(const UnitFormat& format = UnitFormat()) const;

        /**
         *  Test if there are physical dimensions
         *
         * @return Returns true if there are some physical dimensions; otherwise returns false
         */
        bool has_dimensions() const;

        friend std::ostream& operator<<(std::ostream& os, const Dimensions& d);
        /** Compare two dimension descriptors, including numerical and physical parts.
         * @param d Dimension descriptor to compare.
         */
        bool operator==(const Dimensions& d) const;
        /** Compare two dimension descriptors for inequality.
         * @param d Dimension descriptor to compare.
         */
        bool operator!=(const Dimensions& d) const;
    };

} // namespace snt::puq

#endif // PUQ_DIMENSIONS_H
