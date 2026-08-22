#include <snt/dip/settings.h>

namespace snt::dip {

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
}
