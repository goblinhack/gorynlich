/*
 * Copyright (C) 2011-2015 goblinhack@gmail.com
 *
 * See the README file for license info.
 */

#pragma once

#include "main.h"

#include <stdarg.h>

void strrepc(char *s, const char *replace_set, char replace_with);
char *substr(const char *in, int32_t pos, int32_t len);
char *strappend(const char *in, const char *append);
char *strprepend(const char *in, const char *prepend);
char *strsub(const char *in, const char *remove, const char *replace_with);
uint32_t strcommon(const char *a, const char *b);
void strchop(char *s);
void strchopc(char *s, char c);
int32_t strisregexp(const char *in);
void strnoescape(char *uncompressed);
char *dynprintf(const char *fmt, ...);
char *dynvprintf(const char *fmt, va_list args);
char *mybasename(const char *in, const char *who);
uint8_t hex_dump(void *addr, uint64_t offset, uint64_t len);
uint8_t hex_dump_log(void *addr, uint64_t offset, uint64_t len);
char *strcasestr_(const char *s, const char *find);
