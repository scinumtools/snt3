#include "runtime.h"

#include <snt/dip/inspect/diagnostic.h>

#include <utility>

namespace snt::web {

Model::Model(std::string source, std::string override_body)
    : Model(dip::ProjectInput{{}, {}, {{"source", std::move(source)}}, {}}, std::move(override_body)) {}

Model::Model(dip::ProjectInput project, std::string override_body)
    : project_(std::move(project)), override_body_(std::move(override_body)) {
    dip::DIP parser;
    parser.add_project(project_);
    if (!override_body_.empty()) parser.add_override_string(override_body_);
    environment_ = parser.parse(true);
}

Model::Model(dip::ProjectBundle bundle, std::string override_body)
    : bundle_(std::move(bundle)), override_body_(std::move(override_body)) {
    dip::DIP parser;
    parser.add_project(*bundle_);
    if (!override_body_.empty()) parser.add_override_string(override_body_);
    environment_ = parser.parse(true);
}

std::vector<std::string> Model::paths() const {
    return dip::Inspector{environment_}.select_paths();
}

dip::SemanticDescription Model::describe(const std::string& path, std::size_t max_value_elements) const {
    return dip::Inspector{environment_}.describe(path, max_value_elements);
}

std::vector<dip::SemanticDescription> Model::describe_many(
    const std::vector<std::string>& paths, std::size_t max_value_elements) const {
    dip::Inspector inspector{environment_};
    std::vector<dip::SemanticDescription> descriptions;
    descriptions.reserve(paths.size());
    for (const auto& path : paths)
        descriptions.push_back(inspector.describe(path, max_value_elements));
    return descriptions;
}

dip::SchemaHierarchyInspection Model::schemas() const {
    return dip::Inspector{environment_}.schema_hierarchy();
}

Model Model::with_override(std::string body) const {
    if (bundle_) return Model(*bundle_, std::move(body));
    return Model(project_, std::move(body));
}

Validation validate(const std::string& source, const std::string& override_body) {
    return validate(dip::ProjectInput{{}, {}, {{"source", source}}, {}}, override_body);
}

Validation validate(const dip::ProjectInput& project, const std::string& override_body) {
    try {
        Model model(project, override_body);
        return {true, std::nullopt};
    } catch (const std::exception& error) {
        return {false, dip::diagnostic_from_exception(error)};
    }
}

Validation validate(const dip::ProjectBundle& bundle, const std::string& override_body) {
    try {
        Model model(bundle, override_body);
        return {true, std::nullopt};
    } catch (const std::exception& error) {
        return {false, dip::diagnostic_from_exception(error)};
    }
}

} // namespace snt::web
