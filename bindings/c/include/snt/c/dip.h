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

/** Create a DIPL parser. */
int snt_dip_parser_create(snt_dip** result, snt_dip_error* error);
/** Add DIPL source text to a parser. */
int snt_dip_parser_add_string(snt_dip* dip, const char* source, snt_dip_error* error);
/** Add a DIPL source file to a parser. */
int snt_dip_parser_add_file(snt_dip* dip, const char* path, snt_dip_error* error);
/** Parse and evaluate all sources added to a parser. */
int snt_dip_parser_parse(snt_dip* dip, snt_dip_error* error);
/** Format a parsed DIPL value into a caller-provided buffer. */
int snt_dip_parser_get(const snt_dip* dip, const char* path, char* buffer, size_t capacity, snt_dip_error* error);
/** Release a DIPL parser. Accepts null. */
void snt_dip_parser_free(snt_dip* dip);

#ifdef __cplusplus
}
#endif

#endif
