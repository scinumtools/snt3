#include "../service_support.h"
#include "mcp.h"

#include <algorithm>
#include <iostream>
#include <regex>
#include <stdexcept>

namespace snt::mcp {
    namespace {
        constexpr const char* version_key = "io.modelcontextprotocol/protocolVersion";
        constexpr const char* capabilities_key = "io.modelcontextprotocol/clientCapabilities";
        const Json versions = {modern_version, legacy_version, "2025-06-18"};
        const Json capabilities = {
            {"tools", {{"listChanged", false}}}, {"resources", {{"subscribe", false}, {"listChanged", false}}}
        };
        const Json server_info = {{"name", "snt3"}, {"version", CODE_VERSION}};
        constexpr const char* instructions =
            "Discover named inputs with dipl_environments. Inspect parameters and override contracts before "
            "calling dipl_preview. Check candidate_valid and diagnostics. Calls read current files; previews "
            "do not save changes. Read snt://help/dipl for syntax and workflow examples. Create dipl_snapshot "
            "candidates for reproducible inspection/export/comparison. Follow next_cursor for listings and "
            "artifact_read for downloads or result_ref responses. DIPH5 retains evaluated data, not executable schema "
            "definitions.";

        bool legacy(const std::string& version) {
            return version == legacy_version || version == "2025-06-18";
        }
        std::string header(const std::map<std::string, std::string>& headers, const std::string& name) {
            const auto it = headers.find(name);
            return it == headers.end() ? "" : it->second;
        }
        std::string header_value(const std::string& value) {
            if (std::all_of(value.begin(), value.end(), [](unsigned char c) { return c >= 32 && c <= 126; }) &&
                value.rfind("=?base64?", 0) != 0)
                return value;
            constexpr char alphabet[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
            std::string encoded;
            unsigned bits = 0;
            int available = 0;
            for (unsigned char c : value) {
                bits = (bits << 8) | c;
                available += 8;
                while (available >= 6) {
                    available -= 6;
                    encoded += alphabet[(bits >> available) & 63];
                }
            }
            if (available)
                encoded += alphabet[(bits << (6 - available)) & 63];
            while (encoded.size() % 4)
                encoded += '=';
            return "=?base64?" + encoded + "?=";
        }
    } // namespace

    Protocol::Protocol(Projects projects, unsigned timeout_ms)
        : tools_(projects), execution_(
                                [&projects] {
                                    Json config = Json::object();
                                    for (const auto& [name, path] : projects)
                                        config[name] = path.string();
                                    return config;
                                }(),
                                timeout_ms
                            ) {}

    Json Protocol::error(const Json& id, int code, const std::string& message) {
        return {{"jsonrpc", "2.0"}, {"id", id}, {"error", {{"code", code}, {"message", message}}}};
    }

    Reply Protocol::handle(
        const std::string& body,
        bool http,
        const std::map<std::string, std::string>& headers,
        const std::function<bool()>& disconnected
    ) {
        Json request, id = nullptr;
        auto failure = [&](int code, const std::string& message, int status = 400) -> Reply {
            return {error(id, code, message), status};
        };
        try {
            if (body.size() > message_limit)
                return failure(-32600, "Message exceeds 4 MiB.", 413);
            request = Json::parse(body, [](int depth, Json::parse_event_t, Json&) {
                if (depth > 64)
                    throw std::invalid_argument("JSON nesting exceeds 64 levels.");
                return true;
            });
        } catch (const std::exception&) {
            return failure(-32700, "Invalid JSON or excessive nesting.");
        }
        if (!request.is_object() || request.value("jsonrpc", Json{}) != "2.0" || !request.contains("method") ||
            !request["method"].is_string())
            return failure(-32600, "Expected one JSON-RPC 2.0 request or notification; batches are unsupported.");
        if (request.contains("id")) {
            if (!request["id"].is_string() && !request["id"].is_number_integer())
                return failure(-32600, "Request id must be a string or integer.");
            id = request["id"];
        } else {
            if (!http && request["method"] == "notifications/cancelled" && request.contains("params") &&
                request["params"].is_object() && request["params"].contains("requestId"))
                execution_.cancel(request["params"]["requestId"]);
            return {std::nullopt, 202};
        }
        try {
            const std::string method = request["method"];
            const Json params = request.value("params", Json::object());
            if (!params.is_object())
                return failure(-32602, "params must be an object.");
            const Json meta = params.value("_meta", Json::object());
            if (!meta.is_object())
                return failure(-32602, "_meta must be an object.");
            const auto http_version = header(headers, "mcp-protocol-version");
            const bool modern = meta.contains(version_key) || method == "server/discover" ||
                                (!http_version.empty() && !legacy(http_version));
            if (modern) {
                if (!meta.contains(version_key) || !meta[version_key].is_string() || !meta.contains(capabilities_key) ||
                    !meta[capabilities_key].is_object())
                    return failure(-32602, "_meta requires a protocolVersion string and clientCapabilities object.");
                const std::string version = meta[version_key];
                if (http) {
                    if (http_version != version || header(headers, "mcp-method") != method)
                        return failure(-32020, "MCP routing headers do not match the request.");
                    const char* name_key = method == "tools/call" || method == "prompts/get" ? "name"
                                           : method == "resources/read"                      ? "uri"
                                                                                             : nullptr;
                    if (name_key && params.contains(name_key) && params[name_key].is_string() &&
                        header(headers, "mcp-name") != header_value(params[name_key]))
                        return failure(-32020, "Mcp-Name does not match the request.");
                }
                if (version != modern_version) {
                    auto reply = failure(-32022, "Unsupported protocol version.");
                    (*reply.body)["error"]["data"] = {{"supported", versions}, {"requested", version}};
                    return reply;
                }
            } else if (http && method != "initialize" && !legacy(http_version)) {
                return failure(-32600, "A supported MCP-Protocol-Version header is required.");
            }

            Json result = Json::object();
            if (method == "initialize" && !modern) {
                if (!params.contains("protocolVersion") || !params["protocolVersion"].is_string() ||
                    !params.contains("capabilities") || !params["capabilities"].is_object() ||
                    !params.contains("clientInfo") || !params["clientInfo"].is_object() ||
                    !params["clientInfo"].contains("name") || !params["clientInfo"]["name"].is_string() ||
                    !params["clientInfo"].contains("version") || !params["clientInfo"]["version"].is_string())
                    return failure(-32602, "initialize requires protocolVersion, capabilities and clientInfo.");
                const std::string requested = params["protocolVersion"];
                result = {
                    {"protocolVersion", legacy(requested) ? requested : legacy_version},
                    {"capabilities", capabilities},
                    {"serverInfo", server_info},
                    {"instructions", instructions}
                };
                if (!http)
                    initialized_ = true;
            } else if (method == "ping") {
                // Ping is valid before legacy initialization.
            } else {
                if (!modern && !http && !initialized_)
                    return failure(-32600, "Initialize the MCP connection before calling methods.");
                if (method == "server/discover") {
                    result = {
                        {"supportedVersions", versions}, {"capabilities", capabilities}, {"instructions", instructions}
                    };
                } else if (
                    method == "tools/list" || method == "resources/list" || method == "resources/templates/list"
                ) {
                    if (params.contains("cursor"))
                        return failure(-32602, "This catalogue fits in one page; no cursor is available.");
                    if (method == "tools/list")
                        result["tools"] = tools_.list();
                    else if (method == "resources/templates/list")
                        result["resourceTemplates"] = {
                            {{"uriTemplate", "snt://artifacts/{id}{?offset}"},
                             {"name", "artifact"},
                             {"description", "Generated artifact bytes in base64 chunks; offset is a byte offset."}}
                        };
                    else
                        result["resources"] = {
                            {{"uri", "snt://environments"},
                             {"name", "environments"},
                             {"description", "Configured DIPL inputs and DIPH5 snapshots"},
                             {"mimeType", "application/json"}},
                            {{"uri", "snt://help/dipl"}, {"name", "DIPL agent guide"}, {"mimeType", "text/markdown"}}
                        };
                } else if (method == "resources/read") {
                    if (!params.contains("uri") || !params["uri"].is_string())
                        return failure(-32602, "resources/read requires a uri string.");
                    const std::string uri = params["uri"];
                    if (uri == "snt://environments" || uri == "snt://help/dipl") {
                        result["contents"] = {
                            {{"uri", uri},
                             {"mimeType", uri == "snt://environments" ? "application/json" : "text/markdown"},
                             {"text", uri == "snt://environments" ? tools_.environments().dump() : dipl_help()}}
                        };
                    } else {
                        std::smatch match;
                        if (!std::regex_match(
                                uri,
                                match,
                                std::regex("snt://artifacts/(artifact-[0-9]+-[0-9]+)(?:\\?offset=([0-9]+))?")
                            ))
                            return failure(modern ? -32602 : -32002, "Unknown resource.");
                        try {
                            const auto offset = match[2].matched ? std::stoull(match[2].str()) : 0;
                            const auto chunk = execution_.storage().read(match[1].str(), offset, chunk_limit);
                            Json meta = {
                                {"artifact", chunk["artifact"]},
                                {"offset", chunk["offset"]},
                                {"length", chunk["length"]},
                                {"next_uri", nullptr}
                            };
                            if (!chunk["next_offset"].is_null())
                                meta["next_uri"] =
                                    "snt://artifacts/" + match[1].str() + "?offset=" + chunk["next_offset"].dump();
                            result["contents"] = {
                                {{"uri", uri},
                                 {"mimeType", chunk["artifact"]["mimeType"]},
                                 {"blob", chunk["data"]},
                                 {"_meta", meta}}
                            };
                        } catch (const std::exception& exception) {
                            return failure(modern ? -32602 : -32002, exception.what());
                        }
                    }
                } else if (method == "tools/call") {
                    if (!params.contains("name") || !params["name"].is_string())
                        return failure(-32602, "tools/call requires a name string.");
                    const std::string name = params["name"];
                    if (!tools_.contains(name))
                        return failure(-32602, "Unknown tool: " + name);
                    result = execution_.run(id, name, params.value("arguments", Json::object()), disconnected);
                    if (result.is_null())
                        return {std::nullopt, 202};
                } else {
                    return failure(-32601, "Method not found: " + method, modern ? 404 : 200);
                }
            }
            if (modern) {
                result["resultType"] = "complete";
                result["_meta"] = {{"io.modelcontextprotocol/serverInfo", server_info}};
                if (method == "server/discover" || method == "tools/list" || method == "resources/list" ||
                    method == "resources/templates/list" || method == "resources/read") {
                    // Inputs are reloaded on each call, so no result is cached.
                    result["ttlMs"] = 0;
                    result["cacheScope"] = "private";
                }
            }
            return {Json{{"jsonrpc", "2.0"}, {"id", id}, {"result", result}}, 200};
        } catch (const Json::exception&) {
            return failure(-32602, "Invalid parameter types.");
        } catch (const std::exception& exception) {
            std::cerr << "MCP request failed: " << exception.what() << '\n';
            return failure(-32603, "Internal error.", 500);
        }
    }
} // namespace snt::mcp
