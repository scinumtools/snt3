#ifndef SNT_DIP_INSPECTOR_H
#define SNT_DIP_INSPECTOR_H

#include <snt/dip/inspect/inspection.h>
#include <snt/dip/inspect/semantic.h>

namespace snt::dip {

/** Read-only queries over one evaluated environment. The environment must outlive this view.
 * Value and description results are owned; graph and retained block references belong to the environment.
 */
class Inspector {
    const Environment* env_;

  public:
    explicit Inspector(const Environment& env) : env_(&env) {}
    Inspector(Environment&&) = delete;
    Inspector(const Environment&&) = delete;

    /** Select independent value snapshots in environment order. ? selects all values;
     * ?path selects one, and ?path. selects a subtree. Tag filters match explicit tags.
     */
    ValueNode::ListType select(const std::string& query = "?", const TagFilter& tags = {}) const;
    /** Select matching path names without cloning values or arrays. */
    std::vector<std::string> select_paths(const std::string& query = "?", const TagFilter& tags = {}) const;
    /** Traverse a known path without cloning the underlying value. */
    Cursor cursor(std::string_view path) const { return (*env_)[path]; }

    /** Owned value and its type, shape, units, metadata, provenance, and applied changes. */
    ValueInspection value(std::string_view path) const;
    /** All evaluated values, each with its full path. */
    std::vector<ValueInspection> values() const;
    /** Type, shape, element count, units, and metadata without copying value data. */
    ValueSummary value_summary(std::string_view path) const;
    /** Inclusive zero-based ranges from an evaluated in-memory value; no lazy DIPH5 I/O. */
    val::BaseValue::PointerType value_slice(std::string_view path, const val::Array::RangeType& ranges) const;
    /** Row count and ordered column metadata without copying table cells. */
    TableInspection table(std::string_view path) const;
    /** All evaluated tables in environment order. */
    std::vector<TableInspection> tables() const;
    /** Available facts and operations for a value, group, collection, table, or named-source path. */
    InspectionCapabilities capabilities(std::string_view path) const;
    /** Semantic source locations in precedence order; retained source text may be unavailable. */
    std::vector<InspectedSourceLocation> source_locations(const SourceEntity& entity) const;
    /** Original array/table string blocks retained by an opt-in live parse; empty for snapshots. */
    const std::map<std::string, BlockInput>& block_inputs() const;
    /** A retained block by evaluated path, or nullptr when unavailable. */
    const BlockInput* block_input(std::string_view path) const;

    /** Bounded semantic description of one value, group, collection, or table. */
    SemanticDescription describe(std::string_view path, std::size_t max_value_elements = 16) const;
    /** Bounded descriptions selected by a ? query and tags; includes the total match count. */
    SemanticList list_descriptions(const std::string& query = "?", const TagFilter& tags = {},
                                   std::size_t limit = 100, std::size_t max_value_elements = 0) const;

    /** Recorded evaluation events and effective dependency queries. Empty when capture was disabled. */
    const DependencyGraph& graph() const { return env_->dependency_graph_; }
    /** One-hop inputs and readers of a path, with latest value and condition events. */
    DependencyNeighborhood dependency_neighborhood(std::string_view path) const;
};

} // namespace snt::dip

#endif
