#ifndef SNT_DIP_INSPECT_RECORDS_H
#define SNT_DIP_INSPECT_RECORDS_H

#include <snt/dip/inspect/semantic.h>
#include <string>
#include <vector>

namespace snt::dip {
    /** Equality against a scalar field of a keyed collection item. */
    struct RecordFilter {
        std::string field;
        std::string value;
    };

    struct RecordField {
        std::string name;
        bool present = false;
        SemanticDescription description;
    };

    struct RecordItem {
        std::string id;
        std::string path;
        std::vector<RecordField> fields;
    };

    struct RecordQuery {
        std::string collection;
        std::vector<RecordFilter> filters;
        std::vector<std::string> fields;
        std::size_t offset = 0;
        std::size_t limit = 100;
    };

    struct RecordResult {
        std::size_t total = 0;
        std::vector<RecordItem> items;
    };
}

#endif
