#ifndef DIP_ENVIRONMENT_H
#define DIP_ENVIRONMENT_H

#include "nodes/node_value.h"
#include <snt/dip/dependency_graph.h>

#include <filesystem>
#include <optional>
#include <snt/dip/lists/list_branching.h>
#include <snt/dip/lists/list_functions.h>
#include <snt/dip/lists/list_hierarchy.h>
#include <snt/dip/lists/list_node.h>
#include <snt/dip/lists/list_override.h>
#include <snt/dip/lists/list_schema.h>
#include <snt/dip/lists/list_source.h>
#include <snt/dip/lists/list_unit.h>
#include <vector>

namespace snt::dip {

    class Cursor; ///< Forward declaring

    /** Match explicitly assigned node tags; fields combine with AND. */
    struct TagFilter {
        std::vector<std::string> all;  ///< Require every listed tag.
        std::vector<std::string> any;  ///< Require at least one listed tag.
        std::vector<std::string> none; ///< Reject nodes with any listed tag.

        /** Empty filter fields impose no restriction. */
        bool matches(const std::vector<std::string>& tags) const;
    };

    /**
     * Durable identity information for one DIPL source.
     *
     * The digest covers the exact UTF-8 bytes parsed by DIP. It is a
     * fingerprint, not embedded source content.
     */
    struct SourceInfo {
        std::string name;
        std::string path;
        std::string parent_name;
        size_t parent_line = 0;
        std::string hash_algorithm;
        std::string hash;
    };

    /** Durable identity information for one registered DIPL construct. */
    struct TraceInfo {
        std::string id;   ///< Internal trace identifier, e.g. DIP0_UNIT0.
        std::string name; ///< Registered public name.
        std::string kind; ///< "unit", "schema", "function_value", or "function_nodes".
    };

    /** Descriptive schema provenance; contains no reusable schema nodes. */
    struct SchemaInfo {
        std::string id;
        std::string name;
        std::string source_name;
        size_t source_line = 0;
        ValueMetadata metadata;
        std::optional<SourceInfo> source; ///< Source identity and fingerprint when available.
    };

    /**
     * Type of an environment request
     */
    enum class RequestType {
        Reference, ///< Search in the node list
        Function   ///< Search in the function list
    };

    /**
     * List of available DIP generators that produce static parameter lists
     */
    enum class ExportFormat {
        CPP,
        C,
        FORTRAN,
        RUST,
        R,
        JULIA,
        JSON,
        TOML,
        YAML,
    };

    /**
     * Object of this class holds the whole DIP parsing environment
     */
    class Environment {
      private:
        std::vector<SourceInfo> source_manifest_;
        std::vector<TraceInfo> trace_manifest_;
        std::vector<SchemaInfo> schema_manifest_;
        bool schema_manifest_loaded_ = false;
        bool trace_manifest_loaded_ = false;
        bool snapshot_loaded_ = false;
        bool dependency_recording_ = false;
        mutable DependencyGraph dependency_graph_;
        mutable std::optional<size_t> active_dependency_event_;

        void record_dependency(const std::string& target, const std::string& request,
                               std::string_view operand = {}) const;

      public:
        NodeList<ValueNode> nodes; ///< List of parsed nodes
        HierarchyList hierarchy;   ///< List of node hierarchy (parent nodes)
        BranchingList branching;   ///< List of code branching (case, else)
        SourceList sources;        ///< List of code sources
        UnitList units;            ///< List of custom units
        SchemaList schemas;        ///< Reusable definitions; not restored from DIPH5.
        OverrideList overrides;    ///< Collected value modifications for initial evaluation.
        FunctionList functions;    ///< List of functions

        /**
         * Constructor of the Environment class
         */
        Environment();

        /** Graph captured during DIP evaluation. Loaded snapshots may have no graph. */
        const DependencyGraph& dependency_graph() const { return dependency_graph_; }
        /** Enable graph capture for this parse. */
        void set_dependency_recording(bool enabled) {
            dependency_recording_ = enabled;
            dependency_graph_ = DependencyGraph{};
            dependency_graph_.recorded = enabled;
            active_dependency_event_.reset();
        }
        /** Restore a graph from a versioned snapshot. */
        void set_dependency_graph(DependencyGraph graph) {
            dependency_recording_ = false;
            dependency_graph_ = std::move(graph);
            active_dependency_event_.reset();
        }

        /** Scope one parser evaluation. Restores the previous context on exit. */
        class DependencyScope {
            Environment& env_;
            std::optional<size_t> previous_;
          public:
            DependencyScope(Environment& env, std::string owner, DependencyEventKind kind,
                            std::optional<core::SourceLocation> location = std::nullopt);
            ~DependencyScope();
            DependencyScope(const DependencyScope&) = delete;
            DependencyScope& operator=(const DependencyScope&) = delete;
        };

        /** Attach the operation tree produced by the numerical or logical solver. */
        exs::CompositionGraph* active_composition(const std::string& expression);
        void set_active_dependency_owner(std::string owner);
        void set_value_controls(const std::string& owner, const std::vector<size_t>& case_ids);
        void record_import_origin(const std::string& source_node_id);

        /** Whether this environment was loaded from a DIPH5 snapshot. */
        bool is_loaded_snapshot() const { return snapshot_loaded_; }

        /**
         * Load evaluated DIP nodes from a DIPH5 file. Reusable schema definitions
         * are not reconstructed; descriptive schema provenance is restored.
         * @param file File name of the environment file
         */
        void load(const std::filesystem::path& file);

        /**
         * Save evaluated DIP nodes to a DIPH5 file. The schema trace manifest is
         * saved with schema descriptions and source provenance, but reusable definitions are not.
         * @param file File name of the environment file
         */
        void save(const std::filesystem::path& file) const;

        /**
         * Generate static parameter lists from the environment nodes
         * @param format Output format of a generated parameter list
         * @param file File name of the generated parameter list
         */
        void generate(ExportFormat format, const std::filesystem::path& file) const;

        /**
         * Return durable source identities for the parsed or loaded environment.
         *
         * Parsed environments derive the manifest from their registered sources;
         * loaded DIPH5 environments return the manifest stored in the file.
         */
        std::vector<SourceInfo> get_source_manifest() const;

        /** Return the durable identity of a named DIPL source, when available. */
        std::optional<SourceInfo> get_source_info(const std::string& name) const;

        /**
         * Replace persisted source-manifest information during environment loading.
         * This does not recreate executable source definitions or source code.
         */
        void set_source_manifest(std::vector<SourceInfo> manifest);

        /** Return durable identities for registered units, schemas, and functions. */
        std::vector<TraceInfo> get_trace_manifest() const;

        /** Replace persisted trace-registry information during environment loading. */
        void set_trace_manifest(std::vector<TraceInfo> manifest);

        /** Return schema descriptions and source provenance without executable definitions. */
        std::vector<SchemaInfo> get_schema_manifest() const;

        /** Replace persisted schema provenance during DIPH5 loading. */
        void set_schema_manifest(std::vector<SchemaInfo> manifest);

        /** Return schemas applied along a value or collection path, in path order. */
        std::vector<SchemaInfo> get_applied_schemas(const std::string& path) const;

        /** Return the schema that supplied a value node, if known. */
        std::optional<SchemaInfo> get_contributing_schema(const std::string& path) const;

        /**
         * Get a source code
         *
         * @param source_name Name of a source
         * @return Source code
         */
        std::string request_code(const std::string& source_name) const;

        /**
         * Get node data (value + units) from a reference or a function based on a request expression
         *
         * @param request Request expression
         * @param rtype Request type: reference, or function
         * @return Selected value node data
         */
        ValueNodeData request_node_data(
            const std::string& request, const RequestType rtype = RequestType::Reference,
            std::string_view operand = {}
        ) const;

        /**
         * Get value from a reference or a function based on a request expression
         *
         * @param request Request expression
         * @param rtype Request type: reference, or function
         * @param to_unit Request values with a specific unit
         * @return Selected ArrayValue object (in specified units)
         */
        val::BaseValue::PointerType request_value(
            const std::string& request,
            const RequestType rtype = RequestType::Reference,
            const std::optional<std::string_view> to_unit = std::nullopt
        ) const;

        /**
         * Request nodes from a reference or function. Reference results are snapshots
         * with paths relative to the requested root. Tag filtering uses any-match
         * semantics; an empty tag list imposes no restriction. Empty results throw.
         *
         * @param request Request expression
         * @param rtype Request type: reference, or function
         * @param tags Tags of which at least one must occur on each returned node
         * @return Group of selected nodes
         */
        ValueNode::ListType request_group(
            const std::string& request,
            const RequestType rtype = RequestType::Reference,
            const std::vector<std::string>& tags = {}
        ) const;

        /**
         * Select independent snapshots with original fully qualified paths in environment order.
         * `?` selects all values, `?path` an exact value, and `?path.` a subtree,
         * including its value-bearing root and collection members. Each path is
         * returned once. Filters combine with AND and match explicit, non-inherited
         * tags. Selection does not modify the environment. No matches returns an
         * empty list. Source-qualified queries are also supported.
         */
        ValueNode::ListType select(const std::string& request = "?", const TagFilter& tags = {}) const;

        /**
         * Get a keyed collection of  nodes from a reference or a function based on a request expression
         *
         * @param request Request expression
         * @param rtype Request type: reference, or function
         * @param tags List of tags that filter selected set
         * @return Keyed collection of nodes
         */
        std::unordered_map<std::string, ValueNode::ListType> request_map(
            const std::string& request,
            const RequestType rtype = RequestType::Reference,
            const std::vector<std::string>& tags = {}
        ) const;

        /**
         * Get a indexed collection of nodes from a reference or a function based on a request expression
         *
         * @param request Request expression
         * @param rtype Request type: reference, or function
         * @param tags List of tags that filter selected set
         * @return Indexed collection of nodes
         */
        std::vector<ValueNode::ListType> request_list(
            const std::string& request,
            const RequestType rtype = RequestType::Reference,
            const std::vector<std::string>& tags = {}
        ) const;

        /**
         * Get parsed node value at the specific index
         *
         * @param index Index of a node
         * @return ArrayValue of a selected node
         */
        val::BaseValue::PointerType get_value(size_t index) const;

        /**
         * Get an environment-owned pointer to a node with a specific name.
         * The pointer refers to the stored node, unlike the snapshots from select().
         *
         * @param path Path name of a searched node
         * @return Pointer to a selected node
         */
        ValueNode::PointerType get_node(const std::string& path) const;

        /**
         * Represent environment as a string
         *
         * @return String representation of an environment
         */
        const std::string to_string() const;

        /**
         * Get cursor from a fully qualified path
         *
         * @param path Relative or fully qualified DIPL path.
         * @return Cursor at the given path
         */
        Cursor operator[](std::string_view path) const;
    };

} // namespace snt::dip

#endif
