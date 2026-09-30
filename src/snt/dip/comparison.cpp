#include <algorithm>
#include <cmath>
#include <limits>
#include <map>
#include <snt/core/string_format.h>
#include <snt/dip/comparison.h>
#include <snt/dip/cursor.h>
#include <snt/dip/environment.h>
#include <snt/val/values_number.h>
#include <snt/val/values_string.h>
#include <stdexcept>
#include <tuple>

namespace snt::dip {
    namespace {
        void field(Difference& difference, const char* name, bool changed) {
            if (changed)
                difference.fields.emplace_back(name);
        }

        std::string display_value(const ValueNode& node) {
            if (!node.value)
                return "null";
            if (node.value->get_size() > 8)
                return "<" + std::to_string(node.value->get_size()) + " elements>";
            core::StringFormatType format;
            format.valuePrecision = std::numeric_limits<double>::max_digits10;
            auto preview = node.value->to_string(format);
            if (preview.size() > 160) {
                std::size_t end = 160;
                while (end && (static_cast<unsigned char>(preview[end]) & 0xc0) == 0x80)
                    --end;
                preview.resize(end);
                preview += "...";
            }
            return preview;
        }

        std::string display_node(const ValueNode& node) {
            std::string result = display_value(node);
            if (node.units)
                result += " " + node.units->to_string(
                                    puq::UnitFormat(
                                        puq::Format::Display::UNITS,
                                        puq::Format::System::SHOW,
                                        puq::Format::Precision(std::numeric_limits<double>::max_digits10)
                                    )
                                );
            return result;
        }

        template <typename T> bool same_element(const T& left, const T& right) {
            return left == right;
        }
        template <> bool same_element<double>(const double& left, const double& right) {
            return left == right || (std::isnan(left) && std::isnan(right));
        }
        template <> bool same_element<long double>(const long double& left, const long double& right) {
            return left == right || (std::isnan(left) && std::isnan(right));
        }

        template <typename T>
        void compare_elements(
            const val::BaseValue& before,
            const val::BaseValue& after,
            const ComparisonOptions& options,
            Difference& difference
        ) {
            const auto* left = dynamic_cast<const val::ArrayValue<T>*>(&before);
            const auto* right = dynamic_cast<const val::ArrayValue<T>*>(&after);
            if (!left || !right)
                throw std::runtime_error("DIP value storage does not match its declared type.");
            if (left->get_size() != right->get_size()) {
                field(difference, "value", true);
                return;
            }
            for (std::size_t index = 0; index < left->get_size(); ++index) {
                if (same_element(left->get_value(index), right->get_value(index)))
                    continue;
                ++difference.changed_elements;
                if (difference.example_indices.size() < options.max_array_examples)
                    difference.example_indices.push_back(index);
            }
            field(difference, "value", difference.changed_elements != 0);
        }

        void compare_value(
            const ValueNode& before, const ValueNode& after, const ComparisonOptions& options, Difference& difference
        ) {
            if (!before.value || !after.value) {
                field(difference, "value", bool(before.value) != bool(after.value));
                return;
            }
            if (before.value->get_dtype() != after.value->get_dtype()) {
                field(difference, "value", true);
                return;
            }
            using core::DataType;
            switch (before.value->get_dtype()) {
            case DataType::Boolean:
                compare_elements<uint8_t>(*before.value, *after.value, options, difference);
                break;
            case DataType::Character:
                compare_elements<int8_t>(*before.value, *after.value, options, difference);
                break;
            case DataType::Integer8:
            case DataType::Integer16:
            case DataType::Integer32:
            case DataType::Integer64:
            case DataType::Integer:
                compare_elements<int64_t>(*before.value, *after.value, options, difference);
                break;
            case DataType::Integer8_U:
            case DataType::Integer16_U:
            case DataType::Integer32_U:
            case DataType::Integer64_U:
                compare_elements<uint64_t>(*before.value, *after.value, options, difference);
                break;
            case DataType::Float32:
            case DataType::Float64:
            case DataType::Float:
                compare_elements<double>(*before.value, *after.value, options, difference);
                break;
            case DataType::Float128:
                compare_elements<long double>(*before.value, *after.value, options, difference);
                break;
            case DataType::String:
                compare_elements<std::string>(*before.value, *after.value, options, difference);
                break;
            default:
                throw std::runtime_error("Unsupported DIPH5 value type in comparison.");
            }
        }

        bool same_metadata(const ValueMetadata& a, const ValueMetadata& b) {
#define SNT_META_EQUAL(name)                                                                                           \
    if (a.name != b.name)                                                                                              \
    return false
            SNT_META_EQUAL(description);
            SNT_META_EQUAL(authors);
            SNT_META_EQUAL(title);
            SNT_META_EQUAL(journal);
            SNT_META_EQUAL(year);
            SNT_META_EQUAL(volume);
            SNT_META_EQUAL(issue);
            SNT_META_EQUAL(pages);
            SNT_META_EQUAL(doi);
            SNT_META_EQUAL(url);
            SNT_META_EQUAL(version);
            SNT_META_EQUAL(created);
            SNT_META_EQUAL(modified);
            SNT_META_EQUAL(license);
            SNT_META_EQUAL(rationale);
            SNT_META_EQUAL(native);
            SNT_META_EQUAL(requires);
            SNT_META_EQUAL(conflicts);
            SNT_META_EQUAL(implies);
            SNT_META_EQUAL(see);
            SNT_META_EQUAL(example);
            SNT_META_EQUAL(recommended_range);
            SNT_META_EQUAL(performance_impact);
            SNT_META_EQUAL(scientific_impact);
            SNT_META_EQUAL(deprecated);
            SNT_META_EQUAL(replacement);
            SNT_META_EQUAL(since);
            SNT_META_EQUAL(category);
            SNT_META_EQUAL(visibility);
#undef SNT_META_EQUAL
            return true;
        }

        bool same_source(const SourceInfo& a, const SourceInfo& b) {
            return a.name == b.name && a.path == b.path && a.parent_name == b.parent_name &&
                   a.parent_line == b.parent_line && a.hash_algorithm == b.hash_algorithm && a.hash == b.hash;
        }

        bool same_source(const std::optional<SourceInfo>& a, const std::optional<SourceInfo>& b) {
            return (!a && !b) || (a && b && same_source(*a, *b));
        }

        bool same_units(const ValueNode& a, const ValueNode& b) {
            if (bool(a.units) != bool(b.units))
                return false;
            if (!a.units)
                return true;
            if (a.units->stype != b.units->stype)
                return false;
            const auto format = puq::UnitFormat(
                puq::Format::Display::UNITS,
                puq::Format::System::SHOW,
                puq::Format::Precision(std::numeric_limits<double>::max_digits10)
            );
            return a.units->to_string(format) == b.units->to_string(format);
        }

        bool same_options(const ValueNode& a, const ValueNode& b) {
            if (a.options.size() != b.options.size())
                return false;
            for (std::size_t i = 0; i < a.options.size(); ++i)
                if (a.options[i].value_raw != b.options[i].value_raw ||
                    a.options[i].units_raw != b.options[i].units_raw)
                    return false;
            return true;
        }

        void add(ComparisonResult& result, Difference difference) {
            if (difference.kind == DifferenceKind::Added)
                ++result.added;
            else if (difference.kind == DifferenceKind::Removed)
                ++result.removed;
            else
                ++result.changed;
            result.differences.push_back(std::move(difference));
        }

        template <typename T, typename Key>
        std::map<std::string, const T*> index(const std::vector<T>& items, Key key) {
            std::map<std::string, const T*> result;
            for (const auto& item : items)
                result.emplace(key(item), &item);
            return result;
        }

        template <typename T, typename Equal, typename Preview>
        void compare_registry(
            ComparisonResult& result,
            const std::map<std::string, const T*>& before,
            const std::map<std::string, const T*>& after,
            const std::string& category,
            Equal equal,
            Preview preview
        ) {
            for (const auto& [name, left] : before) {
                const auto found = after.find(name);
                if (found == after.end()) {
                    add(result, {name, category, DifferenceKind::Removed, {}, preview(*left), {}});
                } else if (!equal(*left, *found->second)) {
                    add(result,
                        {name, category, DifferenceKind::Changed, {category}, preview(*left), preview(*found->second)});
                }
            }
            for (const auto& [name, right] : after)
                if (!before.count(name))
                    add(result, {name, category, DifferenceKind::Added, {}, {}, preview(*right)});
        }
    } // namespace

    ComparisonResult compare(const Environment& before, const Environment& after, const ComparisonOptions& options) {
        ComparisonResult result;
        result.scope = options.scope;
        std::map<std::string, const ValueNode*> left;
        std::map<std::string, const ValueNode*> right;
        for (const auto& node : before.nodes.get_nodes())

            if (node)
                left.emplace(node->path.name, node.get());
        for (const auto& node : after.nodes.get_nodes())
            if (node)
                right.emplace(node->path.name, node.get());

        for (const auto& [path, old_node] : left) {
            const auto found = right.find(path);
            if (found == right.end()) {
                add(result, {path, "value", DifferenceKind::Removed, {}, display_node(*old_node), {}});
                continue;
            }
            const auto& new_node = *found->second;
            Difference difference{path, "value", DifferenceKind::Changed};
            field(
                difference, "type", old_node->dtype != new_node.dtype || old_node->value_dtype != new_node.value_dtype
            );
            field(
                difference,
                "shape",
                old_node->value && new_node.value && old_node->value->get_shape() != new_node.value->get_shape()
            );
            field(difference, "units", !same_units(*old_node, new_node));
            compare_value(*old_node, new_node, options, difference);
            if (options.scope == ComparisonScope::Full) {
                auto old_tags = old_node->tags;
                auto new_tags = new_node.tags;
                std::sort(old_tags.begin(), old_tags.end());
                std::sort(new_tags.begin(), new_tags.end());
                field(difference, "tags", old_tags != new_tags);
                field(difference, "metadata", !same_metadata(old_node->metadata, new_node.metadata));
                field(difference, "options", !same_options(*old_node, new_node));
                const auto old_origin = before[path].get_provenance();
                const auto new_origin = after[path].get_provenance();
                field(
                    difference,
                    "provenance",
                    old_origin.source_name != new_origin.source_name ||
                        old_origin.source_line != new_origin.source_line ||
                        old_origin.source_code != new_origin.source_code ||
                        old_origin.override_line != new_origin.override_line ||
                        old_origin.override_code != new_origin.override_code ||
                        !same_source(old_origin.source, new_origin.source) ||
                        !same_source(old_origin.override_source, new_origin.override_source)
                );
                field(
                    difference,
                    "settings",
                    old_node->constant != new_node.constant || old_node->override != new_node.override ||
                        old_node->condition != new_node.condition || old_node->format != new_node.format ||
                        old_node->table_path != new_node.table_path ||
                        old_node->table_column_index != new_node.table_column_index
                );
            }
            if (!difference.fields.empty()) {
                difference.before = display_node(*old_node);
                difference.after = display_node(new_node);
                add(result, std::move(difference));
            }
        }
        for (const auto& [path, new_node] : right)
            if (!left.count(path))
                add(result, {path, "value", DifferenceKind::Added, {}, {}, display_node(*new_node)});

        if (options.scope == ComparisonScope::Full) {
            const auto old_sources = before.get_source_manifest();
            const auto new_sources = after.get_source_manifest();
            compare_registry(
                result,
                index(old_sources, [](const SourceInfo& x) { return x.name; }),
                index(new_sources, [](const SourceInfo& x) { return x.name; }),
                "source",
                [](const SourceInfo& a, const SourceInfo& b) { return same_source(a, b); },
                [](const SourceInfo& x) { return x.path + " sha256=" + x.hash; }
            );

            const auto old_trace = before.get_trace_manifest();
            const auto new_trace = after.get_trace_manifest();
            compare_registry(
                result,
                index(old_trace, [](const TraceInfo& x) { return x.kind + ":" + x.name; }),
                index(new_trace, [](const TraceInfo& x) { return x.kind + ":" + x.name; }),
                "trace",
                [](const TraceInfo& a, const TraceInfo& b) {
                    return a.id == b.id && a.name == b.name && a.kind == b.kind;
                },
                [](const TraceInfo& x) { return x.id; }
            );

            const auto old_schema = before.get_schema_manifest();
            const auto new_schema = after.get_schema_manifest();
            compare_registry(
                result,
                index(old_schema, [](const SchemaInfo& x) { return x.name; }),
                index(new_schema, [](const SchemaInfo& x) { return x.name; }),
                "schema",
                [](const SchemaInfo& a, const SchemaInfo& b) {
                    return a.id == b.id && a.name == b.name && a.source_name == b.source_name &&
                           a.source_line == b.source_line && same_metadata(a.metadata, b.metadata) &&
                           same_source(a.source, b.source);
                },
                [](const SchemaInfo& x) { return x.id + " from " + x.source_name; }
            );
        }

        std::sort(result.differences.begin(), result.differences.end(), [](const Difference& a, const Difference& b) {
            return std::tie(a.category, a.path) < std::tie(b.category, b.path);
        });
        return result;
    }

    ComparisonResult compare_diph5(
        const std::filesystem::path& before, const std::filesystem::path& after, const ComparisonOptions& options
    ) {
        Environment old_env;
        Environment new_env;
        old_env.load(before);
        new_env.load(after);
        return compare(old_env, new_env, options);
    }
} // namespace snt::dip
