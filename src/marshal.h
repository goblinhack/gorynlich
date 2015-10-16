/*
 * Copyright (C) 2011-2015 goblinhack@gmail.com
 *
 * See the README file for license info for license.
 */

#pragma once

#include "tree.h"

demarshal_p demarshal(const char *filename);
void demarshal_print(demarshal_p);
void demarshal_fini(demarshal_p);
uint8_t demarshal_gotone(demarshal_p);
uint8_t demarshal_name(demarshal_p, const char *want);
uint8_t demarshal_peek_name(demarshal_p, const char *want);

uint8_t demarshal_string(demarshal_p, char **out);
uint8_t demarshal_named_string(demarshal_p, const char *name, char **out);
uint8_t demarshal_opt_named_string(demarshal_p, const char *name, char **out);
uint8_t demarshal_opt_def_named_string(demarshal_p, const char *name, char **out, const char *def);
uint8_t demarshal_peek_string(demarshal_p, char **out);

uint8_t demarshal_uint8(demarshal_p, uint8_t *out);
uint8_t demarshal_named_uint8(demarshal_p, const char *name, uint8_t *out);
uint8_t demarshal_opt_named_uint8(demarshal_p, const char *name, uint8_t *out);
uint8_t demarshal_opt_def_named_uint8(demarshal_p, const char *name, uint8_t *out, uint8_t def);

uint8_t demarshal_uint16(demarshal_p, uint16_t *out);
uint8_t demarshal_named_uint16(demarshal_p, const char *name, uint16_t *out);
uint8_t demarshal_opt_named_uint16(demarshal_p, const char *name, uint16_t *out);
uint8_t demarshal_opt_def_named_uint16(demarshal_p, const char *name, uint16_t *out, uint16_t def);

uint8_t demarshal_uint32(demarshal_p, uint32_t *out);
uint8_t demarshal_named_uint32(demarshal_p, const char *name, uint32_t *out);
uint8_t demarshal_opt_named_uint32(demarshal_p, const char *name, uint32_t *out);
uint8_t demarshal_opt_def_named_uint32(demarshal_p, const char *name, uint32_t *out, uint32_t def);

uint8_t demarshal_uint64(demarshal_p, uint64_t *out);
uint8_t demarshal_named_uint64(demarshal_p, const char *name, uint64_t *out);
uint8_t demarshal_opt_named_uint64(demarshal_p, const char *name, uint64_t *out);
uint8_t demarshal_opt_def_named_uint64(demarshal_p, const char *name, uint64_t *out, uint64_t def);
uint8_t demarshal_peek_int(demarshal_p);

uint8_t demarshal_int8(demarshal_p, int8_t *out);
uint8_t demarshal_named_int8(demarshal_p, const char *name, int8_t *out);
uint8_t demarshal_opt_named_int8(demarshal_p, const char *name, int8_t *out);
uint8_t demarshal_opt_def_named_int8(demarshal_p, const char *name, int8_t *out, int8_t def);

uint8_t demarshal_int16(demarshal_p, int16_t *out);
uint8_t demarshal_named_int16(demarshal_p, const char *name, int16_t *out);
uint8_t demarshal_opt_named_int16(demarshal_p, const char *name, int16_t *out);
uint8_t demarshal_opt_def_named_int16(demarshal_p, const char *name, int16_t *out, int16_t def);

uint8_t demarshal_int32(demarshal_p, int32_t *out);
uint8_t demarshal_named_int32(demarshal_p, const char *name, int32_t *out);
uint8_t demarshal_opt_named_int32(demarshal_p, const char *name, int32_t *out);
uint8_t demarshal_opt_def_named_int32(demarshal_p, const char *name, int32_t *out, int32_t def);

uint8_t demarshal_int64(demarshal_p, int64_t *out);
uint8_t demarshal_named_int64(demarshal_p, const char *name, int64_t *out);
uint8_t demarshal_opt_named_int64(demarshal_p, const char *name, int64_t *out);
uint8_t demarshal_opt_def_named_int64(demarshal_p, const char *name, int64_t *out, int64_t def);

uint8_t demarshal_float(demarshal_p, float *out);
uint8_t demarshal_named_float(demarshal_p, const char *name, float *out);
uint8_t demarshal_opt_named_float(demarshal_p, const char *name, float *out);
uint8_t demarshal_opt_def_named_float(demarshal_p, const char *name, float *out, float def);
uint8_t demarshal_peek_float(demarshal_p);

uint8_t demarshal_ptr(demarshal_p, void **out);
uint8_t demarshal_named_ptr(demarshal_p, const char *name, void **out);
uint8_t demarshal_opt_named_ptr(demarshal_p, const char *name, void **out);
uint8_t demarshal_peek_ptr(demarshal_p, void **out);

uint8_t demarshal_ptr_ref(demarshal_p, void *out);
uint8_t demarshal_named_ptr_ref(demarshal_p, const char *name, void **out);
uint8_t demarshal_opt_named_ptr_ref(demarshal_p, const char *name, void **out);
uint8_t demarshal_peek_ptr_ref(demarshal_p, void **out);

uint8_t demarshal_bra(demarshal_p);
uint8_t demarshal_peek_bra(demarshal_p);
uint8_t demarshal_ket(demarshal_p);

marshal_p marshal(const char *filename);
void marshal_print(marshal_p);
int marshal_fini(marshal_p);
uint8_t marshal_name(marshal_p, const char *name);
uint8_t marshal_name_only(marshal_p, const char *name);
uint8_t marshal_string(marshal_p, const char *out);
uint8_t marshal_named_string(marshal_p, const char *name, const char *out);

uint8_t marshal_uint8(marshal_p, uint8_t out);
uint8_t marshal_named_uint8(marshal_p, const char *name, uint8_t out);
uint8_t marshal_uint16(marshal_p, uint16_t out);
uint8_t marshal_named_uint16(marshal_p, const char *name, uint16_t out);
uint8_t marshal_uint32(marshal_p, uint32_t out);
uint8_t marshal_named_uint32(marshal_p, const char *name, uint32_t out);
uint8_t marshal_uint64(marshal_p, uint64_t out);
uint8_t marshal_named_uint64(marshal_p, const char *name, uint64_t out);

uint8_t marshal_int8(marshal_p, int8_t out);
uint8_t marshal_named_int8(marshal_p, const char *name, int8_t out);
uint8_t marshal_int16(marshal_p, int16_t out);
uint8_t marshal_named_int16(marshal_p, const char *name, int16_t out);
uint8_t marshal_int32(marshal_p, int32_t out);
uint8_t marshal_named_int32(marshal_p, const char *name, int32_t out);
uint8_t marshal_int64(marshal_p, int64_t out);
uint8_t marshal_named_int64(marshal_p, const char *name, int64_t out);

uint8_t marshal_float(marshal_p, float out);
uint8_t marshal_named_float(marshal_p, const char *name, float out);
uint8_t marshal_ptr(marshal_p, void *out);
uint8_t marshal_named_ptr(marshal_p, const char *name, void *out);
uint8_t marshal_ptr_ref(marshal_p, void *out);
uint8_t marshal_named_ptr_ref(marshal_p, const char *name, void *out);
uint8_t marshal_bra(marshal_p);
uint8_t marshal_ket(marshal_p);
uint8_t marshal_array_bra(marshal_p);
uint8_t marshal_array_ket(marshal_p);

#define GET_NAME(D, N)                 demarshal_name((D), (N))
#define GET_PEEK_NAME(D, N)            demarshal_peek_name((D), (N))

#define GET_STRING(D, V)               demarshal_string((D), &(V))
#define GET_PEEK_STRING(D, V)          demarshal_peek_string((D), &(V))
#define GET_NAMED_STRING(D, N, V)      demarshal_named_string((D), (N), &(V))
#define GET_OPT_NAMED_STRING(D, N, V)  demarshal_opt_named_string((D), (N), &(V))
#define GET_OPT_DEF_NAMED_STRING(D, N, V, DEF)  demarshal_opt_def_named_string((D), (N), &(V), DEF)

#define GET_UINT8(D, V)                demarshal_uint8((D), &(V))
#define GET_PEEK_UINT8(D)              demarshal_peek_int((D))
#define GET_NAMED_UINT8(D, N, V)       demarshal_named_uint8((D), (N), &(V))
#define GET_OPT_NAMED_UINT8(D, N, V)   demarshal_opt_named_uint8((D), (N), &(V))
#define GET_OPT_DEF_NAMED_UINT8(D, N, V, DEF)   demarshal_opt_def_named_uint8((D), (N), &(V), DEF)

#define GET_UINT16(D, V)               demarshal_uint16((D), &(V))
#define GET_PEEK_UINT16(D)             demarshal_peek_int((D))
#define GET_NAMED_UINT16(D, N, V)      demarshal_named_uint16((D), (N), &(V))
#define GET_OPT_NAMED_UINT16(D, N, V)  demarshal_opt_named_uint16((D), (N), &(V))
#define GET_OPT_DEF_NAMED_UINT16(D, N, V, DEF)  demarshal_opt_def_named_uint16((D), (N), &(V), DEF)

#define GET_UINT32(D, V)               demarshal_uint32((D), &(V))
#define GET_PEEK_UINT32(D)             demarshal_peek_int((D))
#define GET_NAMED_UINT32(D, N, V)      demarshal_named_uint32((D), (N), &(V))
#define GET_OPT_NAMED_UINT32(D, N, V)  demarshal_opt_named_uint32((D), (N), &(V))
#define GET_OPT_DEF_NAMED_UINT32(D, N, V, DEF)  demarshal_opt_def_named_uint32((D), (N), &(V), DEF)

#define GET_UINT64(D, V)               demarshal_uint64((D), &(V))
#define GET_PEEK_UINT64(D)             demarshal_peek_int((D))
#define GET_NAMED_UINT64(D, N, V)      demarshal_named_uint64((D), (N), &(V))
#define GET_OPT_NAMED_UINT64(D, N, V)  demarshal_opt_named_uint64((D), (N), &(V))
#define GET_OPT_DEF_NAMED_UINT64(D, N, V, DEF)  demarshal_opt_def_named_uint64((D), (N), &(V), DEF)

#define GET_INT8(D, V)                 demarshal_int8((D), &(V))
#define GET_PEEK_INT8(D)               demarshal_peek_int8((D))
#define GET_NAMED_INT8(D, N, V)        demarshal_named_int8((D), (N), &(V))
#define GET_OPT_NAMED_INT8(D, N, V)    demarshal_opt_named_int8((D), (N), &(V))
#define GET_OPT_DEF_NAMED_INT8(D, N, V, DEF)    demarshal_opt_def_named_int8((D), (N), &(V), DEF)

#define GET_INT16(D, V)                demarshal_int16((D), &(V))
#define GET_PEEK_INT16(D)              demarshal_peek_int16((D))
#define GET_NAMED_INT16(D, N, V)       demarshal_named_int16((D), (N), &(V))
#define GET_OPT_NAMED_INT16(D, N, V)   demarshal_opt_named_int16((D), (N), &(V))
#define GET_OPT_DEF_NAMED_INT16(D, N, V, DEF)   demarshal_opt_def_named_int16((D), (N), &(V), DEF)

#define GET_INT32(D, V)                demarshal_int32((D), &(V))
#define GET_PEEK_INT32(D)              demarshal_peek_int32((D))
#define GET_NAMED_INT32(D, N, V)       demarshal_named_int32((D), (N), &(V))
#define GET_OPT_NAMED_INT32(D, N, V)   demarshal_opt_named_int32((D), (N), &(V))
#define GET_OPT_DEF_NAMED_INT32(D, N, V, DEF)   demarshal_opt_def_named_int32((D), (N), &(V), DEF)

#define GET_INT64(D, V)                demarshal_int64((D), &(V))
#define GET_PEEK_INT64(D)              demarshal_peek_int64((D))
#define GET_NAMED_INT64(D, N, V)       demarshal_named_int64((D), (N), &(V))
#define GET_OPT_NAMED_INT64(D, N, V)   demarshal_opt_named_int64((D), (N), &(V))
#define GET_OPT_DEF_NAMED_INT64(D, N, V, DEF)   demarshal_opt_def_named_int64((D), (N), &(V), DEF)

#define GET_FLOAT(D, V)                demarshal_float((D), &(V))
#define GET_PEEK_FLOAT(D)              demarshal_peek_float((D))
#define GET_NAMED_FLOAT(D, N, V)       demarshal_named_float((D), (N), &(V))
#define GET_OPT_NAMED_FLOAT(D, N, V)   demarshal_opt_named_float((D), (N), &(V))
#define GET_OPT_DEF_NAMED_FLOAT(D, N, V, DEF)   demarshal_opt_def_named_float((D), (N), &(V), DEF)

#ifdef TOO_SLOW
#define GET_PTR(D, V)                  demarshal_ptr((D), &(V))
#define GET_PEEK_PTR(D, V)             demarshal_peek_ptr((D), &(V))
#define GET_NAMED_PTR(D, N, V)         demarshal_named_ptr((D), (N), &(V))
#define GET_OPT_NAMED_PTR(D, N, V)     demarshal_opt_named_ptr((D), (N), &(V))

#define GET_NAMED_PTR_REF(D, N, V)     demarshal_named_ptr_ref((D), (N), &(V))
#define GET_PEEK_PTR_REF(D, V)         demarshal_peek_ptr_ref((D), &(V))
#define GET_OPT_NAMED_PTR_REF(D, N, V) demarshal_opt_named_ptr_ref((D), (N), &(V))
#endif

#define GET_PEEK_BRA(D)                demarshal_peek_bra((D))
#define GET_BRA(D)                     demarshal_bra((D))
#define GET_KET(D)                     demarshal_ket((D))
#define GET_ARRAY_BRA(D)               demarshal_bra((D))
#define GET_ARRAY_KET(D)               demarshal_ket((D))

#define GET_OPT_NAMED_BITFIELD(D, N, V)                 \
                                                        \
    if (GET_PEEK_NAME(D, N))                            \
    {                                                   \
        GET_NAME((D), (N));                             \
        (V) = 1;                                        \
    }

#define PUT_NAMED_BITFIELD(D, N, V)                     \
                                                        \
    if (V)                                              \
    {                                                   \
        PUT_NAME_ONLY((D), (N));                        \
    }

#define GET_NAMED_ENUM(D, N, V, E)                      \
{                                                       \
    char *tmp = 0;                                      \
                                                        \
    GET_NAMED_STRING((D), (N), tmp);                    \
    if (tmp) {                                          \
        (V) = (E)(tmp);                                 \
        myfree(tmp);                                    \
    }                                                   \
}

#define GET_OPT_NAMED_ENUM(D, N, V, E)                  \
if (GET_PEEK_NAME(D, N))                                \
{                                                       \
    char *tmp = 0;                                      \
                                                        \
    GET_NAMED_STRING((D), (N), tmp);                    \
    if (tmp) {                                          \
        (V) = (E)(tmp);                                 \
        myfree(tmp);                                    \
    }                                                   \
}

#define PUT_NAME(D, N)                marshal_name((D), (N))
#define PUT_NAME_ONLY(D, N)           marshal_name_only((D), (N))

#define PUT_STRING(D, V)              marshal_string((D), (V))
#define PUT_NAMED_STRING(D, N, V)     marshal_named_string((D), (N), (V))

#define PUT_UINT8(D, V)               marshal_uint8((D), (V))
#define PUT_NAMED_UINT8(D, N, V)      marshal_named_uint8((D), (N), (V))
#define PUT_UINT16(D, V)              marshal_uint16((D), (V))
#define PUT_NAMED_UINT16(D, N, V)     marshal_named_uint16((D), (N), (V))
#define PUT_UINT32(D, V)              marshal_uint32((D), (V))
#define PUT_NAMED_UINT32(D, N, V)     marshal_named_uint32((D), (N), (V))
#define PUT_UINT64(D, V)              marshal_uint64((D), (V))
#define PUT_NAMED_UINT64(D, N, V)     marshal_named_uint64((D), (N), (V))

#define PUT_INT8(D, V)                marshal_int8((D), (V))
#define PUT_NAMED_INT8(D, N, V)       marshal_named_int8((D), (N), (V))
#define PUT_INT16(D, V)               marshal_int16((D), (V))
#define PUT_NAMED_INT16(D, N, V)      marshal_named_int16((D), (N), (V))
#define PUT_INT32(D, V)               marshal_int32((D), (V))
#define PUT_NAMED_INT32(D, N, V)      marshal_named_int32((D), (N), (V))
#define PUT_INT64(D, V)               marshal_int64((D), (V))
#define PUT_NAMED_INT64(D, N, V)      marshal_named_int64((D), (N), (V))

#define PUT_FLOAT(D, V)               marshal_float((D), (V))
#define PUT_NAMED_FLOAT(D, N, V)           marshal_named_float((D), (N), (V))

#define PUT_PTR(D, V)                 marshal_ptr((D), (V))
#define PUT_NAMED_PTR(D, N, V)        marshal_named_ptr((D), (N), (V))
#define PUT_NAMED_PTR_REF(D, N, V)    marshal_named_ptr_ref((D), (N), (V))

#define PUT_NAMED_ENUM(D, N, V, E)    marshal_named_string((D), (N), (E)(V))

#define PUT_BRA(D)                    marshal_bra((D))
#define PUT_KET(D)                    marshal_ket((D))
#define PUT_ARRAY_BRA(D)              marshal_array_bra((D))
#define PUT_ARRAY_KET(D)              marshal_array_ket((D))
