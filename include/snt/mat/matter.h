#ifndef MAT_MATTER_H
#define MAT_MATTER_H

#include <snt/puq/quantity.h>

namespace snt::mat {

    /**
     * @class Matter
     *  Matter is anything that has mass and occupies space
     */
    class Matter {
      public:
        /**
         * @struct MatterProperties
         *  Set of matter defining properties
         */
        struct MatterProperties {
            puq::Quantity volume; ///< Volume
            puq::Quantity mass;   ///< Mass
        } matterProps;            ///< set of basic matter properties

        puq::Quantity mass_density;   ///< Mass density
        puq::Quantity number_density; ///< Number density
    };

} // namespace snt::mat

#endif // MAT_MATTER_H
