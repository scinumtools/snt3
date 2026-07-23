#ifndef SNT_CLI_DIP_PARSE
#define SNT_CLI_DIP_PARSE

#include <snt/dip/dip.h>
#include <string>
#include <vector>

namespace snt::api {

    /**
     * Parsing of a DIP code
     */
    class DIPParse {
      public:
        enum class PrintOptions {
            NONE, ///< Nothing is printed
            ALL,  ///< Both name and value is printed
            NAME, ///< Only name is printed
            VALUE ///< Only value is printed
        };

      private:
        dip::DIP dip;                  ///< DIP object handle
        std::string request;           ///< Node request query
        std::vector<std::string> tags; ///< List of node tags that should be selected
        PrintOptions print;            ///< Printing option

      public:
        /**
         * Default class constructor
         **/
        DIPParse() : print(PrintOptions::NONE) {};

        /**
         * Add code
         *
         * @param input_type Type of the code input
         * @param input_values Code input values
         **/
        void argument_input(const std::string& input_type, const std::vector<std::string>& input_values);

        /**
         * Set a request path
         *
         * @param path Requested node path
         **/
        void argument_request(const std::string& path);

        /**
         * Set a list of node tags
         *
         * @param list List of tags that should be selected
         **/
        void argument_tags(const std::vector<std::string>& list);

        /**
         * Print parsed nodes with values
         **/
        void argument_print();

        /**
         * Executed parsing command
         *
         * @return Parsed output
         */
        std::string execute();
    };

} // namespace snt::api

#endif // SNT_CLI_DIP_PARSE
