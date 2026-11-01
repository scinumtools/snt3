#include <snt/dip/inspect/inspector.h>

#include <stdexcept>
#include <unordered_set>

namespace snt::dip {
    namespace {
        void check_field(const std::string& field) {
            if (field.empty() || field.front() == '.' || field.back() == '.' ||
                field.find("..") != std::string::npos || field.find_first_of("[]?") != std::string::npos)
                throw std::invalid_argument("Record fields must be relative named paths: " + field);
        }
    }

    RecordResult Inspector::records(const RecordQuery& query) const {
        if (query.collection.empty())
            throw std::invalid_argument("Specify a collection path.");
        if (query.limit > 1000)
            throw std::invalid_argument("Record query limit cannot exceed 1000.");
        const auto& collection = env_->hierarchy.get_collection(query.collection);
        if (collection.kind != Path::Kind::Map)
            throw std::invalid_argument("Record queries require a keyed map collection.");
        for (const auto& field : query.fields)
            check_field(field);
        for (const auto& filter : query.filters)
            check_field(filter.field);

        std::unordered_set<std::string> available;
        for (const auto& node : env_->nodes.get_nodes())
            if (node)
                available.insert(node->path.name);

        RecordResult result;
        for (const auto& key : collection.items) {
            const std::string item_path = query.collection + "[" + key + "]";
            bool matches = true;
            for (const auto& filter : query.filters) {
                const auto path = item_path + "." + filter.field;
                if (!available.count(path)) {
                    matches = false;
                    break;
                }
                const auto value = describe(path, 1);
                if (value.kind != "value" || value.stored_type != "str" || !value.shape.empty() ||
                    !value.value_text || cursor(path).as<std::string>() != filter.value) {
                    matches = false;
                    break;
                }
            }
            if (!matches)
                continue;
            if (result.total++ < query.offset || result.items.size() >= query.limit)
                continue;
            RecordItem item;
            item.id = key;
            item.path = item_path;
            for (const auto& field : query.fields) {
                RecordField selected;
                selected.name = field;
                const auto path = item_path + "." + field;
                selected.present = available.count(path) != 0;
                if (selected.present)
                    selected.description = describe(path, 1);
                item.fields.push_back(std::move(selected));
            }
            result.items.push_back(std::move(item));
        }
        return result;
    }
}
