#ifndef SNT_C_PUQ_H
#define SNT_C_PUQ_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Opaque handle for a parsed PUQ quantity. */
typedef struct snt_puq_quantity snt_puq_quantity;
/** Error code and diagnostic message returned by a PUQ ABI function. */
typedef struct {
    int code;
    const char* message;
} snt_puq_error;

/** Evaluate a PUEL quantity expression. */
int snt_puq_quantity_eval(const char* expression, snt_puq_quantity** result, snt_puq_error* error);
/** Convert a quantity to another unit expression. */
int snt_puq_quantity_convert(
    const snt_puq_quantity* quantity, const char* units, snt_puq_quantity** result, snt_puq_error* error
);
/** Format a quantity into a caller-provided buffer. */
int snt_puq_quantity_format(const snt_puq_quantity* quantity, char* buffer, size_t capacity, snt_puq_error* error);
/** Release a quantity returned by the C ABI. Accepts null. */
void snt_puq_quantity_free(snt_puq_quantity* quantity);

#ifdef __cplusplus
}
#endif

#endif
