#ifndef SNT_C_H
#define SNT_C_H
#include <stddef.h>
#ifdef __cplusplus
extern "C" {
#endif
typedef struct snt_quantity snt_quantity;
typedef struct snt_dip snt_dip;
typedef struct { int code; const char* message; } snt_error;
int snt_quantity_eval(const char*, snt_quantity**, snt_error*);
int snt_quantity_convert(const snt_quantity*, const char*, snt_quantity**, snt_error*);
int snt_quantity_format(const snt_quantity*, char*, size_t, snt_error*);
void snt_quantity_free(snt_quantity*);
int snt_dip_create(snt_dip**, snt_error*);
int snt_dip_add_string(snt_dip*, const char*, snt_error*);
int snt_dip_add_file(snt_dip*, const char*, snt_error*);
int snt_dip_parse(snt_dip*, snt_error*);
int snt_dip_get(const snt_dip*, const char*, char*, size_t, snt_error*);
void snt_dip_free(snt_dip*);
#ifdef __cplusplus
}
#endif
#endif
