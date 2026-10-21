/** @file
 * @brief Immutable MCP artifacts, paging, and wire contracts.
 */
#ifndef SNT_MCP_STORAGE_H
#define SNT_MCP_STORAGE_H
#include <filesystem>
#include <nlohmann/json.hpp>
#include <stdexcept>

namespace snt::mcp {
    using Json = nlohmann::json; ///< JSON values shared by MCP requests and structured results.
    /** Application-layer failure with a stable machine-readable code. */
    struct ServiceError : std::runtime_error {
        std::string code; ///< Stable error category for client recovery decisions.
        /** Preserve a stable code alongside the human-readable exception message. */
        ServiceError(std::string code, const std::string& message)
            : std::runtime_error(message), code(std::move(code)) {}
    };
    constexpr std::uintmax_t artifact_budget = 256 * 1024 * 1024; ///< Maximum stored artifact bytes per server session.
    constexpr std::size_t chunk_limit = 128 * 1024;               ///< Maximum decoded bytes per artifact chunk.
    /** Read immutable session artifacts and stage worker outputs for publication after success. */
    class Storage {
        std::filesystem::path root_, staging_; ///< Published session root and optional private worker workspace.
      public:
        /** Open an existing session root, optionally with a private worker staging directory. */
        Storage(std::filesystem::path root, std::filesystem::path staging = {})
            : root_(std::move(root)), staging_(std::move(staging)) {}
        /** Return the worker scratch directory; empty for a read-only parent view. */
        const std::filesystem::path& workspace() const { return staging_; }
        /** Copy and hash a generated file into staging; return metadata with a one-hour expiry. */
        Json put_file(
            const std::filesystem::path& file,
            const std::string& kind,
            const std::string& mime,
            const std::string& filename
        ) const;
        /** Serialize structured data into a staged JSON artifact. */
        Json put_json(const Json& value, const std::string& kind) const;
        /** Validate an artifact identifier and return its metadata; reject missing or expired entries. */
        Json metadata(const std::string& id) const;
        /** Resolve a validated artifact to its data file, preserving the DIPH5 extension for snapshots. */
        std::filesystem::path path(const std::string& id) const;
        /** Return a bounded base64 chunk and the next byte offset, or null at end of file. */
        Json read(const std::string& id, std::size_t offset, std::size_t limit) const;
        /** Page a parameter listing, freezing it into an artifact when continuation is needed. */
        Json page(Json value, std::size_t limit, const std::string& cursor) const;
        /** Remove expired artifacts from the published store before a new tool executes. */
        void prune() const;
        /** Move successful worker artifacts into the session store after checking its byte budget. */
        void publish() const;
    };
    /** Convert an exception into MCP error content and a structured diagnostic. */
    Json failure_result(const std::exception& error);
    /** Return the tool output schema, including structured errors and large-result references. */
    Json output_schema(const std::string& name);
    /** Attach semantic descriptions to the common tool argument schemas. */
    void describe_arguments(Json& properties);
    /** Return the built-in DIPL and workflow guide exposed as an MCP resource. */
    const std::string& dipl_help();
} // namespace snt::mcp
#endif
