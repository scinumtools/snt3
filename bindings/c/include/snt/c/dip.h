#ifndef SNT_C_DIP_H
#define SNT_C_DIP_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Opaque handle for a DIPL parser and environment. */
typedef struct snt_dip snt_dip;
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
/** Format a parsed DIPL value into a caller-provided buffer. */
int snt_dip_parser_get(const snt_dip* dip, const char* path, char* buffer, size_t capacity, snt_dip_error* error);
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
