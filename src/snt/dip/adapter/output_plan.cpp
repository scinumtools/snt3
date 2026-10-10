#include <snt/dip/environment.h>
#include <snt/dip/output_plan.h>
#include <snt/dip/exceptions.h>

#include <map>
#include <utility>

namespace snt::dip {
    namespace {
        [[noreturn]] void invalid_mapping(const std::string& id, const std::string& detail) {
            throw EnvironmentException(
                "Invalid adapter output mapping",
                "Output mapping `" + id + "` " + detail + ".",
                "Give each output a stable ID, target, key, and typed value; replace existing IDs explicitly.",
                __FILE__, __LINE__
            );
        }

        ValueNode::PointerType copy_value(const ValueNode::PointerType& value, const std::string& id) {
            if (!value || !value->value)
                invalid_mapping(id, "has no evaluated value");
            return std::dynamic_pointer_cast<ValueNode>(value->clone(value->path));
        }

        OutputMapping make_mapping(std::string id, std::string target, std::string key,
                                   ValueNode::PointerType value, bool active, std::string rule,
                                   std::string origin, std::vector<std::string> dependencies) {
            OutputMapping mapping;
            mapping.id = std::move(id);
            mapping.target = std::move(target);
            mapping.key = std::move(key);
            mapping.value = copy_value(value, mapping.id);
            mapping.active = active;
            mapping.rule = std::move(rule);
            mapping.origin = std::move(origin);
            mapping.dependencies = std::move(dependencies);
            return mapping;
        }
    } // namespace

    void OutputPlan::insert(OutputMapping mapping, bool replace) {
        if (mapping.id.empty() || mapping.target.empty() || mapping.key.empty())
            invalid_mapping(mapping.id, "needs a nonempty ID, target, and native key");
        if (!mapping.value || !mapping.value->value)
            invalid_mapping(mapping.id, "has no evaluated value");
        for (auto& current : mappings_) {
            if (current.id != mapping.id)
                continue;
            if (!replace)
                invalid_mapping(mapping.id, "duplicates an existing ID");
            mapping.replacements = current.replacements;
            mapping.replacements.push_back(current.origin);
            current = std::move(mapping);
            return;
        }
        if (replace)
            invalid_mapping(mapping.id, "cannot replace a missing ID");
        mappings_.push_back(std::move(mapping));
    }

    void OutputPlan::add_node(const Environment& env, std::string id, std::string target, std::string key,
                              std::string source_path, bool active, std::string rule,
                              std::string origin, std::vector<std::string> dependencies) {
        auto mapping = make_mapping(std::move(id), std::move(target), std::move(key),
                                    env.get_node(source_path), active, std::move(rule),
                                    std::move(origin), std::move(dependencies));
        mapping.source_path = std::move(source_path);
        insert(std::move(mapping), false);
    }

    void OutputPlan::replace_node(const Environment& env, std::string id, std::string target, std::string key,
                                  std::string source_path, bool active, std::string rule,
                                  std::string origin, std::vector<std::string> dependencies) {
        auto mapping = make_mapping(std::move(id), std::move(target), std::move(key),
                                    env.get_node(source_path), active, std::move(rule),
                                    std::move(origin), std::move(dependencies));
        mapping.source_path = std::move(source_path);
        insert(std::move(mapping), true);
    }

    void OutputPlan::add_value(std::string id, std::string target, std::string key, ValueNode::PointerType value,
                               bool active, std::string rule, std::string origin,
                               std::vector<std::string> dependencies) {
        insert(make_mapping(std::move(id), std::move(target), std::move(key), std::move(value),
                            active, std::move(rule), std::move(origin), std::move(dependencies)), false);
    }

    void OutputPlan::replace_value(std::string id, std::string target, std::string key, ValueNode::PointerType value,
                                   bool active, std::string rule, std::string origin,
                                   std::vector<std::string> dependencies) {
        insert(make_mapping(std::move(id), std::move(target), std::move(key), std::move(value),
                            active, std::move(rule), std::move(origin), std::move(dependencies)), true);
    }

    void OutputPlan::restore(OutputMapping mapping) { insert(std::move(mapping), false); }

    void OutputPlan::validate() const {
        std::map<std::pair<std::string, std::string>, std::string> keys;
        for (const auto& mapping : mappings_) {
            if (!mapping.active)
                continue;
            const auto [it, inserted] = keys.emplace(std::make_pair(mapping.target, mapping.key), mapping.id);
            if (!inserted)
                invalid_mapping(mapping.id, "shares active native key `" + mapping.key + "` with `" + it->second + "`");
        }
    }

    std::vector<OutputMapping> OutputPlan::select(const std::string& target, bool active_only) const {
        std::vector<OutputMapping> selected;
        for (const auto& mapping : mappings_)
            if (mapping.target == target && (!active_only || mapping.active))
                selected.push_back(mapping);
        return selected;
    }
} // namespace snt::dip
