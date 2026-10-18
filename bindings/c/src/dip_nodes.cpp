#include "dip_internal.h"
#include "../../../src/snt/dip/nodes/metadata_fields.h"

#include <algorithm>
#include <stdexcept>
#include <string>
#include <vector>

namespace {
using namespace snt::c::dip_detail;

const std::string& metadata_string(const snt::dip::ValueMetadata& metadata, const char* name) {
#define SNT_METADATA_STRING(TYPE, FIELD, NAME) if (std::strcmp(name, NAME) == 0) return metadata.FIELD;
    SNT_DIP_METADATA_SCALAR_FIELDS(SNT_METADATA_STRING)
#undef SNT_METADATA_STRING
    throw std::invalid_argument("unknown scalar metadata name");
}

snt::dip::PropertyType metadata_property(const char* name) {
    if (std::strcmp(name, "descr") == 0) return snt::dip::PropertyType::Description;
#define SNT_METADATA_PROPERTY(TYPE, FIELD, NAME) if (std::strcmp(name, NAME) == 0) return snt::dip::PropertyType::TYPE;
    SNT_DIP_METADATA_SCALAR_FIELDS(SNT_METADATA_PROPERTY)
    SNT_DIP_METADATA_LIST_FIELDS(SNT_METADATA_PROPERTY)
#undef SNT_METADATA_PROPERTY
    throw std::invalid_argument("unknown metadata name");
}

const std::vector<std::string>& metadata_list(const snt::dip::ValueMetadata& metadata, const char* name) {
#define SNT_METADATA_LIST(TYPE, FIELD, NAME) if (std::strcmp(name, NAME) == 0) return metadata.FIELD;
    SNT_DIP_METADATA_LIST_FIELDS(SNT_METADATA_LIST)
#undef SNT_METADATA_LIST
    throw std::invalid_argument("unknown list metadata name");
}

const snt::dip::ValueNode::OptionStruct& get_option(const snt_dip_node* node, size_t index) {
    const auto value_node = cursor(node).get_node();
    if (index >= value_node->options.size()) throw std::out_of_range("option index is out of range");
    return value_node->options[index];
}

template <typename T> T array_element(const snt_dip_node* node, size_t index) {
    const auto values = cursor(node).as<std::vector<T>>();
    if (index >= values.size()) throw std::out_of_range("value index is out of range");
    return values[index];
}
} // namespace

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

extern "C" int snt_dip_node_get_node_type(
    const snt_dip_node* node, char* buffer, size_t capacity, size_t* required, snt_dip_error* error
) {
    return node_call(error, [&] {
        const auto current = cursor(node);
        std::string type_name;
        try {
            const auto value_node = current.get_node();
            const auto type = snt::dip::NodeDtypeNames.find(value_node->dtype);
            type_name = type == snt::dip::NodeDtypeNames.end() ? "node" : type->second;
        } catch (const snt::dip::EnvironmentException&) {
            type_name = snt::dip::Path::KindNames.at(current.get_kind());
        }
        copy_string(type_name, buffer, capacity, required, error);
    });
}

extern "C" int snt_dip_node_get_link_target(
    const snt_dip_node* node,
    char* source, size_t source_capacity, size_t* source_required,
    char* target, size_t target_capacity, size_t* target_required,
    snt_dip_error* error
) {
    return node_call(error, [&] {
        const auto value_node = cursor(node).get_node();
        if (value_node->dtype != snt::dip::NodeDtype::Link)
            throw std::invalid_argument("node is not a link");
        copy_string(value_node->link_source, source, source_capacity, source_required, error);
        copy_string(value_node->link_target, target, target_capacity, target_required, error);
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

