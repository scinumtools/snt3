#include <snt/dip/settings.h>

namespace snt::dip {

    std::unordered_map<ValueOrigin, std::string> ValueOriginNames{
        {ValueOrigin::Empty, "empty"},
        {ValueOrigin::None, "none"},
        {ValueOrigin::Keyword, "keyword"},
        {ValueOrigin::Boolean, "boolean"},
        {ValueOrigin::Number, "number"},
        {ValueOrigin::String, "string"},
        {ValueOrigin::Array, "array"},
        {ValueOrigin::Reference, "reference"},
        {ValueOrigin::ReferenceRaw, "raw reference"},
        {ValueOrigin::ReferenceRel, "relative reference"},
        {ValueOrigin::ReferenceSelf, "self reference"},
        {ValueOrigin::Function, "function"},
        {ValueOrigin::Expression, "expression"},
        {ValueOrigin::Schema, "schema"},
    };

    std::unordered_map<NodeDtype, std::string> NodeDtypeNames{
        {NodeDtype::None, "node"},
        {NodeDtype::Empty, "empty"},
        {NodeDtype::Unit, "unit"},
        {NodeDtype::Source, "source"},
        {NodeDtype::Group, "group"},
        {NodeDtype::Case, "case"},
        {NodeDtype::Schema, "schema"},
        {NodeDtype::Import, "import"},
        {NodeDtype::Boolean, "boolean"},
        {NodeDtype::Integer, "integer"},
        {NodeDtype::Float, "float"},
        {NodeDtype::String, "string"},
        {NodeDtype::Table, "table"},
        {NodeDtype::Deferred, "deffered"},
        {NodeDtype::Modification, "modification"},
        {NodeDtype::Property, "property"},
    };
} // namespace snt::dip
