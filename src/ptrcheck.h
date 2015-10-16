/*
 * Copyright (C) 2011-2015 goblinhack@gmail.com
 *
 * See the README file for license info for license.
 * A useful hack to help debug stale/free/bad pointers.
 */
#pragma once

#ifndef __PTRCHECK_H__
#define __PTRCHECK_H__

/*
 * Convert function and line number into a single string for debugging.
 */
#define PTRCHECK_STRINGIFY(x)   #x
#define PTRCHECK_TO_STRING(x)   PTRCHECK_STRINGIFY(x)

/*
 * __FUNCTION__ is not a preprocessor directive so we can't convert it into a
 * string
 */
#define PTRCHECK_AT             __FILE__, __FUNCTION__, __LINE__

#define myzalloc(__size__, __what__) \
    myzalloc_((__size__), (__what__), PTRCHECK_AT)

#define mymalloc(__size__, __what__) \
    mymalloc_((__size__), (__what__), PTRCHECK_AT)

#define myrealloc(__ptr__, __size__, __what__) \
    myrealloc_((__ptr__), (__size__), (__what__), PTRCHECK_AT)

#define myfree(__ptr__)             \
    myfree_((__ptr__), PTRCHECK_AT)

#define dupstr(__ptr__, __what__)   \
    dupstr_((__ptr__), (__what__), PTRCHECK_AT)

uint8_t ptrcheck_init(void);
void ptrcheck_fini(void);

void *ptrcheck_alloc(const void *ptr, const char *what, const uint32_t size,
                     const char *file, const char *func, const uint32_t line);

uint8_t ptrcheck_verify(const void *ptr, const char *file, const char *func,
                        const uint32_t line);

uint8_t ptrcheck_fast_verify(const void *ptr, 
                             const char *file, const char *func,
                             const uint32_t line);

uint8_t ptrcheck_free(void *ptr, const char *file, const char *func,
                      const uint32_t line);

void ptrcheck_leak_print(void);
void ptrcheck_leak_snapshot(void);
void ptrcheck_usage_print(void);

#ifdef ENABLE_PTRCHECK /* { */
#define newptr(__ptr__, __what__) (ptrcheck_alloc((__ptr__),( __what__), sizeof(*(__ptr__)), PTRCHECK_AT))
#define oldptr(__ptr__) (ptrcheck_free((__ptr__), PTRCHECK_AT))
#define verify(__ptr__) (ptrcheck_verify((__ptr__), PTRCHECK_AT))
#define fast_verify(__ptr__) (ptrcheck_fast_verify((__ptr__), PTRCHECK_AT))
#else /* } { */
#define newptr(__ptr__, __what__)
#define oldptr(__ptr__) ASSERT(__ptr__)
#define verify(__ptr__) ASSERT(__ptr__)
#define fast_verify(__ptr__) ASSERT(__ptr__)
#endif /* ENABLE_PTRCHECK } */

#endif /* __PTRCHECK_H__ */
