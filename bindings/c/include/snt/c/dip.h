#ifndef SNT_C_DIP_H
#define SNT_C_DIP_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Opaque handle for a DIPL parser and environment. */
typedef struct snt_dip snt_dip;
/** Error code and diagnostic message returned by a DIP ABI function. */
typedef struct {
    int code;
    const char* message;
} snt_dip_error;

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
/** Generate a TeX or PDF report from a parsed or loaded environment.
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
