#include "main.h"

#include <snt/dip/project_bundle.h>

#include <fstream>
#include <filesystem>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

int module_dip_package(int argc, char* argv[]) {
    if (argc == 2 && std::string(argv[1]) == "--help") {
        std::cout << "Usage: snt dip package --project DIPfile [--output project.bundle.json] [--include FILE ...]\n"
                     "Collect local text files needed to evaluate the project in a browser.\n"
                     "--include adds a file under the DIPfile directory for alternate source choices.\n"
                     "Without --output, write the JSON bundle to stdout.\n";
        return 0;
    }
    std::string project;
    std::string output;
    std::vector<std::filesystem::path> additional_files;
    for (int i = 1; i < argc; ++i) {
        const std::string option = argv[i];
        if (option != "--project" && option != "--output" && option != "--include")
            throw std::invalid_argument("Unknown option: " + option);
        if (++i >= argc) throw std::invalid_argument(option + " requires a value.");
        if (option == "--project") {
            if (!project.empty()) throw std::invalid_argument("Specify one --project.");
            project = argv[i];
        } else if (option == "--include") {
            additional_files.emplace_back(argv[i]);
        } else {
            if (!output.empty()) throw std::invalid_argument("Specify one --output.");
            output = argv[i];
        }
    }
    if (project.empty()) throw std::invalid_argument("Specify --project DIPfile.");
    const std::string json = snt::dip::project_bundle_json(snt::dip::package_project(project, additional_files));
    if (output.empty()) std::cout << json << '\n';
    else {
        std::ofstream file(output, std::ios::binary);
        if (!file) throw std::runtime_error("Could not open bundle output: " + output);
        file << json << '\n';
        if (!file) throw std::runtime_error("Could not write bundle output: " + output);
    }
    return 0;
}
