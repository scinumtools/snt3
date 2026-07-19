#ifndef SNT_CLI_DIP_PARSE
#define SNT_CLI_DIP_PARSE

#include <snt/dip/dip.h>
#include <string>
#include <vector>

namespace snt::api {

    /**
     * @class DIPParse
     *  Parsing of a DIP code
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
         *  Default class constructor
         **/
        DIPParse() : print(PrintOptions::NONE) {};

        /**
         *  Add code from a file
         *
         * @param file_name Name of a DIP file
         **/
        void argument_file(const std::string& file_name);

        /**
         *  Add code from a list of string
         *
         * @param list List of DIP code
         **/
        void argument_string(const std::vector<std::string>& list);

        /**
         *  Add code from a string
         *
         * @param code DIP code
         **/
        void argument_string(const std::string& code);

        /**
         *  Set a request path
         *
         * @param path Requested node path
         **/
        void argument_request(const std::string& path);

        /**
         *  Set a list of node tags
         *
         * @param list List of tags that should be selected
         **/
        void argument_tags(const std::vector<std::string>& list);

        /**
         *  Print parsed nodes with values
         **/
        void argument_print();

        /**
         *  Executed parsing command
         *
         * @return Parsed output
         */
        std::string execute();
    };

} // namespace snt::api

#endif // SNT_CLI_DIP_PARSE
