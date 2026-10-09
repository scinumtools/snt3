#ifndef SNT_DIP_ARTIFACT_H
#define SNT_DIP_ARTIFACT_H

#include <snt/dip/environment.h>

#include <filesystem>

namespace snt::dip {

/** Classification by conventional filename; it does not validate file contents. */
enum class ArtifactKind { Unknown, Project, DIPL, TableText, DIPH5 };

ArtifactKind detect_artifact(const std::filesystem::path& path);

/** Load a project, DIPL file, or DIPH5 snapshot into a new environment.
 * .dipt is table input within DIPL, not a standalone environment.
 * Recording is opt-in for parsed sources; snapshots retain their saved graph state
 * regardless of record_dependency_graph. Parsed string blocks are retained only
 * when retain_block_inputs is true for a live parse.
 */
Environment open_artifact(const std::filesystem::path& path, bool record_dependency_graph = false,
                          bool retain_block_inputs = false);

/** Replace the current environment only after the new artifact loads successfully.
 * Pass recording options again when refreshing a parsed source.
 */
void reload_artifact(Environment& current, const std::filesystem::path& path,
                     bool record_dependency_graph = false, bool retain_block_inputs = false);

} // namespace snt::dip

#endif
