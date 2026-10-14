#ifndef DIP_VALUE_METADATA_H
#define DIP_VALUE_METADATA_H

#include <set>
#include <snt/dip/settings.h>
#include <snt/val/array.h>
#include <string>
#include <vector>

namespace snt::dip {

    /** Metadata attached to an explicit DIPL node or schema definition. */
    struct ValueMetadata {
        std::string description;
        std::string authors;
        std::string title;
        std::string journal;
        std::string year;
        std::string volume;
        std::string issue;
        std::string pages;
        std::string doi;
        std::string url;
        std::string version;
        std::string created;
        std::string modified;
        std::string license;
        std::string rationale;
        std::vector<std::string> native;
        std::vector<std::string>
            requires;
        std::vector<std::string> conflicts;
        std::vector<std::string> implies;
        std::vector<std::string> see;
        std::vector<std::string> example;
        std::string recommended_range;
        std::string performance_impact;
        std::string scientific_impact;
        std::string deprecated;
        std::string replacement;
        std::string since;
        std::string category;
        std::string visibility;
        std::set<PropertyType> present; ///< Explicitly assigned fields, including empty values.

        bool set_property(PropertyType property, const val::Array::StringType& values);
        bool has_property(PropertyType property) const { return present.count(property) != 0; }
        std::vector<std::string> present_names() const;
        /** Replace fields explicitly present in `other`; list metadata replaces as a whole. */
        void merge_explicit(const ValueMetadata& other);
    };

} // namespace snt::dip

#endif
