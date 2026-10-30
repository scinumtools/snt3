#include "application.h"
#include "viewer_model.h"
#include "../snt/help.h"

#include <filesystem>
#include <iostream>
#include <string_view>

namespace {
    void print_help() {
        std::cout << snt::cli::format_help(R"(Scientific Numerical Tools v3 (SNT)
Module: Parameter viewer

Usage:
  snt view <artifact>

Description:
  Open the graphical DIP parameter browser and editor. Browse evaluated values,
  sources, dependencies, and provenance. DIPfile projects and standalone DIPL
  files support source editing. Projects also support guided setup, Save As,
  and DIPH5 export. A standalone source can be converted into a project from
  the File menu.

Arguments:
  <artifact>    Path to a DIPfile project, standalone .dip or .dipl source,
                or evaluated .diph5 snapshot. DIPH5 snapshots are read-only.
                Standalone .dipt is not supported.

Options:
  -h, --help    Show this help.

Examples:
  snt view examples/dip/ParameterViewer/DIPfile
  snt view ./DIPfile
  snt view results.diph5
)");
    }
} // namespace

int module_view(int argc, char* argv[]) {
    if (argc == 1 ||
        (argc == 2 && (std::string_view(argv[1]) == "--help" || std::string_view(argv[1]) == "-h"))) {
        print_help();
        return 0;
    }
    if (argc != 2 || std::string_view(argv[1]).empty()) {
        print_help();
        return 2;
    }
    try {
        snt::view::ViewerModel model{std::filesystem::path(argv[1])};
        return snt::view::run_application(model);
    } catch (const std::exception& error) {
        std::cerr << "snt view: " << error.what() << '\n';
        return 2;
    }
}
