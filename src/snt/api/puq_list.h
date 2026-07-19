#ifndef SNT_CLI_PUQ_LIST
#define SNT_CLI_PUQ_LIST

#include <snt/puq/lists.h>
#include <string>

namespace snt::api {

    /**
     * Conversion of PUQ unit expressions
     */
    class PUQList {

        std::string list;             ///< List that should be diplayed
        puq::SystemType input_system; ///< Unit system that should be listed

      public:
        /**
         * Default class constructor
         *
         * @param li Name of a list that should be displayed
         **/
        PUQList(const std::string& li = "") : list(li), input_system(puq::SystemType::NONE) {};

        /**
         * Select unit system
         *
         * @param system Unit system
         **/
        void argument_system(const std::string& system);

        /**
         * Executed parsing command
         *
         * @return Parsed output
         */
        std::string execute();
    };

} // namespace snt::api

#endif // SNT_CLI_PUQ_LIST
