#ifndef SNT_DIP_INSPECTION_H
#define SNT_DIP_INSPECTION_H

#include <snt/core/exceptions.h>
#include <snt/dip/cursor.h>
#include <snt/dip/environment.h>

#include <map>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace snt::dip {

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

/** Value facts without copying its potentially large array. Accepts an evaluated
 * path or a source-qualified source?path ID. */
struct ValueSummary {
    std::string path;
    core::DataType type;
    val::Array::ShapeType shape;
    std::size_t elements = 0;
    std::optional<puq::Quantity> units;
    ValueMetadata metadata;
    std::string table_path;
};

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

/** What an override may target in this evaluated model. This is a static
 * guide, not validation of a proposed value or a substitute for preview.
 */
enum class OverrideTargetKind { Unavailable, ExistingValue, ExistingItem, NewItem };

struct OverrideContract {
    std::string path;
    OverrideTargetKind kind = OverrideTargetKind::Unavailable;
    std::string reason; ///< Stable reason when unavailable.
    std::string resolved_path; ///< New list items receive the next index at this moment.
    core::DataType declared_type = core::DataType::None;
    std::optional<val::Array::ShapeType> current_shape; ///< Evaluated shape, not declared bounds.
    std::optional<std::string> units;
    std::vector<std::string> enforced_options;
    std::string enforced_condition;
    std::vector<std::string> item_schemas;
    bool snapshot_input = false; ///< A snapshot has no parser input to preview directly.
};

/** One distinct value or source adjacent to a selected dependency-graph ID.
 * request and operand describe the first effective read connecting the pair.
 */
struct DependencyNeighbor {
    std::string id;
    std::string request;
    std::string operand;
};

/** Local effective dependency graph for a value or source ID. Dependencies
 * combine value and condition reads in that order, once per target. Readers
 * are values whose latest evaluation reads the selected ID. The latest value
 * and condition events remain separate so clients can inspect both expressions.
 */
struct DependencyNeighborhood {
    std::string id;
    bool recorded = false;
    std::vector<DependencyNeighbor> dependencies;
    std::vector<DependencyNeighbor> readers;
    std::optional<DependencyEvent> value_event;
    std::optional<DependencyEvent> condition_event;
};

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

} // namespace snt::dip

#endif
