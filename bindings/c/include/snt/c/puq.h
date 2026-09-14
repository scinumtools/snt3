#ifndef SNT_C_PUQ_H
#define SNT_C_PUQ_H
#include <stddef.h>
#ifdef __cplusplus
extern "C" {
#endif
typedef struct snt_puq_quantity snt_puq_quantity;
typedef struct { int code; const char* message; } snt_c_error;
int snt_puq_eval(const char*, snt_puq_quantity**, snt_c_error*);
int snt_puq_convert(const snt_puq_quantity*, const char*, snt_puq_quantity**, snt_c_error*);
int snt_puq_format(const snt_puq_quantity*, char*, size_t, snt_c_error*);
void snt_puq_quantity_free(snt_puq_quantity*);
#ifdef __cplusplus
}
#endif
#endif
