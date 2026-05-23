#ifndef SNT_CLI_PUQ_LIST
#define SNT_CLI_PUQ_LIST

#include <snt/puq/lists.h>
#include <string>

namespace snt::cli {

    /**
     * @class PUQList
     * @brief Conversion of PUQ unit expressions
     */
    class PUQList {

        std::string list;             ///< List that should be diplayed
        puq::SystemType input_system; ///< Unit system that should be listed

      public:
        /**
         * @brief Default class constructor
         *
         * @param li Name of a list that should be displayed
         **/
        PUQList(const std::string& li = "") : list(li),
                                              input_system(puq::SystemType::NONE) {};

        /**
         * @brief Select unit system
         *
         * @param system Unit system
         **/
        void argument_system(const std::string& system);

        /**
         * @brief Executed parsing command
         */
        void execute();
    };

} // namespace snt::cli

#endif // SNT_CLI_PUQ_LIST
