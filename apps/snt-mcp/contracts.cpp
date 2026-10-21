#include "storage.h"

#include <snt/dip/inspect/diagnostic.h>

namespace snt::mcp {
    namespace {
        Json type(const char* name) {
            return {{"type", name}};
        }
        Json array(const Json& item) {
            return {{"type", "array"}, {"items", item}};
        }
        Json object(Json properties, Json required = Json::array()) {
            return {{"type", "object"}, {"properties", std::move(properties)}, {"required", std::move(required)}};
        }
        const Json string = type("string"), integer = type("integer"), boolean = type("boolean");
        const Json nullable_string = {{"type", {"string", "null"}}};
        Json artifact_schema() {
            return object(
                {{"id", string},
                 {"uri", string},
                 {"kind", string},
                 {"mimeType", string},
                 {"filename", string},
                 {"size", integer},
                 {"sha256", string},
                 {"created_at", integer},
                 {"expires_at", integer}},
                {"id", "uri", "kind", "mimeType", "filename", "size", "sha256", "created_at", "expires_at"}
            );
        }
        Json description_schema() {
            return object(
                {{"schema_version", string},
                 {"path", string},
                 {"kind", string},
                 {"canonical_path", nullable_string},
                 {"declared_type", string},
                 {"stored_type", string},
                 {"shape", array(integer)},
                 {"elements", integer},
                 {"units", nullable_string},
                 {"tags", array(string)},
                 {"overridden", boolean},
                 {"value",
                  object(
                      {{"available", boolean}, {"encoding", string}, {"text", string}, {"reason", string}},
                      {"available"}
                  )},
                 {"metadata", type("object")},
                 {"rules", type("object")},
                 {"dependencies", object({{"available", boolean}, {"reads", array(type("object"))}}, {"available"})},
                 {"provenance", type("object")}},
                {"path", "kind", "value", "units", "shape", "metadata"}
            );
        }
    } // namespace
    Json output_schema(const std::string& name) {
        const auto artifact = artifact_schema();
        Json result;
        if (name == "dipl_environments")
            result = object(
                {{"environments",
                  array(object(
                      {{"name", string},
                       {"input", string},
                       {"snapshot", boolean},
                       {"capabilities",
                        object(
                            {{"overrides", boolean}, {"snapshot", boolean}, {"export", boolean}, {"report", boolean}}
                        )}},
                      {"name", "input", "snapshot", "capabilities"}
                  ))}},
                {"environments"}
            );
        else if (name == "dipl_list" || name == "dipl_evaluate")
            result = object(
                {{"schema_version", string},
                 {"total", integer},
                 {"truncated", boolean},
                 {"items", array(description_schema())},
                 {"next_cursor", nullable_string}},
                {"schema_version", "total", "truncated", "items", "next_cursor"}
            );
        else if (name == "dipl_describe")
            result = description_schema();
        else if (name == "dipl_sources")
            result = object(
                {{"schema", string},
                 {"sources",
                  array(object(
                      {{"name", string},
                       {"path", string},
                       {"hash_algorithm", string},
                       {"hash", string},
                       {"metadata", type("object")}},
                      {"name", "path", "hash_algorithm", "hash"}
                  ))}},
                {"schema", "sources"}
            );
        else if (name == "dipl_schemas")
            result = object(
                {{"schema", string},
                 {"definitions_available", boolean},
                 {"applications_complete", boolean},
                 {"definitions", array(type("object"))},
                 {"applications", array(type("object"))},
                 {"values", array(type("object"))}},
                {"schema", "definitions_available", "applications_complete", "definitions", "applications", "values"}
            );
        else if (name == "dipl_override_contract")
            result = object(
                {{"schema_version", string},
                 {"path", string},
                 {"kind", string},
                 {"reason", nullable_string},
                 {"resolved_path", nullable_string},
                 {"declared_type", nullable_string},
                 {"current_shape", array(integer)},
                 {"units", nullable_string},
                 {"rules", type("object")},
                 {"item_schemas", array(string)},
                 {"snapshot_input", boolean},
                 {"requires_preview", boolean}},
                {"path", "kind", "rules", "snapshot_input", "requires_preview"}
            );
        else if (name == "dipl_preview")
            result = object(
                {{"schema_version", string},
                 {"baseline_valid", boolean},
                 {"candidate_valid", boolean},
                 {"baseline_diagnostics", array(type("object"))},
                 {"candidate_diagnostics", array(type("object"))},
                 {"accepted_override_targets", array(string)},
                 {"diff", type("object")}},
                {"baseline_valid", "candidate_valid", "baseline_diagnostics", "candidate_diagnostics", "diff"}
            );
        else if (name == "dipl_compare")
            result = object(
                {{"scope", string},
                 {"equal", boolean},
                 {"added", integer},
                 {"removed", integer},
                 {"changed", integer},
                 {"omitted", integer},
                 {"result", string},
                 {"differences",
                  array(object(
                      {{"path", string},
                       {"category", string},
                       {"kind", string},
                       {"fields", array(string)},
                       {"before", string},
                       {"after", string},
                       {"changed_elements", integer},
                       {"example_indices", array(integer)}},
                      {"path", "category", "kind", "fields", "before", "after", "changed_elements", "example_indices"}
                  ))}},
                {"scope", "equal", "added", "removed", "changed", "omitted", "differences", "result"}
            );
        else if (name == "dipl_export" || name == "dipl_report")
            result = object(
                {{"format", string}, {"content", string}, {"artifact", artifact}, {"inline", boolean}},
                {"format", "artifact", "inline"}
            );
        else if (name == "dipl_snapshot")
            result = object(
                {{"environment", string}, {"artifact", artifact}, {"overrides", array(string)}},
                {"environment", "artifact", "overrides"}
            );
        else if (name == "artifact_read")
            result = object(
                {{"artifact", artifact},
                 {"encoding", string},
                 {"data", string},
                 {"offset", integer},
                 {"length", integer},
                 {"next_offset", {{"type", {"integer", "null"}}}}},
                {"artifact", "encoding", "data", "offset", "length", "next_offset"}
            );
        else
            result = object({{"result", string}}, {"result"});
        const auto error = object(
            {{"error",
              object(
                  {{"code", string},
                   {"message", string},
                   {"details", string},
                   {"suggestion", string},
                   {"retryable", boolean},
                   {"location", {{"type", {"object", "null"}}}}},
                  {"code", "message", "details", "suggestion", "retryable", "location"}
              )}},
            {"error"}
        );
        const auto reference = object({{"result_ref", artifact}, {"summary", string}}, {"result_ref", "summary"});
        return {{"type", "object"}, {"anyOf", {result, error, reference}}};
    }
    void describe_arguments(Json& properties) {
        const std::map<std::string, std::string> descriptions = {
            {"environment", "Configured input name or environment identifier returned by dipl_snapshot."},
            {"overrides",
             "Unwrapped DIPL override bodies, e.g. simulation.distance = 48 m. Source inputs only; never saved to "
             "source files."},
            {"query", "DIPL selection: ? for all values, ?simulation. for a subtree, ?simulation.speed for one value."},
            {"path", "Exact DIPL parameter or group path, without the leading ?."},
            {"limit",
             "Maximum nodes in this page (0 gives counts); follow next_cursor with a positive limit for more."},
            {"cursor",
             "Continuation returned as next_cursor. Resumes the original immutable listing; query and tag filters are "
             "ignored."},
            {"max_value_elements", "Maximum elements included in each value description; zero omits value text."},
            {"max_details", "Maximum differences returned; counts still cover all differences."},
            {"max_array_examples", "Maximum flat zero-based changed array indices per difference."},
            {"record_dependencies", "Capture evaluation reads for source inputs; snapshots expose their saved graph."},
            {"before", "Configured DIPH5 input or snapshot environment identifier for the baseline."},
            {"after", "Configured DIPH5 input or snapshot environment identifier for the candidate."},
            {"format", "Output format; generated content is also retrievable through the returned artifact."},
            {"source", "Inline DIPL declarations; use a configured project for relative multi-file inputs."},
            {"id", "Opaque artifact id returned by a tool; not a filesystem path."},
            {"offset", "Zero-based byte offset in the artifact, normally the previous next_offset."},
            {"length", "Maximum bytes to read, 1..131072. Decode data as base64 and concatenate chunks."},
            {"tags_all", "Require all these explicit tags."},
            {"tags_any", "Require at least one of these explicit tags."},
            {"tags_none", "Exclude parameters with any of these explicit tags."},
            {"scope", "effective compares evaluated parameters; full also compares saved metadata and provenance."},
            {"operation", "PUQ operation: eval, convert, info, or list."},
            {"expression", "PUEL expression, e.g. 2*m. Required except for list."},
            {"output_units", "Target units, e.g. cm. Required for convert."},
            {"input_system", "Input unit system; empty uses the SNT default."},
            {"output_system", "Output unit system; empty uses the SNT default."},
            {"output_quantity", "Optional named physical output quantity."},
            {"category", "Unit-definition category for list."}
        };
        for (auto it = properties.begin(); it != properties.end(); ++it)
            if (const auto found = descriptions.find(it.key()); found != descriptions.end())
                it.value()["description"] = found->second;
    }
    Json failure_result(const std::exception& error) {
        auto diagnostic = dip::diagnostic_from_exception(error);
        if (const auto* service = dynamic_cast<const ServiceError*>(&error))
            diagnostic.code = service->code;
        else if (dynamic_cast<const std::invalid_argument*>(&error) || dynamic_cast<const Json::exception*>(&error))
            diagnostic.code = "argument.invalid";
        Json location = nullptr;
        if (diagnostic.location) {
            location = {{"source", diagnostic.location->source}, {"line", diagnostic.location->line}};
            if (diagnostic.location->column)
                location["column"] = *diagnostic.location->column;
        }
        auto bound = [](std::string text) {
            auto size = std::min(text.size(), std::size_t{16384});
            while (size < text.size() && size > 0 && (static_cast<unsigned char>(text[size]) & 0xc0) == 0x80)
                --size;
            return text.substr(0, size);
        };
        Json detail = {
            {"code", diagnostic.code},
            {"message", bound(diagnostic.message)},
            {"details", bound(diagnostic.details)},
            {"suggestion", bound(diagnostic.suggestion)},
            {"location", location},
            {"retryable", diagnostic.code == "server.busy"}
        };
        return {
            {"isError", true},
            {"structuredContent", {{"error", detail}}},
            {"content", {{{"type", "text"}, {"text", bound(error.what())}}}}
        };
    }
    const std::string& dipl_help() {
        static const std::string text = R"HELP(# SNT3 agent guide

Start with dipl_environments, then dipl_list and dipl_describe. Inspect units,
constraints and provenance before suggesting values. Validation establishes DIPL
consistency, not scientific suitability.

DIPL uses indentation and declared types:
```
simulation
  distance float = 24 m
  duration float = 8 s
    !condition ({.} > 0 s)
  speed float = ({?simulation.distance} / {?simulation.duration}) m/s
    ?descr "Average speed"
    !tags ["derived", "physics"]
```
`?simulation.speed` selects one parameter; `?simulation.` selects its subtree;
`?` selects all. Descriptions preserve values as DIPL text, units separately.
Use dipl_override_contract for writable targets; do not infer them from names.

Override arguments contain assignment bodies without an override wrapper:
`["simulation.distance = 48 m"]`. Preview them with dipl_preview and check
candidate_valid and candidate_diagnostics. An invalid candidate is a successful
preview result, not a successful validation. Export/report errors use isError.

Reproducible workflow:
1. Create a baseline using dipl_snapshot(environment="default").
2. Preview overrides on the source input.
3. Create a candidate using dipl_snapshot(environment="default", overrides=[...]).
4. Use the returned environment identifier for describe/list/export/report.
5. Compare the two snapshot environment identifiers with dipl_compare.

Snapshot creation validates the actual candidate and records its dependency graph.
It does not lock or rewrite source files. Live-source calls independently read
current files; a preview does not freeze them. Snapshot handles are immutable
evaluated data, not source definitions: they cannot accept more overrides or
restore executable schemas. Source fingerprints remain available in dipl_sources.

dipl_export and dipl_report also accept overrides directly on source inputs.
Their artifact includes a SHA-256, MIME type, size and expiry. Small outputs have
inline content. Retrieve any artifact using artifact_read(id, offset, length);
base64-decode data, append bytes, and continue at next_offset until it is null.
Large structured tool results return result_ref: retrieve and JSON-decode it.
resources/read also accepts the artifact URI, with optional ?offset=N chunks.

Follow dipl_list.next_cursor using dipl_list(cursor=..., limit=100). Continuations
read the frozen listing, even if source inputs later change. They expire with
artifacts. Keep page limits small when including large values.

Artifacts expire after one hour or when this server stops. Download outputs you
want to keep. Storage is bounded; nothing is written into the published project.
There is one active tool execution at a time. server.busy is retryable. Execution
limits stop native workers; cancellation publishes no partial artifacts.

The server exposes files with its own permissions. It is intended for trusted
projects and clients. HTTP can require a service bearer token from an owner-only
file; remote deployment also needs an HTTPS reverse proxy. Token holders share
access to every published input and artifact. This MCP
does not edit sources, launch simulations, or register host callbacks.
)HELP";
        return text;
    }
} // namespace snt::mcp
