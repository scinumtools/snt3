#ifndef PUQ_UNIT_SYSTEM_H
#define PUQ_UNIT_SYSTEM_H

#include <snt/puq/systems/systems.h>

namespace snt::puq {

    enum class SystemType; ///< Forward definition of the SystemType list

    extern std::unordered_map<SystemType, SystemDataType*>
        SystemMap; ///< Map of sytem types and corresponding pointers to the system data structures

    extern std::unordered_map<SystemType, SystemDataType> CustomSystemMap; ///< Map of custom units for each unit system

    /**
     * @class UnitSystem
     * @brief Manages a pointer to the central unit-system registry (`current`).
     *
     * The pointer may reference any unit system registered in `SystemMap`.
     * When a `UnitSystem` instance is opened, the pointer is redirected to
     * the selected unit-system data, while the previously active registry
     * pointer is pushed onto `stack`.
     *
     * Multiple `UnitSystem` instances may be open simultaneously. In that
     * case, they must be closed in the reverse order in which they were
     * opened (LIFO order). Each open operation pushes a new entry onto
     * `stack`, and each close operation restores the previous registry
     * by popping the corresponding entry from the stack.
     */
    class UnitSystem {
      public:
        /**
         * @struct Record
         * @brief Holds record of an openned unit system
         */
        struct Record {
            SystemType type;
            SystemDataType* data;
            SystemDataType* custom;
        };

      private:
        bool closed;                     ///< Indicates if the current instance of the UnitSystem was manually closed
        static std::stack<Record> stack; ///< Stack of previously openned unit systems

      public:
        static Record current; ///< Record of a current openned unit system

        /**
         * @brief Set a custom unit into the current unit system record
         *
         * @param name Unit name
         * @param definition Defining unit expression
         * @return Current stack number
         */
        static size_t set_custom_unit(const std::string& name, const std::string& definition);

        /**
         * @brief Constructor of the UnitSystem class
         *
         * @param st Unit system type
         */
        UnitSystem(const SystemType st = SystemType::NONE);

        /**
         * @brief Destructor of the UnitSystem class
         */
        ~UnitSystem();

        /**
         * @brief Change currently openned unit system
         *
         * @param st New unit system type
         */
        void change(const SystemType st);

        /**
         * @brief Close the currently openned unit system
         */
        void close();

        /**
         * @brief String representation of this class
         *
         * @return String representation
         */
        const std::string to_string() const;
    };

} // namespace snt::puq

#endif // PUQ_UNIT_SYSTEM_H
