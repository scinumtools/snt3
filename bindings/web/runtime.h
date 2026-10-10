#ifndef SNT_WEB_RUNTIME_H
#define SNT_WEB_RUNTIME_H

#include <snt/core/diagnostic.h>
#include <snt/dip/dip.h>
#include <snt/dip/inspect/inspector.h>

#include <string>
#include <optional>
#include <vector>

namespace snt::web {

/** One evaluated, string-backed DIPL model. Overrides rebuild the model through
 * the ordinary parser, so references, units, and validation retain C++ semantics.
 */
class Model {
    dip::ProjectInput project_;
    std::string override_body_;
    dip::Environment environment_;

  public:
    explicit Model(std::string source, std::string override_body = {});
    explicit Model(dip::ProjectInput project, std::string override_body = {});
    const dip::Environment& environment() const { return environment_; }
    std::vector<std::string> paths() const;
    dip::SemanticDescription describe(const std::string& path, std::size_t max_value_elements = 16) const;
    dip::SchemaHierarchyInspection schemas() const;
    /** Evaluate a replacement override body against the same original source. */
    Model with_override(std::string body) const;
};

struct Validation {
    bool valid = false;
    std::optional<core::Diagnostic> diagnostic;
};

Validation validate(const std::string& source, const std::string& override_body = {});
Validation validate(const dip::ProjectInput& project, const std::string& override_body = {});

} // namespace snt::web

#endif
