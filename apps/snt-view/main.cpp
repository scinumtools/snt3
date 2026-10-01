#include "application.h"
#include "viewer_model.h"

#include <filesystem>
#include <iostream>
#include <string_view>

int module_view(int argc, char* argv[]) {
    if (argc == 2 && (std::string_view(argv[1]) == "--help" || std::string_view(argv[1]) == "-h")) {
        std::cout << R"(Scientific Numerical Tools v3 (SNT)
Module: Parameter viewer

Usage:
  snt view <artifact>

Open a read-only DIP parameter browser. Supported artifacts are .dip,
.dipl, DIPfile, and .diph5. Standalone .dipt files are not yet supported.
Press Ctrl+R or use File > Reload after editing a source externally.
)";
        return 0;
    }
    if (argc != 2 || std::string_view(argv[1]).empty()) {
        std::cerr << "Usage: snt view <artifact>\n";
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
