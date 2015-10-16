/*
 * Copyright (C) 2011-2015 goblinhack@gmail.com
 *
 * See the README file for license info for license.
 */

#pragma once

void backtrace_print(void);

struct traceback_;
typedef struct traceback_ * tracebackp;

tracebackp traceback_alloc(void);
void traceback_free(tracebackp);
void traceback_stdout(tracebackp);
void traceback_stderr(tracebackp);
