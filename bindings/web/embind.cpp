#include "runtime.h"

#include <emscripten/bind.h>

namespace {
using emscripten::val;

val strings(const std::vector<std::string>& values) {
    val result = val::array();
    for (const auto& value : values) result.call<void>("push", value);
    return result;
}

val optional_string(const std::optional<std::string>& value) {
    return value ? val(*value) : val::null();
}

val location(const std::optional<snt::core::SourceLocation>& value) {
    if (!value) return val::null();
    val result = val::object();
    result.set("source", value->source);
    result.set("line", value->line);
    return result;
}

val metadata(const snt::dip::ValueMetadata& value) {
    val result = val::object();
    result.set("description", value.description);
    result.set("category", value.category);
    result.set("visibility", value.visibility);
    result.set("rationale", value.rationale);
    result.set("recommendedRange", value.recommended_range);
    result.set("scientificImpact", value.scientific_impact);
    result.set("performanceImpact", value.performance_impact);
    result.set("deprecated", value.deprecated);
    result.set("replacement", value.replacement);
    result.set("since", value.since);
    result.set("native", strings(value.native));
    result.set("requires", strings(value.requires));
    result.set("conflicts", strings(value.conflicts));
    result.set("implies", strings(value.implies));
    result.set("see", strings(value.see));
    result.set("example", strings(value.example));
    return result;
}

val description(const snt::web::Model& model, const std::string& path, std::size_t max_elements) {
    const auto item = model.describe(path, max_elements);
    val result = val::object();
    result.set("path", item.path);
    result.set("kind", item.kind);
    result.set("declaredType", item.declared_type);
    result.set("storedType", item.stored_type);
    val shape = val::array();
    for (const auto dimension : item.shape) shape.call<void>("push", dimension);
    result.set("shape", shape);
    result.set("elements", item.elements);
    result.set("valueText", optional_string(item.value_text));
    result.set("valueUnavailableReason", item.value_unavailable_reason);
    result.set("units", optional_string(item.units));
    result.set("metadata", metadata(item.metadata));
    result.set("tags", strings(item.tags));
    result.set("overridden", item.overridden);
    result.set("options", strings(item.enforced_options));
    result.set("condition", item.enforced_condition.empty() ? val::null() : val(item.enforced_condition));
    result.set("declaration", location(item.declaration));
    result.set("overrideLocation", location(item.override_location));
    result.set("dependenciesRecorded", item.dependencies_recorded);
    val dependencies = val::array();
    for (const auto& edge : item.dependencies) {
        val entry = val::object();
        entry.set("target", edge.target);
        entry.set("request", edge.request);
        dependencies.call<void>("push", entry);
    }
    result.set("dependencies", dependencies);
    return result;
}

val paths(const snt::web::Model& model) { return strings(model.paths()); }

val member(const snt::dip::SchemaMemberInspection& item) {
    val result = val::object();
    result.set("name", item.name);
    result.set("relativePath", item.relative_path);
    result.set("kind", item.kind);
    result.set("type", item.type.empty() ? val::null() : val(item.type));
    result.set("units", item.units.empty() ? val::null() : val(item.units));
    result.set("schemaRefs", strings(item.schema_refs));
    result.set("options", strings(item.options));
    result.set("condition", item.condition.empty() ? val::null() : val(item.condition));
    result.set("metadata", metadata(item.metadata));
    result.set("origin", location(item.origin));
    val dimensions = val::array();
    for (const auto& dimension : item.dimensions) {
        val range = val::object();
        range.set("min", dimension.dmin);
        range.set("max", dimension.dmax == snt::val::Array::max_range
            ? val::null() : val(dimension.dmax));
        dimensions.call<void>("push", range);
    }
    result.set("dimensions", dimensions);
    val children = val::array();
    for (const auto& child : item.members) children.call<void>("push", member(child));
    result.set("members", children);
    return result;
}

val schemas(const snt::web::Model& model) {
    const auto inspected = model.schemas();
    val result = val::object();
    result.set("schema", "snt-schema-hierarchy/1");
    result.set("definitionsAvailable", inspected.definitions_available);
    result.set("applicationsComplete", inspected.applications_complete);
    val definitions = val::array();
    for (const auto& definition : inspected.definitions) {
        val entry = val::object();
        entry.set("id", definition.id);
        entry.set("name", definition.name);
        entry.set("metadata", metadata(definition.metadata));
        entry.set("origin", location(definition.origin));
        val members = val::array();
        for (const auto& item : definition.members) members.call<void>("push", member(item));
        entry.set("members", members);
        definitions.call<void>("push", entry);
    }
    result.set("definitions", definitions);
    val applications = val::array();
    for (const auto& application : inspected.applications) {
        val entry = val::object();
        entry.set("path", application.path);
        entry.set("kind", application.kind);
        entry.set("schemaIds", strings(application.schema_ids));
        entry.set("inheritedFromCollection", application.inherited_from_collection
            ? val(*application.inherited_from_collection) : val::null());
        entry.set("origin", location(application.origin));
        applications.call<void>("push", entry);
    }
    result.set("applications", applications);
    val values = val::array();
    for (const auto& item : inspected.values) {
        val entry = val::object();
        entry.set("path", item.path);
        entry.set("appliedSchemaIds", strings(item.applied_schema_ids));
        entry.set("contributingSchemaId", optional_string(item.contributing_schema_id));
        values.call<void>("push", entry);
    }
    result.set("values", values);
    return result;
}

std::vector<snt::dip::ProjectText> named_texts(const val& project, const char* field) {
    const val entries = project[field];
    if (entries.isUndefined() || entries.isNull()) return {};
    const auto count = entries["length"].as<unsigned>();
    std::vector<snt::dip::ProjectText> result;
    result.reserve(count);
    for (unsigned i = 0; i < count; ++i) {
        const val item = entries[i];
        result.push_back({item["name"].as<std::string>(), item["text"].as<std::string>()});
    }
    return result;
}

snt::dip::ProjectInput project_input(const val& project) {
    snt::dip::ProjectInput result;
    result.units = named_texts(project, "units");
    result.schemas = named_texts(project, "schemas");
    result.code = named_texts(project, "code");
    result.overrides = named_texts(project, "overrides");
    return result;
}

snt::web::Model parse_project(const val& project, const std::string& override_body) {
    return snt::web::Model(project_input(project), override_body);
}

val validation_result(const snt::web::Validation& result) {
    val output = val::object();
    output.set("valid", result.valid);
    if (result.diagnostic) {
        val diagnostic = val::object();
        diagnostic.set("code", result.diagnostic->code);
        diagnostic.set("message", result.diagnostic->message);
        diagnostic.set("details", result.diagnostic->details);
        diagnostic.set("suggestion", result.diagnostic->suggestion);
        diagnostic.set("path", optional_string(result.diagnostic->node_path));
        diagnostic.set("location", location(result.diagnostic->location));
        output.set("diagnostic", diagnostic);
    } else output.set("diagnostic", val::null());
    return output;
}

val validate(const std::string& source, const std::string& override_body) {
    return validation_result(snt::web::validate(source, override_body));
}

val validate_project(const val& project, const std::string& override_body) {
    return validation_result(snt::web::validate(project_input(project), override_body));
}
} // namespace

EMSCRIPTEN_BINDINGS(snt_dipl_web) {
    emscripten::class_<snt::web::Model>("NativeModel")
        .constructor<std::string, std::string>()
        .function("paths", &paths)
        .function("describe", &description)
        .function("schemas", &schemas)
        .function("withOverride", &snt::web::Model::with_override);
    emscripten::function("validateDIPL", &validate);
    emscripten::function("parseProjectDIPL", &parse_project);
    emscripten::function("validateProjectDIPL", &validate_project);
}
