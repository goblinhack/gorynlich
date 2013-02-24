/*
 * Copyright (C) 2011 Neil McGill
 *
 * See the README file.
 */

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
