#include "main.h"
#include "help.h"

#include <filesystem>
#include <iostream>
#include <sstream>
#include <snt/api/dip_semantic.h>
#include <stdexcept>
#include <string>
#include <vector>

namespace {
    std::size_t count(const std::string& value, const std::string& option) {
        if (value.empty() || value.find_first_not_of("0123456789") != std::string::npos)
            throw std::invalid_argument(option + " requires a nonnegative integer.");
        return static_cast<std::size_t>(std::stoull(value));
    }
} // namespace

int module_dip_semantic(int argc, char* argv[]) {
    const std::string command = argv[0];
    if (argc == 1 || (argc == 2 && (std::string(argv[1]) == "--help" || std::string(argv[1]) == "-h"))) {
        std::ostringstream out;
        out << "Module: Dimensional Input Parameters (DIP)\nCommand: " << command
            << "\n\nUsage:\n  snt dip " << command << " (--project DIPfile | --input FILE)";
        if (command == "describe" || command == "override-contract")
            out << " --path PATH";
        out << " [options]\n\nDescription:\n";
        if (command == "describe")
            out << "Describe one evaluated node, including values, metadata, and provenance.\n";
        else if (command == "override-contract")
            out << "Show the target kind and constraints for a proposed override.\n";
        else if (command == "schemas")
            out << "List schema definitions, applications, and value associations.\n";
        else if (command == "sources")
            out << "List the project's registered sources.\n";
        else if (command == "list")
            out << "Select evaluated nodes by path query and tags.\n";
        else if (command == "records")
            out << "Query scalar fields of keyed collection items. Equality filters accept strings.\n";
        else
            out << "Validate proposed overrides and report their effective-value changes.\n";
        out << "\nOptions:\n"
                     "  -h, --help              Show this help.\n"
                     "  --project DIPfile        Read a DIPfile project\n"
                     "  --input FILE             Read a DIPfile, .dip, or .dipl file";
        if (command == "describe" || command == "list" || command == "schemas" || command == "override-contract")
            out << ", or a .diph5 snapshot";
        out << '\n';
        if (command == "describe")
            out << "  --path PATH              Evaluated node to inspect\n"
                         "  --max-value-elements N   Include values up to N elements (default 16)\n";
        if (command == "override-contract")
            out << "  --path PATH              Override target to inspect\n";
        if (command == "list")
            out << "  --query QUERY            Path selection (default ?)\n"
                         "  --tag-all TAG            Require every specified tag; repeatable\n"
                         "  --tag-any TAG            Require any specified tag; repeatable\n"
                         "  --tag-none TAG           Exclude specified tags; repeatable\n"
                         "  --limit N                Maximum results (default 100)\n"
                         "  --max-value-elements N   Include values up to N elements (default 0)\n";
        if (command == "records")
            out << "  --collection PATH        Keyed map collection to query\n"
                   "  --where-eq FIELD=VALUE   Match a scalar string field; repeatable\n"
                   "  --field FIELD            Include this relative field; repeatable\n"
                   "  --offset N               Skip N matching records (default 0)\n"
                   "  --limit N                Maximum records (default 100)\n";
        if (command == "preview")
            out << "  --override TEXT          Add a DIPL override body; repeatable\n"
                         "  --override-file FILE     Add an override file; repeatable\n"
                         "  --max-details N          Maximum reported differences (default 50)\n";
        out << "  --record-graph           Record dependencies while parsing DIPL input\n"
                     "  --format json            Output JSON (the default and only format)\n";
        std::cout << snt::cli::format_help(out.str());
        return 0;
    }
    std::filesystem::path input;
    std::string path;
    std::string query = "?";
    snt::dip::TagFilter tags;
    std::vector<snt::dip::PreviewOverride> overrides;
    std::size_t limit = 100;
    std::size_t max_details = 50;
    std::size_t max_value_elements = command == "describe" ? 16 : 0;
    bool record_graph = false;
    bool has_path = false;
    bool has_list_options = false;
    bool has_max_details = false;
    bool has_value_limit = false;
    snt::dip::RecordQuery records;
    bool has_record_options = false;
    for (int i = 1; i < argc; ++i) {
        const std::string option = argv[i];
        if (option == "--record-graph") {
            record_graph = true;
            continue;
        }
        if (++i >= argc)
            throw std::invalid_argument(option + " requires a value.");
        const std::string value = argv[i];
        if (option == "--project" || option == "--input") {
            if (!input.empty())
                throw std::invalid_argument("Specify one input.");
            input = value;
        } else if (option == "--path") {
            path = value;
            has_path = true;
        } else if (option == "--query") {
            query = value;
            has_list_options = true;
        } else if (option == "--collection") {
            records.collection = value;
            has_record_options = true;
        } else if (option == "--field") {
            records.fields.push_back(value);
            has_record_options = true;
        } else if (option == "--where-eq") {
            const auto equal = value.find('=');
            if (equal == std::string::npos || equal == 0)
                throw std::invalid_argument("--where-eq requires FIELD=VALUE.");
            records.filters.push_back({value.substr(0, equal), value.substr(equal + 1)});
            has_record_options = true;
        } else if (option == "--offset") {
            records.offset = count(value, option);
            has_record_options = true;
        } else if (option == "--tag-all") {
            tags.all.push_back(value);
            has_list_options = true;
        } else if (option == "--tag-any") {
            tags.any.push_back(value);
            has_list_options = true;
        } else if (option == "--tag-none") {
            tags.none.push_back(value);
            has_list_options = true;
        } else if (option == "--limit") {
            limit = count(value, option);
            if (command == "records") {
                records.limit = limit;
                has_record_options = true;
            } else
                has_list_options = true;
        } else if (option == "--max-value-elements") {
            max_value_elements = count(value, option);
            has_value_limit = true;
        } else if (option == "--max-details") {
            max_details = count(value, option);
            has_max_details = true;
        } else if (option == "--override")
            overrides.push_back({snt::dip::PreviewOverride::Kind::Text, value});
        else if (option == "--override-file")
            overrides.push_back({snt::dip::PreviewOverride::Kind::File, value});
        else if (option == "--format") {
            if (value != "json")
                throw std::invalid_argument("Only --format json is supported.");
        } else
            throw std::invalid_argument("Unknown option: " + option);
    }
    if (input.empty())
        throw std::invalid_argument("Specify --project or --input.");
    if ((command == "describe" || command == "override-contract") && path.empty())
        throw std::invalid_argument(command + " requires --path.");
    if (command != "preview" && !overrides.empty())
        throw std::invalid_argument("Overrides require preview.");
    if (has_path && command != "describe" && command != "override-contract")
        throw std::invalid_argument("--path requires describe or override-contract.");
    if (has_list_options && command != "list")
        throw std::invalid_argument("List filters require list.");
    if (has_record_options && command != "records")
        throw std::invalid_argument("Record options require records.");
    if (command == "records" && records.collection.empty())
        throw std::invalid_argument("Records requires --collection.");
    if (has_max_details && command != "preview")
        throw std::invalid_argument("--max-details requires preview.");
    if (has_value_limit && command != "describe" && command != "list")
        throw std::invalid_argument("--max-value-elements requires describe or list.");
    const snt::api::DIPSemantic semantic(input, record_graph);
    if (command == "describe")
        std::cout << semantic.describe_json(path, max_value_elements) << '\n';
    else if (command == "override-contract")
        std::cout << semantic.override_contract_json(path) << '\n';
    else if (command == "schemas")
        std::cout << semantic.schemas_json() << '\n';
    else if (command == "sources")
        std::cout << semantic.sources_json() << '\n';
    else if (command == "list")
        std::cout << semantic.list_json(query, tags, limit, max_value_elements) << '\n';
    else if (command == "records")
        std::cout << semantic.records_json(records) << '\n';
    else
        std::cout << semantic.preview_json(overrides, max_details) << '\n';
    return 0;
}
