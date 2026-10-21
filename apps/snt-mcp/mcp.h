/** @file
 * @brief MCP tool catalogue, request dispatch, and transports.
 */
#ifndef SNT_APP_MCP_H
#define SNT_APP_MCP_H
#include "execution.h"

#include <filesystem>
#include <functional>
#include <map>
#include <nlohmann/json.hpp>
#include <optional>
#include <string>
#include <vector>

namespace snt::mcp {
    using Json = nlohmann::json; ///< JSON values shared by MCP requests and structured results.
    using Projects = std::map<std::string, std::filesystem::path>; ///< Named source projects or saved DIPH5 inputs.
    constexpr std::size_t message_limit = 4 * 1024 * 1024;         ///< Maximum encoded protocol message size in bytes.
    constexpr const char* modern_version =
        "2026-07-28"; ///< Protocol revision using discovery and per-request metadata.
    constexpr const char* legacy_version = "2025-11-25"; ///< Preferred revision for initialization-based clients.

    /** A discoverable tool contract and its native API callback. */
    struct Tool {
        Json definition;                         ///< Tool name, descriptions, schemas, and annotations.
        std::function<Json(const Json&)> invoke; ///< Native operation returning structured tool data.
    };

    /** Build the tool catalogue and dispatch validated arguments to existing SNT APIs. */
    class Tools {
      public:
        /** Construct a catalogue; storage is required when invoking artifact-producing tools. */
        explicit Tools(Projects projects, Storage* storage = nullptr);
        /** Return MCP tool definitions, including input and output JSON schemas. */
        Json list() const;
        /** Describe configured inputs and their supported operations. */
        Json environments() const;
        /** Validate arguments and invoke a registered tool; exceptions become errors at the execution boundary. */
        Json call(const std::string& name, const Json& arguments) const;
        /** Return whether a tool name is registered in this build. */
        bool contains(const std::string& name) const;

      private:
        Projects projects_;                 ///< Configured names and filesystem inputs.
        Storage* storage_;                  ///< Borrowed worker artifact store; null for catalogue-only use.
        std::map<std::string, Tool> tools_; ///< Tool definitions indexed by public name.
        /** Resolve a configured environment name or an immutable snapshot artifact identifier. */
        std::filesystem::path project(const Json& arguments, const std::string& key = "environment") const;
        /** Register a callback with its argument descriptions and output schema. */
        void add(
            std::string name,
            std::string description,
            Json properties,
            Json required,
            std::function<Json(const Json&)> invoke
        );
    };

    /** Transport-neutral JSON-RPC response; notifications have no body. */
    struct Reply {
        std::optional<Json> body; ///< Response body, absent for notifications or cancelled requests.
        int status = 200;         ///< HTTP status; ignored by stdio.
    };

    /** Dispatch MCP requests and supervise tool execution without retaining evaluated core objects. */
    class Protocol {
      public:
        /** Configure named inputs and the maximum wall time in milliseconds for each tool call. */
        explicit Protocol(Projects projects, unsigned timeout_ms = 30000);
        /** Handle one JSON-RPC message. HTTP supplies routing headers and a disconnect probe. */
        Reply handle(
            const std::string& body,
            bool http = false,
            const std::map<std::string, std::string>& headers = {},
            const std::function<bool()>& disconnected = {}
        );
        /** Construct a JSON-RPC error response with the supplied request identifier. */
        static Json error(const Json& id, int code, const std::string& message);
        /** Report whether a tool worker currently owns the execution slot. */
        bool busy() const { return execution_.busy(); }

      private:
        Tools tools_;                          ///< Parent-side catalogue used for discovery and validation.
        Execution execution_;                  ///< Worker supervisor and session artifact owner.
        std::atomic<bool> initialized_{false}; ///< Legacy stdio handshake state shared with the request thread.
    };

    /** Serve newline-delimited JSON-RPC while accepting cancellation notifications during tool execution. */
    int run_stdio(Protocol& protocol);
    /** Serve POST /mcp with cpp-httplib; reject browser origins outside the supplied allowlist. */
    int run_http(
        Protocol& protocol, const std::string& address, int port, const std::vector<std::string>& allowed_origins
    );
} // namespace snt::mcp
#endif
