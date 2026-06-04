#ifndef DIP_LIST_UNIT_H
#define DIP_LIST_UNIT_H

#include <map>
#include <string>

namespace snt::dip {

    /**
     * @struct EnvUnit
     * @brief Custom unit data
     */
    struct EnvUnit {
        std::string name;       ///< unit key
        std::string definition; ///< unit definition
        size_t stack;           ///< number of the current unit system stack
    };

    /**
     * @class UnitList
     * @brief List of custom units
     */
    class UnitList {
      private:
        std::map<std::string, EnvUnit> units; ///< Map of custom units

      public:
        /**
         * @brief Unit list constructor
         */
        UnitList();

        /**
         * @brief Append new custom unit from a name and a definition expression
         *
         * @param name Name of a custom unit
         * @param definition Expression that defines a new unit
         */
        void append(const std::string& name, const std::string& definition);

        /**
         * @brief Append new custom unit from a name and a custom unit data
         *
         * @param name Name of a custom unit
         * @param data Custom unit data object
         */
        void append(const std::string& name, EnvUnit data);

        /**
         * @brief Select custom unit data
         *
         * @param name Name of a custom unit
         * @return Custom unit data
         */
        EnvUnit& at(const std::string& name);

        /**
         * @brief Select custom unit data for read-only
         *
         * @param name Name of a custom unit
         * @return Custom unit data
         */
        const EnvUnit& at(const std::string& name) const;
    };

} // namespace snt::dip

#endif // DIP_LIST_UNIT_H
