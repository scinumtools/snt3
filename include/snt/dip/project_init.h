#ifndef SNT_DIP_PROJECT_INIT_H
#define SNT_DIP_PROJECT_INIT_H

#include <filesystem>

namespace snt::dip {

    /** Create a starter DIPfile and main.dip in an existing directory.
     * Existing entries are preserved; a partial source file is removed if
     * creating the DIPfile fails.
     */
    void initialize_project(const std::filesystem::path& directory);

    /** Register an existing standalone .dip/.dipl file in a new sibling DIPfile.
     * The source is left untouched. The new manifest is removed if the resulting
     * project cannot be evaluated, and an existing DIPfile is never replaced.
     */
    std::filesystem::path create_project_from_source(const std::filesystem::path& source_file);

} // namespace snt::dip

#endif
