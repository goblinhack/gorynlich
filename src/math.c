/*
 * Copyright (C) 2011 Neil McGill
 *
 * See the README file for license.
 */

#include <math.h>

#include "main.h"
#include "math.h"

uint32_t nextpoweroftwo (uint32_t val)
{
    val--;
    val = (val >> 1) | val;
    val = (val >> 2) | val;
    val = (val >> 4) | val;
    val = (val >> 8) | val;
    val = (val >> 16) | val;
    val++; // Val is now the next highest power of 2.

    return (val);
}

float FSIN[RAD_MAX];
float FCOS[RAD_MAX];

static void fsin_init (void)
{
    uint16_t i;

    for (i = 0; i < RAD_MAX; i++) {
        FSIN[i] = (float)sin(RAD_STEP * (float) i);
    }
}

static void fcos_init (void)
{
    uint16_t i;

    for (i = 0; i < RAD_MAX; i++) {
        FCOS[i] = (float)cos(RAD_STEP * (float) i);
    }
}

void math_init (void)
{
    fsin_init();
    fcos_init();
}
