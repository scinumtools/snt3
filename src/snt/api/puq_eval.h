#ifndef SNT_CLI_PUQ_EVAL
#define SNT_CLI_PUQ_EVAL

#include <snt/puq/lists.h>
#include <string>

namespace snt::api {

    /**
     * Evaluation of PUQ unit expressions
     */
    class PUQEval {

        std::string expression;        ///< Unit expression that should be evaluated
        puq::SystemType input_system;  ///< Input unit system
        puq::SystemType output_system; ///< Output unit system
        std::string output_units;      ///< Output units
        std::string output_quantity;   ///< Output quantity

      public:
        /**
         * Default class constructor
         **/
        PUQEval(const std::string& expr)
            : expression(expr), input_system(puq::SystemType::NONE), output_system(puq::SystemType::NONE) {};

        /**
         * Set input unit system
         *
         * @param system Unit system
         **/
        void argument_input_system(const std::string& isystem);

        /**
         * Set output unit system
         *
         * @param system Unit system
         **/
        void argument_output_system(const std::string& osystem);

        /**
         * Set output units
         *
         * @param units Output units
         **/
        void argument_output_units(const std::string& units);

        /**
         * Set output quantity
         *
         * @param quantity Output quantity
         **/
        void argument_output_quantity(const std::string& quantity);

        /**
         * Executed parsing command
         *
         * @return Parsed output
         */
        std::string execute();
    };

} // namespace snt::api

#endif // SNT_CLI_PUQ_EVAL
