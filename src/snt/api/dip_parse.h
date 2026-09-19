#ifndef SNT_CLI_DIP_PARSE
#define SNT_CLI_DIP_PARSE

#include <snt/dip/dip.h>
#include <optional>
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
        std::string value_type;        ///< Optional scalar output type constraint
        std::string load_file;         ///< Optional DIPH5 input path
        std::string save_file;         ///< Optional DIPH5 output path
        std::optional<dip::ExportFormat> generate_format; ///< Optional static export format
        std::string generate_file;     ///< Optional static export output path
        bool has_input = false;        ///< Whether DIPL inputs have been added

      public:
        /**
         * Default class constructor
         **/
        DIPParse() : print(PrintOptions::NONE) {};

        /**
         * Add code
         *
         * @param add_type Type of the code input
         * @param add_values Code input values
         **/
        void argument_add(const std::string& add_type, const std::vector<std::string>& add_values);

        /// Load an evaluated DIPH5 environment during execute(), instead of parsing.
        /// Cannot be combined with argument_add(). The file path must be nonempty.
        void argument_load(const std::string& file);

        /// Save the complete evaluated environment during execute(), overwriting file.
        /// Request/tag filters affect text output only. The file path must be nonempty.
        /// Saving occurs after successful output validation.
        void argument_save(const std::string& file);

        /// Generate static parameters during execute().
        /// format is one of cpp, c, fortran, rust, julia, json, or yaml.
        void argument_generate(const std::string& format, const std::string& file);

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

        /// Print exactly one unitless scalar, without a name or string quotes.
        /// type is empty, bool, integer, float, or string.
        void argument_value(const std::string& type = "");

        /**
         * Executed parsing command
         *
         * @return Parsed output
         */
        std::string execute();
    };

} // namespace snt::api

#endif // SNT_CLI_DIP_PARSE
