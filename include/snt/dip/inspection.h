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
 * Recording is opt-in for parsed sources; snapshots retain their saved graph state
 * regardless of record_dependency_graph.
 */
Environment open_artifact(const std::filesystem::path& path, bool record_dependency_graph = false);

/** Replace the current environment only after the new artifact loads successfully.
 * Pass record_dependency_graph again when refreshing parsed source with a graph.
 */
void reload_artifact(Environment& current, const std::filesystem::path& path,
                     bool record_dependency_graph = false);

/** A live DIP entity whose source can be inspected. For Path, source_name
 * selects a named source; an empty source_name selects the evaluated project.
 * ProjectEntry uses index instead of name.
 */
enum class SourceEntityKind { Path, NamedSource, Schema, Unit, ProjectEntry };

struct SourceEntity {
    SourceEntityKind kind;
    std::string name;
    std::string source_name;
    std::size_t index = 0;
};

enum class SourceLocationRole { Source, Declaration, Modification, Override, Definition, Registration };

/** A semantic source location. When embedded text has a host registration,
 * source and line identify that physical location while logical_source_name
 * and logical_line retain the original declaration. All lines are one-based.
 */
struct InspectedSourceLocation {
    SourceLocationRole role;
    SourceInfo source;
    std::size_t line = 0;
    std::string logical_source_name;
    std::size_t logical_line = 0;
    std::size_t modification_index = 0;
    bool embedded_registration = false;
    bool source_text_available = false; ///< Parsed text is retained in this environment.
};

/** Return semantic source locations in precedence order. Source text may be
 * unavailable, especially after DIPH5 loading. Callers decide whether and how
 * to open physical paths and whether equivalent targets should be collapsed.
 */
std::vector<InspectedSourceLocation> inspect_source_locations(
    const Environment& env, const SourceEntity& entity);

/** Owned, read-only facts about one evaluated value. */
enum class ValueChangeKind { Declaration, Modification, Override };

struct ValueChange {
    ValueChangeKind kind;
    core::SourceLocation location;
};

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
    std::string table_path; ///< Empty unless the value is a table column.
    std::vector<ValueChange> changes; ///< Applied changes in evaluation order.
};

ValueInspection inspect_value(const Environment& env, std::string_view path);

/** Values in environment order, each retaining its fully qualified path. */
std::vector<ValueInspection> inspect_values(const Environment& env);

/** Supported inspection operations and retained facts at a DIP path. */
struct InspectionCapabilities {
    bool hasValue = false;
    bool hasChildren = false;
    bool hasSource = false;
    bool hasProvenance = false;
    bool hasTabularData = false;
    bool hasArrayData = false;
    bool hasReferenceGraph = false;
    bool sourceEditable = false;
    bool directlyWritable = false;
};

/** Works for evaluated value paths, groups, and collection paths. */
InspectionCapabilities inspect_capabilities(const Environment& env, std::string_view path);

/** Metadata for a table column; values remain in the environment. */
struct TableColumnInspection {
    size_t index = 0;
    std::string name;
    std::string path;
    core::DataType type;
    std::optional<puq::Quantity> units;
    ValueMetadata metadata;
};

/** A read-only view of an evaluated DIPL table. */
struct TableInspection {
    std::string path;
    size_t rows = 0;
    std::vector<TableColumnInspection> columns;
};

TableInspection inspect_table(const Environment& env, std::string_view path);
std::vector<TableInspection> inspect_tables(const Environment& env);

/** Read an in-memory value slice without first cloning the whole array.
 * Ranges are zero-based and inclusive. This does not perform lazy DIPH5 I/O.
 */
val::BaseValue::PointerType read_value_slice(
    const Environment& env, std::string_view path, const val::Array::RangeType& ranges);

} // namespace snt::dip

#endif
