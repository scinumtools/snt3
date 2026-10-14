#include "snt/c/dip.h"

#include <cstring>
#include <algorithm>
#include <stdexcept>
#include <string>
#include <vector>

#include <snt/dip/dip.h>
#include <snt/dip/cursor.h>
#include <snt/dip/report/report.h>

struct snt_dip {
    snt::dip::DIP parser;
    snt::dip::Environment env;
    bool parsed = false;
};

struct snt_dip_node {
    const snt_dip* owner;
    std::string path;
};

namespace {
thread_local std::string last_error;

int fail(snt_dip_error* error, const std::exception& exception) {
    last_error = exception.what();
    if (error) {
        error->code = 1;
        error->message = last_error.c_str();
    }
    return 1;
}

void ok(snt_dip_error* error) {
    if (error) {
        error->code = 0;
        error->message = nullptr;
    }
}

snt::dip::Cursor cursor(const snt_dip_node* node) {
    if (!node || !node->owner || !node->owner->parsed)
        throw std::invalid_argument("valid node and parsed environment are required");
    return snt::dip::Cursor(&node->owner->env, node->path);
}

int copy_string(const std::string& value, char* buffer, size_t capacity, size_t* required, snt_dip_error* error) {
    if (!required)
        throw std::invalid_argument("required-size output is required");
    *required = value.size() + 1;
    if (!buffer && capacity == 0) {
        ok(error);
        return 0;
    }
    if (!buffer || capacity < *required)
        throw std::invalid_argument("output buffer is too small");
    std::memcpy(buffer, value.c_str(), *required);
    ok(error);
    return 0;
}

const std::string& metadata_string(const snt::dip::ValueMetadata& metadata, const char* name) {
#define SNT_DIP_METADATA_CASE(key) if (std::strcmp(name, #key) == 0) return metadata.key
    SNT_DIP_METADATA_CASE(description); SNT_DIP_METADATA_CASE(authors); SNT_DIP_METADATA_CASE(title);
    SNT_DIP_METADATA_CASE(journal); SNT_DIP_METADATA_CASE(year); SNT_DIP_METADATA_CASE(volume);
    SNT_DIP_METADATA_CASE(issue); SNT_DIP_METADATA_CASE(pages); SNT_DIP_METADATA_CASE(doi);
    SNT_DIP_METADATA_CASE(url); SNT_DIP_METADATA_CASE(version); SNT_DIP_METADATA_CASE(created);
    SNT_DIP_METADATA_CASE(modified); SNT_DIP_METADATA_CASE(license); SNT_DIP_METADATA_CASE(rationale);
    SNT_DIP_METADATA_CASE(recommended_range); SNT_DIP_METADATA_CASE(performance_impact);
    SNT_DIP_METADATA_CASE(scientific_impact); SNT_DIP_METADATA_CASE(deprecated);
    SNT_DIP_METADATA_CASE(replacement); SNT_DIP_METADATA_CASE(since); SNT_DIP_METADATA_CASE(category);
    SNT_DIP_METADATA_CASE(visibility);
#undef SNT_DIP_METADATA_CASE
    throw std::invalid_argument("unknown scalar metadata name");
}

snt::dip::PropertyType metadata_property(const char* name) {
#define SNT_DIP_METADATA_PROPERTY(key, property) if (std::strcmp(name, #key) == 0) return snt::dip::PropertyType::property
    if (std::strcmp(name, "description") == 0 || std::strcmp(name, "descr") == 0)
        return snt::dip::PropertyType::Description;
    SNT_DIP_METADATA_PROPERTY(authors, Authors); SNT_DIP_METADATA_PROPERTY(title, Title);
    SNT_DIP_METADATA_PROPERTY(journal, Journal); SNT_DIP_METADATA_PROPERTY(year, Year);
    SNT_DIP_METADATA_PROPERTY(volume, Volume); SNT_DIP_METADATA_PROPERTY(issue, Issue);
    SNT_DIP_METADATA_PROPERTY(pages, Pages); SNT_DIP_METADATA_PROPERTY(doi, DOI);
    SNT_DIP_METADATA_PROPERTY(url, URL); SNT_DIP_METADATA_PROPERTY(version, Version);
    SNT_DIP_METADATA_PROPERTY(created, Created); SNT_DIP_METADATA_PROPERTY(modified, Modified);
    SNT_DIP_METADATA_PROPERTY(license, License); SNT_DIP_METADATA_PROPERTY(rationale, Rationale);
    SNT_DIP_METADATA_PROPERTY(native, Native); SNT_DIP_METADATA_PROPERTY(requires, Requires);
    SNT_DIP_METADATA_PROPERTY(conflicts, Conflicts); SNT_DIP_METADATA_PROPERTY(implies, Implies);
    SNT_DIP_METADATA_PROPERTY(see, See); SNT_DIP_METADATA_PROPERTY(example, Example);
    SNT_DIP_METADATA_PROPERTY(recommended_range, RecommendedRange);
    SNT_DIP_METADATA_PROPERTY(performance_impact, PerformanceImpact);
    SNT_DIP_METADATA_PROPERTY(scientific_impact, ScientificImpact);
    SNT_DIP_METADATA_PROPERTY(deprecated, Deprecated); SNT_DIP_METADATA_PROPERTY(replacement, Replacement);
    SNT_DIP_METADATA_PROPERTY(since, Since); SNT_DIP_METADATA_PROPERTY(category, Category);
    SNT_DIP_METADATA_PROPERTY(visibility, Visibility);
#undef SNT_DIP_METADATA_PROPERTY
    throw std::invalid_argument("unknown metadata name");
}

const std::vector<std::string>& metadata_list(const snt::dip::ValueMetadata& metadata, const char* name) {
#define SNT_DIP_METADATA_LIST_CASE(key) if (std::strcmp(name, #key) == 0) return metadata.key
    SNT_DIP_METADATA_LIST_CASE(native); SNT_DIP_METADATA_LIST_CASE(requires);
    SNT_DIP_METADATA_LIST_CASE(conflicts); SNT_DIP_METADATA_LIST_CASE(implies);
    SNT_DIP_METADATA_LIST_CASE(see); SNT_DIP_METADATA_LIST_CASE(example);
#undef SNT_DIP_METADATA_LIST_CASE
    throw std::invalid_argument("unknown list metadata name");
}

const snt::dip::ValueNode::OptionStruct& get_option(const snt_dip_node* node, size_t index) {
    const auto value_node = cursor(node).get_node();
    if (index >= value_node->options.size()) throw std::out_of_range("option index is out of range");
    return value_node->options[index];
}

template <typename F> int node_call(snt_dip_error* error, F&& function) {
    try { function(); ok(error); return 0; }
    catch (const std::exception& exception) { return fail(error, exception); }
}

template <typename T> T array_element(const snt_dip_node* node, size_t index) {
    const auto values = cursor(node).as<std::vector<T>>();
    if (index >= values.size()) throw std::out_of_range("value index is out of range");
    return values[index];
}

snt::dip::ExportFormat export_format(snt_dip_export_format format) {
    switch (format) {
    case SNT_DIP_EXPORT_CPP:
        return snt::dip::ExportFormat::CPP;
    case SNT_DIP_EXPORT_C:
        return snt::dip::ExportFormat::C;
    case SNT_DIP_EXPORT_FORTRAN:
        return snt::dip::ExportFormat::FORTRAN;
    case SNT_DIP_EXPORT_RUST:
        return snt::dip::ExportFormat::RUST;
    case SNT_DIP_EXPORT_R:
        return snt::dip::ExportFormat::R;
    case SNT_DIP_EXPORT_JULIA:
        return snt::dip::ExportFormat::JULIA;
    case SNT_DIP_EXPORT_JSON:
        return snt::dip::ExportFormat::JSON;
    case SNT_DIP_EXPORT_TOML:
        return snt::dip::ExportFormat::TOML;
    case SNT_DIP_EXPORT_YAML:
        return snt::dip::ExportFormat::YAML;
    default:
        throw std::invalid_argument("invalid DIP output format");
    }
}

snt::dip::report::ReportFormat report_format(snt_dip_report_format format) {
    switch (format) {
    case SNT_DIP_REPORT_TEX: return snt::dip::report::ReportFormat::Tex;
    case SNT_DIP_REPORT_PDF: return snt::dip::report::ReportFormat::Pdf;
    case SNT_DIP_REPORT_MD: return snt::dip::report::ReportFormat::Markdown;
    case SNT_DIP_REPORT_RST: return snt::dip::report::ReportFormat::Rst;
    case SNT_DIP_REPORT_HTML: return snt::dip::report::ReportFormat::Html;
    case SNT_DIP_REPORT_TYP: return snt::dip::report::ReportFormat::Typst;
    case SNT_DIP_REPORT_TXT: return snt::dip::report::ReportFormat::Text;
    case SNT_DIP_REPORT_JSON: return snt::dip::report::ReportFormat::Json;
    default: throw std::invalid_argument("invalid DIP report format");
    }
}
} // namespace

extern "C" int snt_dip_parser_create(snt_dip** result, snt_dip_error* error) {
    try {
        if (!result)
            throw std::invalid_argument("output is required");
        *result = new snt_dip;
        ok(error);
        return 0;
    } catch (const std::exception& exception) {
        return fail(error, exception);
    }
}

extern "C" int snt_dip_parser_add_string(snt_dip* dip, const char* source, snt_dip_error* error) {
    try {
        if (!dip || !source)
            throw std::invalid_argument("DIP and source text are required");
        dip->parser.add_string(source);
        ok(error);
        return 0;
    } catch (const std::exception& exception) {
        return fail(error, exception);
    }
}

extern "C" int snt_dip_parser_add_file(snt_dip* dip, const char* path, snt_dip_error* error) {
    try {
        if (!dip || !path)
            throw std::invalid_argument("DIP and filename are required");
        dip->parser.add_file(path);
        ok(error);
        return 0;
    } catch (const std::exception& exception) {
        return fail(error, exception);
    }
}

extern "C" int snt_dip_parser_add_override_string(snt_dip* dip, const char* source, snt_dip_error* error) {
    try {
        if (!dip || !source)
            throw std::invalid_argument("DIP and override text are required");
        dip->parser.add_override_string(source);
        ok(error);
        return 0;
    } catch (const std::exception& exception) {
        return fail(error, exception);
    }
}

extern "C" int snt_dip_parser_add_override_file(snt_dip* dip, const char* path, snt_dip_error* error) {
    try {
        if (!dip || !path)
            throw std::invalid_argument("DIP and override file path are required");
        dip->parser.add_override_file(path);
        ok(error);
        return 0;
    } catch (const std::exception& exception) {
        return fail(error, exception);
    }
}

extern "C" int snt_dip_parser_is_overridden(
    const snt_dip* dip, const char* path, int* result, snt_dip_error* error
) {
    try {
        if (!dip || !dip->parsed || !path || !result)
            throw std::invalid_argument("parsed DIP, path, and output are required");
        *result = dip->env.get_node(path)->override ? 1 : 0;
        ok(error);
        return 0;
    } catch (const std::exception& exception) {
        return fail(error, exception);
    }
}

extern "C" int snt_dip_parser_add_project(snt_dip* dip, const char* path, snt_dip_error* error) {
    try {
        if (!dip || !path)
            throw std::invalid_argument("DIP and project filename are required");
        dip->parser.add_project(path);
        ok(error);
        return 0;
    } catch (const std::exception& exception) {
        return fail(error, exception);
    }
}

extern "C" int snt_dip_parser_parse(snt_dip* dip, snt_dip_error* error) {
    return snt_dip_parser_parse_with_graph(dip, 0, error);
}

extern "C" int snt_dip_parser_parse_with_graph(snt_dip* dip, int record_dependency_graph, snt_dip_error* error) {
    try {
        if (!dip)
            throw std::invalid_argument("DIP is required");
        dip->env = dip->parser.parse(record_dependency_graph != 0);
        dip->parsed = true;
        ok(error);
        return 0;
    } catch (const std::exception& exception) {
        return fail(error, exception);
    }
}

extern "C" int snt_dip_parser_get(
    const snt_dip* dip, const char* path, char* buffer, size_t capacity, snt_dip_error* error
) {
    try {
        if (!dip || !dip->parsed || !path || !buffer || !capacity)
            throw std::invalid_argument("parsed DIP, path, buffer, and capacity are required");
        auto value = dip->env.request_node_data(std::string("?") + path);
        if (!value.value)
            throw std::runtime_error("DIPL path has no value");
        auto output = value.value->to_string();
        if (output.size() + 1 > capacity)
            throw std::invalid_argument("output buffer is too small");
        std::memcpy(buffer, output.c_str(), output.size() + 1);
        ok(error);
        return 0;
    } catch (const std::exception& exception) {
        return fail(error, exception);
    }
}

extern "C" int snt_dip_environment_get_node(
    const snt_dip* dip, const char* path, snt_dip_node** result, snt_dip_error* error
) {
    return node_call(error, [&] {
        if (!dip || !dip->parsed || !path || !result)
            throw std::invalid_argument("parsed DIP, path, and result are required");
        (void)snt::dip::Cursor(&dip->env, path);
        *result = new snt_dip_node{dip, path};
    });
}

extern "C" int snt_dip_node_get_kind(const snt_dip_node* node, snt_dip_node_kind* result, snt_dip_error* error) {
    return node_call(error, [&] {
        if (!result) throw std::invalid_argument("kind output is required");
        *result = static_cast<snt_dip_node_kind>(cursor(node).get_kind());
    });
}

extern "C" int snt_dip_node_get_path(
    const snt_dip_node* node, char* buffer, size_t capacity, size_t* required, snt_dip_error* error
) {
    return node_call(error, [&] { copy_string(cursor(node).get_path(), buffer, capacity, required, error); });
}

extern "C" int snt_dip_node_get_child(
    const snt_dip_node* node, const char* name, snt_dip_node** result, snt_dip_error* error
) {
    return node_call(error, [&] {
        if (!name || !result) throw std::invalid_argument("child name and result are required");
        const auto child = cursor(node)[name];
        *result = new snt_dip_node{node->owner, child.get_path()};
    });
}

extern "C" int snt_dip_node_get_item(
    const snt_dip_node* node, const char* key, snt_dip_node** result, snt_dip_error* error
) {
    return node_call(error, [&] {
        if (!key || !result) throw std::invalid_argument("item key and result are required");
        const auto entries = cursor(node).items();
        const auto found = entries.find(key);
        if (found == entries.end()) throw std::invalid_argument("map item was not found");
        *result = new snt_dip_node{node->owner, found->second.get_path()};
    });
}

extern "C" int snt_dip_node_get_element(
    const snt_dip_node* node, size_t index, snt_dip_node** result, snt_dip_error* error
) {
    return node_call(error, [&] {
        if (!result) throw std::invalid_argument("element result is required");
        const auto elements = cursor(node).elements();
        if (index >= elements.size()) throw std::out_of_range("list index is out of range");
        *result = new snt_dip_node{node->owner, elements[index].get_path()};
    });
}

extern "C" int snt_dip_node_get_key(
    const snt_dip_node* node, size_t index, char* buffer, size_t capacity, size_t* required, snt_dip_error* error
) {
    return node_call(error, [&] {
        const auto entries = cursor(node).items();
        std::vector<std::string> keys;
        keys.reserve(entries.size());
        for (const auto& [key, value] : entries) { (void)value; keys.push_back(key); }
        std::sort(keys.begin(), keys.end());
        if (index >= keys.size()) throw std::out_of_range("map key index is out of range");
        copy_string(keys[index], buffer, capacity, required, error);
    });
}

extern "C" int snt_dip_node_get_size(const snt_dip_node* node, size_t* result, snt_dip_error* error) {
    return node_call(error, [&] {
        if (!result) throw std::invalid_argument("size output is required");
        const auto current = cursor(node);
        switch (current.get_kind()) {
        case snt::dip::Path::Kind::Empty:
        case snt::dip::Path::Kind::Group:
        case snt::dip::Path::Kind::Item: *result = current.children().size(); break;
        case snt::dip::Path::Kind::Map: *result = current.items().size(); break;
        case snt::dip::Path::Kind::List: *result = current.elements().size(); break;
        default: throw std::invalid_argument("node is not a group, map, list, or item");
        }
    });
}

extern "C" int snt_dip_node_get_type(const snt_dip_node* node, snt_dip_value_type* result, snt_dip_error* error) {
    return node_call(error, [&] {
        if (!result) throw std::invalid_argument("type output is required");
        *result = static_cast<snt_dip_value_type>(snt::core::to_underlying(cursor(node).get_node()->value_dtype));
    });
}

extern "C" int snt_dip_node_get_shape(
    const snt_dip_node* node, size_t* dimensions, size_t capacity, size_t* rank, snt_dip_error* error
) {
    return node_call(error, [&] {
        if (!rank) throw std::invalid_argument("shape rank output is required");
        const auto shape = cursor(node).get_shape();
        *rank = shape.size();
        if (!dimensions && capacity == 0) return;
        if (!dimensions || capacity < shape.size()) throw std::invalid_argument("shape buffer is too small");
        std::copy(shape.begin(), shape.end(), dimensions);
    });
}

extern "C" int snt_dip_node_get_value_size(const snt_dip_node* node, size_t* result, snt_dip_error* error) {
    return node_call(error, [&] {
        if (!result) throw std::invalid_argument("value-size output is required");
        const auto value = cursor(node).get_value();
        if (!value) throw std::invalid_argument("node has no scalar or array value");
        *result = value->get_size();
    });
}

extern "C" int snt_dip_node_get_bool_at(const snt_dip_node* node, size_t index, int* result, snt_dip_error* error) {
    return node_call(error, [&] {
        if (!result) throw std::invalid_argument("value output is required");
        *result = array_element<bool>(node, index) ? 1 : 0;
    });
}

extern "C" int snt_dip_node_get_int64_at(
    const snt_dip_node* node, size_t index, int64_t* result, snt_dip_error* error
) {
    return node_call(error, [&] {
        if (!result) throw std::invalid_argument("value output is required");
        *result = array_element<int64_t>(node, index);
    });
}

extern "C" int snt_dip_node_get_uint64_at(
    const snt_dip_node* node, size_t index, uint64_t* result, snt_dip_error* error
) {
    return node_call(error, [&] {
        if (!result) throw std::invalid_argument("value output is required");
        *result = array_element<uint64_t>(node, index);
    });
}

extern "C" int snt_dip_node_get_double_at(
    const snt_dip_node* node, size_t index, double* result, snt_dip_error* error
) {
    return node_call(error, [&] {
        if (!result) throw std::invalid_argument("value output is required");
        *result = array_element<double>(node, index);
    });
}

extern "C" int snt_dip_node_get_string_at(
    const snt_dip_node* node, size_t index, char* buffer, size_t capacity, size_t* required, snt_dip_error* error
) {
    return node_call(error, [&] { copy_string(array_element<std::string>(node, index), buffer, capacity, required, error); });
}

extern "C" int snt_dip_node_get_provenance(
    const snt_dip_node* node, char* source_name, size_t source_name_capacity, size_t* source_name_required,
    size_t* source_line, char* source_code, size_t source_code_capacity, size_t* source_code_required,
    char* override_source_name, size_t override_source_name_capacity, size_t* override_source_name_required,
    size_t* override_line, char* override_code, size_t override_code_capacity, size_t* override_code_required,
    snt_dip_error* error
) {
    return node_call(error, [&] {
        if (!source_line || !override_line) throw std::invalid_argument("provenance line outputs are required");
        const auto provenance = cursor(node).get_provenance();
        *source_line = provenance.source_line;
        *override_line = provenance.override_line;
        copy_string(provenance.source_name, source_name, source_name_capacity, source_name_required, error);
        copy_string(provenance.source_code, source_code, source_code_capacity, source_code_required, error);
        copy_string(provenance.override_source ? provenance.override_source->name : std::string{},
                    override_source_name, override_source_name_capacity, override_source_name_required, error);
        copy_string(provenance.override_code, override_code, override_code_capacity, override_code_required, error);
    });
}

extern "C" int snt_dip_node_get_metadata(
    const snt_dip_node* node, const char* name, char* buffer, size_t capacity, size_t* required, snt_dip_error* error
) {
    return node_call(error, [&] {
        if (!name) throw std::invalid_argument("metadata name is required");
        copy_string(metadata_string(cursor(node).get_metadata(), name), buffer, capacity, required, error);
    });
}

extern "C" int snt_dip_node_has_metadata(
    const snt_dip_node* node, const char* name, int* result, snt_dip_error* error
) {
    return node_call(error, [&] {
        if (!name || !result) throw std::invalid_argument("metadata name and presence output are required");
        *result = cursor(node).get_metadata().has_property(metadata_property(name)) ? 1 : 0;
    });
}

extern "C" int snt_dip_node_get_metadata_list_size(
    const snt_dip_node* node, const char* name, size_t* result, snt_dip_error* error
) {
    return node_call(error, [&] {
        if (!name || !result) throw std::invalid_argument("metadata name and size output are required");
        *result = metadata_list(cursor(node).get_metadata(), name).size();
    });
}

extern "C" int snt_dip_node_get_metadata_list_item(
    const snt_dip_node* node, const char* name, size_t index, char* buffer, size_t capacity,
    size_t* required, snt_dip_error* error
) {
    return node_call(error, [&] {
        if (!name) throw std::invalid_argument("metadata name is required");
        const auto values = metadata_list(cursor(node).get_metadata(), name);
        if (index >= values.size()) throw std::out_of_range("metadata index is out of range");
        copy_string(values[index], buffer, capacity, required, error);
    });
}

extern "C" int snt_dip_node_has_units(const snt_dip_node* node, int* result, snt_dip_error* error) {
    return node_call(error, [&] {
        if (!result) throw std::invalid_argument("units output is required");
        *result = cursor(node).get_units().has_value() ? 1 : 0;
    });
}

extern "C" int snt_dip_node_get_units(
    const snt_dip_node* node, char* buffer, size_t capacity, size_t* required, snt_dip_error* error
) {
    return node_call(error, [&] {
        const auto units = cursor(node).get_units();
        const std::string formatted = units ? units->to_string(snt::puq::UnitFormat(snt::puq::Format::Display::UNITS)) : "";
        copy_string(formatted, buffer, capacity, required, error);
    });
}

extern "C" int snt_dip_node_is_overridden(const snt_dip_node* node, int* result, snt_dip_error* error) {
    return node_call(error, [&] {
        if (!result) throw std::invalid_argument("override output is required");
        *result = cursor(node).get_node()->override ? 1 : 0;
    });
}

extern "C" int snt_dip_node_get_option_count(const snt_dip_node* node, size_t* result, snt_dip_error* error) {
    return node_call(error, [&] {
        if (!result) throw std::invalid_argument("option-count output is required");
        *result = cursor(node).get_node()->options.size();
    });
}

extern "C" int snt_dip_node_get_option_value(
    const snt_dip_node* node, size_t index, char* buffer, size_t capacity, size_t* required, snt_dip_error* error
) {
    return node_call(error, [&] {
        const auto& value = get_option(node, index).value;
        if (!value) throw std::runtime_error("option has no value");
        copy_string(value->to_string(), buffer, capacity, required, error);
    });
}

extern "C" int snt_dip_node_get_option_label(
    const snt_dip_node* node, size_t index, char* buffer, size_t capacity, size_t* required, snt_dip_error* error
) {
    return node_call(error, [&] { copy_string(get_option(node, index).label, buffer, capacity, required, error); });
}

extern "C" int snt_dip_node_get_option_units(
    const snt_dip_node* node, size_t index, char* buffer, size_t capacity, size_t* required, snt_dip_error* error
) {
    return node_call(error, [&] { copy_string(get_option(node, index).units_raw, buffer, capacity, required, error); });
}

extern "C" int snt_dip_node_get_bool(const snt_dip_node* node, int* result, snt_dip_error* error) {
    return node_call(error, [&] { if (!result) throw std::invalid_argument("value output is required"); *result = cursor(node).as<bool>() ? 1 : 0; });
}
extern "C" int snt_dip_node_get_int64(const snt_dip_node* node, int64_t* result, snt_dip_error* error) {
    return node_call(error, [&] { if (!result) throw std::invalid_argument("value output is required"); *result = cursor(node).as<int64_t>(); });
}
extern "C" int snt_dip_node_get_uint64(const snt_dip_node* node, uint64_t* result, snt_dip_error* error) {
    return node_call(error, [&] { if (!result) throw std::invalid_argument("value output is required"); *result = cursor(node).as<uint64_t>(); });
}
extern "C" int snt_dip_node_get_double(const snt_dip_node* node, double* result, snt_dip_error* error) {
    return node_call(error, [&] { if (!result) throw std::invalid_argument("value output is required"); *result = cursor(node).as<double>(); });
}
extern "C" int snt_dip_node_get_string(
    const snt_dip_node* node, char* buffer, size_t capacity, size_t* required, snt_dip_error* error
) {
    return node_call(error, [&] { copy_string(cursor(node).as<std::string>(), buffer, capacity, required, error); });
}
extern "C" void snt_dip_node_free(snt_dip_node* node) { delete node; }

extern "C" int snt_dip_environment_load(snt_dip* dip, const char* path, snt_dip_error* error) {
    try {
        if (!dip || !path)
            throw std::invalid_argument("DIP and filename are required");
        dip->env.load(path);
        dip->parsed = true;
        ok(error);
        return 0;
    } catch (const std::exception& exception) {
        return fail(error, exception);
    }
}

extern "C" int snt_dip_environment_save(snt_dip* dip, const char* path, snt_dip_error* error) {
    try {
        if (!dip || !path)
            throw std::invalid_argument("DIP and filename are required");
        dip->env.save(path);
        ok(error);
        return 0;
    } catch (const std::exception& exception) {
        return fail(error, exception);
    }
}

extern "C" int snt_dip_environment_generate(
    snt_dip* dip, snt_dip_export_format format, const char* path, snt_dip_error* error
) {
    try {
        if (!dip || !path)
            throw std::invalid_argument("DIP and filename are required");
        dip->env.generate(export_format(format), path);
        ok(error);
        return 0;
    } catch (const std::exception& exception) {
        return fail(error, exception);
    }
}

extern "C" int snt_dip_environment_generate_report(
    snt_dip* dip, snt_dip_report_format format, const char* path,
    const char* input_label, const char* intro_file, const char* tex_compiler,
    snt_dip_error* error
) {
    const snt_dip_report_options options{input_label, intro_file, tex_compiler, nullptr, nullptr, nullptr, nullptr};
    return snt_dip_environment_generate_report_with_options(dip, format, path, &options, error);
}

extern "C" int snt_dip_environment_generate_report_with_options(
    snt_dip* dip, snt_dip_report_format format, const char* path,
    const snt_dip_report_options* cover, snt_dip_error* error
) {
    try {
        if (!dip || !dip->parsed || !path || !*path)
            throw std::invalid_argument("parsed DIP and output path are required");
        snt::dip::report::ReportOptions options;
        if (cover) {
            if (cover->input_label) options.input_label = cover->input_label;
            if (cover->intro_file) options.introduction_file = cover->intro_file;
            if (cover->tex_compiler) options.tex_compiler = cover->tex_compiler;
            if (cover->title) options.title = cover->title;
            if (cover->author) options.author = cover->author;
            if (cover->date) options.date = cover->date;
            if (cover->version) options.version = cover->version;
        }
        snt::dip::report::generate(dip->env, report_format(format), path, options);
        ok(error);
        return 0;
    } catch (const std::exception& exception) {
        return fail(error, exception);
    }
}

extern "C" void snt_dip_parser_free(snt_dip* dip) {
    delete dip;
}

extern "C" int snt_dip_parser_add_schema_string(snt_dip* dip, const char* name, const char* source, snt_dip_error* error) {
    try {
        if (!dip || !name || !source)
            throw std::invalid_argument("DIP, schema name, and schema input are required");
        dip->parser.add_schema_string(name, source);
        ok(error);
        return 0;
    } catch (const std::exception& exception) {
        return fail(error, exception);
    }
}

extern "C" int snt_dip_parser_add_schema_file(snt_dip* dip, const char* name, const char* path, snt_dip_error* error) {
    try {
        if (!dip || !name || !path)
            throw std::invalid_argument("DIP, schema name, and schema input are required");
        dip->parser.add_schema_file(name, path);
        ok(error);
        return 0;
    } catch (const std::exception& exception) {
        return fail(error, exception);
    }
}
