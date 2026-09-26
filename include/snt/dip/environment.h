#ifndef DIP_ENVIRONMENT_H
#define DIP_ENVIRONMENT_H

#include "nodes/node_value.h"

#include <filesystem>
#include <optional>
#include <snt/dip/lists/list_branching.h>
#include <snt/dip/lists/list_functions.h>
#include <snt/dip/lists/list_hierarchy.h>
#include <snt/dip/lists/list_node.h>
#include <snt/dip/lists/list_schema.h>
#include <snt/dip/lists/list_source.h>
#include <snt/dip/lists/list_unit.h>
#include <vector>

namespace snt::dip {

    class Cursor; ///< Forward declaring

    /** Explicit node tag filters, combined with AND; empty fields impose no restriction. */
    struct TagFilter {
        std::vector<std::string> all;
        std::vector<std::string> any;
        std::vector<std::string> none;

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
        bool trace_manifest_loaded_ = false;

      public:
        NodeList<ValueNode> nodes; ///< List of parsed nodes
        HierarchyList hierarchy;   ///< List of node hierarchy (parent nodes)
        BranchingList branching;   ///< List of code branching (case, else)
        SourceList sources;        ///< List of code sources
        UnitList units;            ///< List of custom units
        SchemaList schemas;        ///< List of schemas
        FunctionList functions;    ///< List of functions

        /**
         * Constructor of the Environment class
         */
        Environment();

        /**
         * Load DIP environment from a HDF5 file
         * @param file File name of the environment file
         */
        void load(const std::filesystem::path& file);

        /**
         * Save DIP environment into a HDF5 file
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
            const std::string& request, const RequestType rtype = RequestType::Reference
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
         * Get group of nodes from a reference or a function based on a request expression
         *
         * @param request Request expression
         * @param rtype Request type: reference, or function
         * @param tags List of tags that filter selected set
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
         * including its value-bearing root and collection members. Tags are not inherited.
         * No matches returns an empty list. Source-qualified queries are also supported.
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
         * Get pointer to a node with a specific name
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
