#ifndef SNT_CLI_PUQ_INFO
#define SNT_CLI_PUQ_INFO

#include <snt/puq/lists.h>
#include <string>

namespace snt::cli {

    /**
     * @class PUQInfo
     * @brief Display information about PUQ expressions
     */
    class PUQInfo {

        std::string expression;       ///< Input PUQ expression
        puq::SystemType input_system; ///< Input unit system

      public:
        /**
         * @brief Default class constructor
         **/
        PUQInfo(const std::string& expr) : expression(expr),
                                           input_system(puq::SystemType::NONE) {};

        /**
         * @brief Set input unit system
         *
         * @param system Unit system
         **/
        void argument_input_system(const std::string& isystem);

        /**
         * @brief Executed parsing command
         */
        void execute();
    };

} // namespace snt::cli

#endif // SNT_CLI_PUQ_INFO
