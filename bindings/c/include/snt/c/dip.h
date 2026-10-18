#ifndef SNT_C_DIP_H
#define SNT_C_DIP_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Opaque handle for a DIPL parser and environment. */
typedef struct snt_dip snt_dip;
/** Opaque read-only node view. It is owned by the caller and borrows its parser. */
typedef struct snt_dip_node snt_dip_node;
/** Opaque, owned result of comparing two DIPH5 files. */
typedef struct snt_dip_comparison snt_dip_comparison;
/** Error code and diagnostic message returned by a DIP ABI function. */
typedef struct {
    int code;
    const char* message;
} snt_dip_error;

typedef enum {
    SNT_DIP_COMPARE_EFFECTIVE,
    SNT_DIP_COMPARE_FULL,
} snt_dip_compare_scope;

typedef enum {
    SNT_DIP_DIFFERENCE_ADDED,
    SNT_DIP_DIFFERENCE_REMOVED,
    SNT_DIP_DIFFERENCE_CHANGED,
} snt_dip_difference_kind;

/** DIPL path/node kinds exposed by the typed traversal API. */
typedef enum {
    SNT_DIP_NODE_NONE, SNT_DIP_NODE_EMPTY, SNT_DIP_NODE_GROUP, SNT_DIP_NODE_MAP,
    SNT_DIP_NODE_LIST, SNT_DIP_NODE_ITEM, SNT_DIP_NODE_ROOT
} snt_dip_node_kind;

/** Primitive DIPL value type. Values match SNT's stable primitive type codes. */
typedef enum {
    SNT_DIP_VALUE_NONE = 0, SNT_DIP_VALUE_BOOL = 1, SNT_DIP_VALUE_STRING = 2, SNT_DIP_VALUE_CHAR = 4,
    SNT_DIP_VALUE_INT16 = 8, SNT_DIP_VALUE_INT32 = 16, SNT_DIP_VALUE_INT64 = 32,
    SNT_DIP_VALUE_UINT16 = 128, SNT_DIP_VALUE_UINT32 = 256, SNT_DIP_VALUE_UINT64 = 512,
    SNT_DIP_VALUE_FLOAT32 = 1024, SNT_DIP_VALUE_FLOAT64 = 2048, SNT_DIP_VALUE_FLOAT128 = 4096,
    SNT_DIP_VALUE_INT8 = 16384, SNT_DIP_VALUE_UINT8 = 32768,
    SNT_DIP_VALUE_INTX = 64, SNT_DIP_VALUE_FLOATX = 8192
} snt_dip_value_type;

/** String and index pointers remain valid until the comparison is freed. */
typedef struct {
    const char* path;
    const char* category;
    snt_dip_difference_kind kind;
    const char* fields; ///< Comma-separated changed fields.
    const char* before;
    const char* after;
    size_t changed_elements;
    const size_t* example_indices;
    size_t example_count;
} snt_dip_difference;

/** Load and compare two DIPH5 files. The caller owns *result. */
int snt_dip_compare_files(
    const char* before, const char* after, snt_dip_compare_scope scope,
    size_t max_array_examples, snt_dip_comparison** result, snt_dip_error* error
);
/** Retrieve total added, removed, and changed entries. */
int snt_dip_comparison_summary(
    const snt_dip_comparison* result, size_t* added, size_t* removed, size_t* changed, snt_dip_error* error
);
/** Return the number of indexed differences; zero for a comparison with no differences. */
size_t snt_dip_comparison_count(const snt_dip_comparison* result);
/** Get one difference; string and index pointers are borrowed from result. */
int snt_dip_comparison_get(
    const snt_dip_comparison* result, size_t index, snt_dip_difference* difference, snt_dip_error* error
);
/** Render text; call with buffer=NULL and capacity=0 to query required bytes including NUL. */
int snt_dip_comparison_render_text(
    const snt_dip_comparison* result, size_t max_details, char* buffer, size_t capacity,
    size_t* required, snt_dip_error* error
);
void snt_dip_comparison_free(snt_dip_comparison* result);

/** Output format for generated parameter lists. */
typedef enum {
    SNT_DIP_EXPORT_CPP,
    SNT_DIP_EXPORT_C,
    SNT_DIP_EXPORT_FORTRAN,
    SNT_DIP_EXPORT_RUST,
    SNT_DIP_EXPORT_R,
    SNT_DIP_EXPORT_JULIA,
    SNT_DIP_EXPORT_JSON,
    SNT_DIP_EXPORT_TOML,
    SNT_DIP_EXPORT_YAML,
} snt_dip_export_format;

/** Output format for a generated DIP report. */
typedef enum {
    SNT_DIP_REPORT_TEX,
    SNT_DIP_REPORT_PDF,
    SNT_DIP_REPORT_MD,
    SNT_DIP_REPORT_RST,
    SNT_DIP_REPORT_HTML,
    SNT_DIP_REPORT_TYP,
    SNT_DIP_REPORT_TXT,
    SNT_DIP_REPORT_JSON,
} snt_dip_report_format;

/** Optional report settings. Null fields use the documented defaults. */
typedef struct {
    const char* input_label;
    const char* intro_file;
    const char* tex_compiler;
    const char* title;
    const char* author;
    const char* date;
    const char* version;
} snt_dip_report_options;

/** Create a DIPL parser. */
int snt_dip_parser_create(snt_dip** result, snt_dip_error* error);
/** Register a named schema body from a string. */
int snt_dip_parser_add_schema_string(snt_dip* dip, const char* name, const char* source, snt_dip_error* error);
/** Register a named schema body from a file. */
int snt_dip_parser_add_schema_file(snt_dip* dip, const char* name, const char* path, snt_dip_error* error);
/** Collect an unwrapped override body of value modifications and optional nested path prefixes. */
int snt_dip_parser_add_override_string(snt_dip* dip, const char* source, snt_dip_error* error);
/** Register an unwrapped override body from a file, retaining its source path. */
int snt_dip_parser_add_override_file(snt_dip* dip, const char* path, snt_dip_error* error);
/** Add DIPL source text to a parser. */
int snt_dip_parser_add_string(snt_dip* dip, const char* source, snt_dip_error* error);
/** Add a DIPL source file to a parser. */
int snt_dip_parser_add_file(snt_dip* dip, const char* path, snt_dip_error* error);
/** Add a DIPfile project manifest to a parser. */
int snt_dip_parser_add_project(snt_dip* dip, const char* path, snt_dip_error* error);
/** Parse and evaluate all sources added to a parser. */
int snt_dip_parser_parse(snt_dip* dip, snt_dip_error* error);
/** Parse and optionally record evaluation dependencies (nonzero enables recording). */
int snt_dip_parser_parse_with_graph(snt_dip* dip, int record_dependency_graph, snt_dip_error* error);
/** Format a parsed DIPL value into a caller-provided buffer. */
int snt_dip_parser_get(const snt_dip* dip, const char* path, char* buffer, size_t capacity, snt_dip_error* error);
/** Get a read-only node handle. It resolves its path against dip's current environment. */
int snt_dip_environment_get_node(const snt_dip* dip, const char* path, snt_dip_node** result, snt_dip_error* error);
/** Return the node kind. */
int snt_dip_node_get_kind(const snt_dip_node* node, snt_dip_node_kind* result, snt_dip_error* error);
/** Copy the DIPL node type name (for example, `integer`, `string`, or `link`). */
int snt_dip_node_get_node_type(const snt_dip_node* node, char* buffer, size_t capacity, size_t* required, snt_dip_error* error);
/** Copy a link's named source and target path. The source is empty for a local link. */
int snt_dip_node_get_link_target(
    const snt_dip_node* node,
    char* source, size_t source_capacity, size_t* source_required,
    char* target, size_t target_capacity, size_t* target_required,
    snt_dip_error* error
);
/** Copy the UTF-8 path into a caller buffer; query required bytes (including NUL) with NULL/0. */
int snt_dip_node_get_path(const snt_dip_node* node, char* buffer, size_t capacity, size_t* required, snt_dip_error* error);
/** Return direct group child by name, map item by key, or list element by index. Caller owns result. */
int snt_dip_node_get_child(const snt_dip_node* node, const char* name, snt_dip_node** result, snt_dip_error* error);
int snt_dip_node_get_item(const snt_dip_node* node, const char* key, snt_dip_node** result, snt_dip_error* error);
/** Copy a map key by zero-based position in sorted key order. */
int snt_dip_node_get_key(const snt_dip_node* node, size_t index, char* buffer, size_t capacity, size_t* required, snt_dip_error* error);
int snt_dip_node_get_element(const snt_dip_node* node, size_t index, snt_dip_node** result, snt_dip_error* error);
/** Return direct child, map item, or list element count. */
int snt_dip_node_get_size(const snt_dip_node* node, size_t* result, snt_dip_error* error);
int snt_dip_node_get_type(const snt_dip_node* node, snt_dip_value_type* result, snt_dip_error* error);
/** Query shape rank with dimensions=NULL/capacity=0, then fill dimensions. */
int snt_dip_node_get_shape(const snt_dip_node* node, size_t* dimensions, size_t capacity, size_t* rank, snt_dip_error* error);
/** Return the flattened scalar/array element count. */
int snt_dip_node_get_value_size(const snt_dip_node* node, size_t* result, snt_dip_error* error);
int snt_dip_node_get_bool_at(const snt_dip_node* node, size_t index, int* result, snt_dip_error* error);
int snt_dip_node_get_int64_at(const snt_dip_node* node, size_t index, int64_t* result, snt_dip_error* error);
int snt_dip_node_get_uint64_at(const snt_dip_node* node, size_t index, uint64_t* result, snt_dip_error* error);
int snt_dip_node_get_double_at(const snt_dip_node* node, size_t index, double* result, snt_dip_error* error);
int snt_dip_node_get_string_at(const snt_dip_node* node, size_t index, char* buffer, size_t capacity, size_t* required, snt_dip_error* error);
/** Copy provenance text fields; source_name and override_source_name may be empty. */
int snt_dip_node_get_provenance(const snt_dip_node* node, char* source_name, size_t source_name_capacity, size_t* source_name_required,
    size_t* source_line, char* source_code, size_t source_code_capacity, size_t* source_code_required,
    char* override_source_name, size_t override_source_name_capacity, size_t* override_source_name_required,
    size_t* override_line, char* override_code, size_t override_code_capacity, size_t* override_code_required,
    snt_dip_error* error);
/** Read scalar metadata by DIPL name. Use snt_dip_node_has_metadata to test presence. */
int snt_dip_node_get_metadata(const snt_dip_node* node, const char* name, char* buffer, size_t capacity, size_t* required, snt_dip_error* error);
int snt_dip_node_has_metadata(const snt_dip_node* node, const char* name, int* result, snt_dip_error* error);
/** Read list metadata such as native, requires, conflicts, implies, see, or example. */
int snt_dip_node_get_metadata_list_size(const snt_dip_node* node, const char* name, size_t* result, snt_dip_error* error);
int snt_dip_node_get_metadata_list_item(const snt_dip_node* node, const char* name, size_t index, char* buffer, size_t capacity, size_t* required, snt_dip_error* error);
/** Report whether this value has physical units. */
int snt_dip_node_has_units(const snt_dip_node* node, int* result, snt_dip_error* error);
/** Copy the formatted unit expression; returns an empty string when the node is unitless. */
int snt_dip_node_get_units(const snt_dip_node* node, char* buffer, size_t capacity, size_t* required, snt_dip_error* error);
int snt_dip_node_is_overridden(const snt_dip_node* node, int* result, snt_dip_error* error);
int snt_dip_node_get_option_count(const snt_dip_node* node, size_t* result, snt_dip_error* error);
/** Option values use DIPL formatting. Option label and units are empty when unspecified. */
int snt_dip_node_get_option_value(const snt_dip_node* node, size_t index, char* buffer, size_t capacity, size_t* required, snt_dip_error* error);
int snt_dip_node_get_option_label(const snt_dip_node* node, size_t index, char* buffer, size_t capacity, size_t* required, snt_dip_error* error);
int snt_dip_node_get_option_units(const snt_dip_node* node, size_t index, char* buffer, size_t capacity, size_t* required, snt_dip_error* error);
int snt_dip_node_get_bool(const snt_dip_node* node, int* result, snt_dip_error* error);
int snt_dip_node_get_int64(const snt_dip_node* node, int64_t* result, snt_dip_error* error);
int snt_dip_node_get_uint64(const snt_dip_node* node, uint64_t* result, snt_dip_error* error);
int snt_dip_node_get_double(const snt_dip_node* node, double* result, snt_dip_error* error);
/** Copy the UTF-8 scalar string; query required bytes (including NUL) with NULL/0. */
int snt_dip_node_get_string(const snt_dip_node* node, char* buffer, size_t capacity, size_t* required, snt_dip_error* error);
void snt_dip_node_free(snt_dip_node* node);
/** Set result to 1 when the parsed or loaded value was explicitly overridden, otherwise 0. */
int snt_dip_parser_is_overridden(const snt_dip* dip, const char* path, int* result, snt_dip_error* error);
/** Load the environment from an HDF5 file. */
int snt_dip_environment_load(snt_dip* dip, const char* path, snt_dip_error* error);
/** Save the environment to an HDF5 file. */
int snt_dip_environment_save(snt_dip* dip, const char* path, snt_dip_error* error);
/** Generate a static parameter list from the environment. */
int snt_dip_environment_generate(
    snt_dip* dip, snt_dip_export_format format, const char* path, snt_dip_error* error
);
/** Generate a report from a parsed or loaded environment.
 * input_label, intro_file, and tex_compiler may be null. The default compiler is pdflatex.
 */
int snt_dip_environment_generate_report(
    snt_dip* dip, snt_dip_report_format format, const char* path,
    const char* input_label, const char* intro_file, const char* tex_compiler,
    snt_dip_error* error
);
/** Generate a report with optional cover metadata. options may be null. */
int snt_dip_environment_generate_report_with_options(
    snt_dip* dip, snt_dip_report_format format, const char* path,
    const snt_dip_report_options* options, snt_dip_error* error
);
/** Release a DIPL parser. Accepts null. */
void snt_dip_parser_free(snt_dip* dip);

#ifdef __cplusplus
}
#endif

#endif
