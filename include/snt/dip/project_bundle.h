#ifndef SNT_DIP_PROJECT_BUNDLE_H
#define SNT_DIP_PROJECT_BUNDLE_H

#include <filesystem>
#include <functional>
#include <string>
#include <vector>

namespace snt::dip {

/** Text files needed to evaluate one DIPfile without external I/O. Paths are
 * relative to the directory containing the DIPfile and use forward slashes.
 */
struct ProjectBundleFile {
    std::string path;
    std::string text;
};

struct ProjectBundle {
    std::string entry = "DIPfile";
    std::vector<ProjectBundleFile> files;
};

using ProjectFileReader = std::function<std::string(const std::filesystem::path&)>;

/** Read a local text file, throwing an IOException if it is unavailable. */
std::string read_project_file(const std::filesystem::path& path);

/** Collect every local file opened while evaluating a DIPfile, plus optional
 * files for source paths selected by later overrides. Files outside the
 * DIPfile directory, including symlinks escaping it, are rejected.
 */
ProjectBundle package_project(const std::filesystem::path& project_file,
                              const std::vector<std::filesystem::path>& additional_files = {});

/** Stable JSON representation accepted by the browser parseBundle API. */
std::string project_bundle_json(const ProjectBundle& bundle);

} // namespace snt::dip

#endif
