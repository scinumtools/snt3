#ifndef SNT_CLI_PUQ_CONVERT
#define SNT_CLI_PUQ_CONVERT

#include <snt/puq/lists.h>
#include <string>

namespace snt::api {

    /**
     * @class PUQConvert
     * @brief Conversion of PUQ unit expressions
     */
    class PUQConvert {

        std::string expression;        ///< Unit expression that should be converted
        puq::SystemType input_system;  ///< Input unit system
        puq::SystemType output_system; ///< Output unit system
        std::string output_units;      ///< Output units
        std::string output_quantity;   ///< Output quantity

      public:
        /**
         * @brief Default class constructor
         **/
        PUQConvert(const std::string& expr1, const std::string& expr2)
            : expression(expr1), output_units(expr2), input_system(puq::SystemType::NONE),
              output_system(puq::SystemType::NONE) {};

        /**
         * @brief Set input unit system
         *
         * @param system Unit system
         **/
        void argument_input_system(const std::string& system);

        /**
         * @brief Set output unit system
         *
         * @param system Unit system
         **/
        void argument_output_system(const std::string& system);

        /**
         * @brief Set output quantity
         *
         * @param quantity Output quantity
         **/
        void argument_output_quantity(const std::string& quantity);

        /**
         * @brief Executed parsing command
         *
         * @return Parsed output
         */
        std::string execute();
    };

} // namespace snt::api

#endif // SNT_CLI_PUQ_CONVERT
