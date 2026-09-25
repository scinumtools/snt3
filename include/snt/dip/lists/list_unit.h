#ifndef DIP_LIST_UNIT_H
#define DIP_LIST_UNIT_H

#include <map>
#include <limits>
#include <string>

namespace snt::dip {

    /**
     * Custom unit data
     */
    struct EnvUnit {
        std::string name;       ///< unit key
        std::string definition; ///< unit definition
        size_t stack;           ///< number of the current unit system stack
        std::string id;         ///< Internal trace identifier, e.g. DIP0_UNIT0.
        size_t registration_order = std::numeric_limits<size_t>::max(); ///< Environment-local registration order.
    };

    /**
     * List of custom units
     */
    class UnitList {
      private:
        std::map<std::string, EnvUnit> units; ///< Map of custom units
        std::map<std::string, size_t> id_counters;
        size_t next_registration_order = 0;

      public:
        /**
         * Unit list constructor
         */
        UnitList();

        /**
         * Append new custom unit from a name and a definition expression
         *
         * @param name Name of a custom unit
         * @param definition Expression that defines a new unit
         * @param parent_id Internal identifier of the registering source.
         */
        void append(const std::string& name, const std::string& definition, const std::string& parent_id = {});

        /**
         * Append new custom unit from a name and a custom unit data
         *
         * @param name Name of a custom unit
         * @param data Custom unit data object
         * @param parent_id Internal identifier of the registering source.
         */
        void append(const std::string& name, EnvUnit data, const std::string& parent_id = {});

        /**
         * Select custom unit data
         *
         * @param name Name of a custom unit
         * @return Custom unit data
         */
        EnvUnit& at(const std::string& name);

        /**
         * Select custom unit data for read-only
         *
         * @param name Name of a custom unit
         * @return Custom unit data
         */
        const EnvUnit& at(const std::string& name) const;

        /** Return all registered custom units keyed by unit name. */
        const std::map<std::string, EnvUnit>& entries() const;
    };

} // namespace snt::dip

#endif // DIP_LIST_UNIT_H
