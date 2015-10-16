/*
 * Copyright (C) 2011-2015 goblinhack@gmail.com
 *
 * See the README file for license info for license.
 */

#define ENUM_EXAMPLE(list_macro)                     \
    list_macro(ENUM_EXAMPLE_STEELBLUE = 0, "red"),   \
    list_macro(ENUM_EXAMPLE_GREEN,         "green"), \
    list_macro(ENUM_EXAMPLE_BLUE,          "blue"),  \

ENUM_DEF_H(ENUM_EXAMPLE, enum_example)
ENUM_DEF_C(ENUM_EXAMPLE, enum_example)

/*
 * u8=255
 * u16=65535
 * u32=4294967295
 * u64=18446744073709551615
 * i8=-1
 * i16=-1
 * i32=-1
 * i64=-1
 * a=1
 * b="hello"
 * c={
 *  d=-1.234000
 *  e={
 *   f="there"
 *   g=[ 1 2 3 ]
 *   h=1
 *  }
 *  ptr_ref1=@0x10a712a10
 *  ptr=*0x10a712a10
 *  ptr_ref2=@0x10a712a10
 * }
 */
struct A {
    int8_t i8;
    int16_t i16;
    int32_t i32;
    int64_t i64;
    uint8_t u8;
    uint16_t u16;
    uint32_t u32;
    uint64_t u64;
    int32_t a;
    char *b;
    struct C {
        float d;
        struct E {
            char *f;
            int32_t g[3];
        } e;
        unsigned char h:1;
    } c;
    void *ptr_ref1;
    void *ptr_ref2;
    void *ptr;
    enum_example en;
};

static void demarshal_a (demarshal_p ctx, struct A *a)
{
    memset(a, 0, sizeof(*a));

    GET_OPT_NAMED_UINT8(ctx,  "u8",  a->u8);
    GET_OPT_NAMED_UINT16(ctx, "u16", a->u16);
    GET_OPT_NAMED_UINT32(ctx, "u32", a->u32);
    GET_OPT_NAMED_UINT64(ctx, "u64", a->u64);
    GET_OPT_NAMED_INT8(ctx,  "i8",  a->i8);
    GET_OPT_NAMED_INT16(ctx, "i16", a->i16);
    GET_OPT_NAMED_INT32(ctx, "i32", a->i32);
    GET_OPT_NAMED_INT64(ctx, "i64", a->i64);
    GET_OPT_DEF_NAMED_INT32(ctx, "a", a->a, 123);
    GET_OPT_DEF_NAMED_STRING(ctx, "b", a->b, "hello default");
    GET_NAME(ctx, "c");
    GET_BRA(ctx); /* { */
        GET_OPT_NFLOAT(ctx, "d", a->c.d);
        GET_NAME(ctx, "e");
        GET_BRA(ctx); /* { */
            GET_OPT_NAMED_STRING(ctx, "f", a->c.e.f);
            GET_NAME(ctx, "g");
            GET_ARRAY_BRA(ctx); /* { */
                GET_INT32(ctx, a->c.e.g[0]);
                GET_INT32(ctx, a->c.e.g[1]);
                GET_INT32(ctx, a->c.e.g[2]);
            GET_ARRAY_KET(ctx); /* } */
            GET_OPT_NAMED_BITFIELD(ctx, "h", a->c.h);
        GET_KET(ctx); /* } */
        GET_OPT_NAMED_PTR_REF(ctx, "ptr_ref1", a->ptr_ref1);
        GET_OPT_NAMED_PTR(ctx, "ptr", a->ptr);
        a->ptr = (void*)mymalloc(100, "test");
        GET_OPT_NAMED_PTR_REF(ctx, "ptr_ref2", a->ptr_ref2);
        GET_OPT_NAMED_ENUM(ctx, "enum_example", a->en, enum_example_str2val);
    GET_KET(ctx); /* } */
}

static void marshal_a (marshal_p ctx, struct A *a)
{
    PUT_NAMED_UINT8(ctx,  "u8",  a->u8);
    PUT_NAMED_UINT16(ctx, "u16", a->u16);
    PUT_NAMED_UINT32(ctx, "u32", a->u32);
    PUT_NAMED_UINT64(ctx, "u64", a->u64);
    PUT_NAMED_INT8(ctx,  "i8",  a->i8);
    PUT_NAMED_INT16(ctx, "i16", a->i16);
    PUT_NAMED_INT32(ctx, "i32", a->i32);
    PUT_NAMED_INT64(ctx, "i64", a->i64);
    PUT_NAMED_INT32(ctx, "a", a->a);
    PUT_NAMED_STRING(ctx, "b", a->b);
    PUT_NAME(ctx, "c");
    PUT_BRA(ctx); /* { */
        PUT_NAMED_FLOAT(ctx, "d", a->c.d);
        PUT_NAME(ctx, "e");
        PUT_BRA(ctx); /* { */
            PUT_NAMED_STRING(ctx, "f", a->c.e.f);
            PUT_NAME(ctx, "g");
            PUT_ARRAY_BRA(ctx); /* { */
                PUT_INT32(ctx, a->c.e.g[0]);
                PUT_INT32(ctx, a->c.e.g[1]);
                PUT_INT32(ctx, a->c.e.g[2]);
            PUT_ARRAY_KET(ctx); /* } */
            PUT_NAMED_BITFIELD(ctx, "h", a->c.h);
        PUT_KET(ctx); /* } */
        PUT_NAMED_PTR_REF(ctx, "ptr_ref1", a->ptr_ref1);
        PUT_NAMED_PTR(ctx, "ptr", a->ptr);
        PUT_NAMED_PTR_REF(ctx, "ptr_ref2", a->ptr_ref2);
        PUT_NAMED_ENUM(ctx, "enum_example", a->en, enum_example_val2str);
    PUT_KET(ctx); /* } */
}

uint8_t marshal_test (int32_t argc, char *argv[])
{
    struct A a;

#ifdef INIT_A
    a.u8  = (typeof(a.u8)) -1;
    a.u16 = (typeof(a.u16)) -1;
    a.u32 = (typeof(a.u32)) -1;
    a.u64 = (typeof(a.u64)) -1;
    a.i8  = (typeof(a.i8)) -1;
    a.i16 = (typeof(a.i16)) -1;
    a.i32 = (typeof(a.i32)) -1;
    a.i64 = (typeof(a.i64)) -1;
    a.a   = 1;
    a.b   = (char*)"hello";
    a.c.d = -1.234;
    a.c.e.f = (char*)"there";
    a.c.e.g[0] = 1;
    a.c.e.g[1] = 2;
    a.c.e.g[2] = 3;
    a.c.h = 1;
    a.ptr = (void *)&a;
    a.ptr_ref1 = a.ptr;
    a.ptr_ref2 = a.ptr;
#endif

    demarshal_p in;
    in = demarshal("marshal_test");
    demarshal_a(in, &a);
    demarshal_fini(in);

    marshal_p out;
    out = marshal("marshal_test");
    marshal_a(out, &a);
    marshal_fini(out);

    out = marshal(0); /* MY_STDOUT */
    marshal_a(out, &a);
    marshal_fini(out);
    myfree(a.ptr);
    myfree(a.b);
    myfree(a.c.e.f);

    return (0);
}
