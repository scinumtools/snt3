#ifndef SNT_DIP_INSPECT_VALUE_FACTS_H
#define SNT_DIP_INSPECT_VALUE_FACTS_H

#include <snt/dip/inspect/inspection.h>

namespace snt::dip::detail {

// Shared, un-cloned facts for the owned inspection and bounded description APIs.
// The value pointer borrows from the environment while assembling a result; callers decide
// whether to clone it or serialize a bounded representation.
struct ValueFacts {
    std::string path;
    core::DataType declared_type;
    core::DataType stored_type;
    val::Array::ShapeType shape;
    std::size_t elements;
    bool scalar_dimension;
    const val::BaseValue* value;
    std::optional<puq::Quantity> units;
    ValueMetadata metadata;
    std::vector<std::string> tags;
    bool overridden;
    Provenance provenance;
    core::SourceLocation declaration_location;
    std::optional<core::SourceLocation> override_location;
    std::vector<ValueChange> changes;
    bool source_text_available;
    std::string table_path;
    std::string enforced_condition;
    std::vector<std::string> enforced_options;
};

ValueFacts inspect_value_facts(const Environment& env, std::string_view path,
                              bool include_description_details = false);

} // namespace snt::dip::detail

#endif
