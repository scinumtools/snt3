#ifndef SNT_DIP_INSPECTION_H
#define SNT_DIP_INSPECTION_H

#include <snt/core/exceptions.h>
#include <snt/dip/cursor.h>
#include <snt/dip/environment.h>

#include <filesystem>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace snt::dip {

/** Classification by conventional filename; it does not validate file contents. */
enum class ArtifactKind { Unknown, Project, DIPL, TableText, DIPH5 };

ArtifactKind detect_artifact(const std::filesystem::path& path);

/** Load a project, DIPL file, or DIPH5 snapshot into a new environment.
 * .dipt is table input within DIPL, not a standalone environment.
 */
Environment open_artifact(const std::filesystem::path& path);

/** Replace the current environment only after the new artifact loads successfully. */
void reload_artifact(Environment& current, const std::filesystem::path& path);

/** Owned, read-only facts about one evaluated value. */
struct ValueInspection {
    std::string path;
    core::DataType type;
    val::Array::ShapeType shape;
    val::BaseValue::PointerType value;
    std::optional<puq::Quantity> units;
    ValueMetadata metadata;
    std::vector<std::string> tags;
    Provenance provenance;
    core::SourceLocation declaration_location;
    std::optional<core::SourceLocation> override_location;
    std::vector<SchemaInfo> applied_schemas;
    std::optional<SchemaInfo> contributing_schema;
};

ValueInspection inspect_value(const Environment& env, std::string_view path);

/** Values in environment order, each retaining its fully qualified path. */
std::vector<ValueInspection> inspect_values(const Environment& env);

/** Read an in-memory value slice without first cloning the whole array.
 * Ranges are zero-based and inclusive. This does not perform lazy DIPH5 I/O.
 */
val::BaseValue::PointerType read_value_slice(
    const Environment& env, std::string_view path, const val::Array::RangeType& ranges);

} // namespace snt::dip

#endif
