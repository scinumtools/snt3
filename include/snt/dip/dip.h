#ifndef DIP_DIP_H
#define DIP_DIP_H

#include "environment.h"

#include <array>
#include <filesystem>
#include <iostream>
#include <queue>

namespace snt::dip {

    class DIP {
      private:
        static int num_instances; ///< counter of DIP class instances
        int instance_number;      ///< instance number of the current class object
        Environment env;          ///< node environment object
        std::queue<Line> lines;   ///< list of code lines that are being processed

        Source source; ///< source information of this class

        static constexpr std::array<NodeDtype, 4> nodes_special = {
            NodeDtype::Empty, NodeDtype::Unit, NodeDtype::Source, NodeDtype::Schema
        }; ///< list of nodes that define some environmental constants or denote empty line
        static constexpr std::array<NodeDtype, 6> nodes_properties = {NodeDtype::Property}; ///< property nodes
        static constexpr std::array<NodeDtype, 1> nodes_hierarchy = {
            NodeDtype::Group
        }; ///< additional nodes that enter node path
        std::vector<NodeDtype> nodes_nohierarchy; ///< list of nodes that do not enter node path
        std::vector<NodeDtype> nodes_notypes;     ///< list of nodes that don't have types

        size_t num_strings = 0; ///< counter of code inputs from a string
        size_t num_files = 0;   ///< counter of code inputs from a file
        size_t num_sources = 0; ///< number of explicitely added sources
        size_t num_units = 0;   ///< number of explicitely added units

      public:
        /**
         * Empty class constructor
         */
        DIP();

        /**
         * Class constructor with a specified source
         * @param src Source of the parent DIP class
         */
        DIP(const Source& src);

        /**
         * Add DIPL code from a string
         * @param source_code Text with a DIPL code
         */
        void add_string(const std::string& source_code);

        /**
         * Add DIPL code from a file
         * @param source_file File name of a DIPL code file
         * @param source_name Specific source name of the nodes
         * @param absolute
         */
        void add_file(const std::filesystem::path& source_file, std::string source_name = {}, bool absolute = false);

        /**
         * Add DIPL source
         * @param source_name Name of the new source
         * @param source_file File name of a DIPL source file
         */
        void add_source(const std::string& source_name, const std::string& source_file);

        /**
         * Add unit definition
         * @param unit_name Name of the new units
         * @param unit_expression Unit definition
         */
        void add_unit(const std::string& unit_name, const std::string& unit_expression);

        /**
         * Add function that returns a value
         * @param name Name of the function
         * @param func Function pointer
         */
        void add_function_value(const std::string& name, FunctionList::DataFunctionType func);

        /**
         * Add function that returns list of nodes
         * @param name Name of the function
         * @param func Function pointer
         */
        void add_function_nodes(const std::string& name, FunctionList::NodesFunctionType func);

        /**
         * Parse DIPL code lines
         */
        Environment parse();

        /**
         * Get a string representation of the current DIPL instance
         * @return String representation of this class
         */
        std::string to_string();
    };

} // namespace snt::dip

#endif // DIP_DIP_H
