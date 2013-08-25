/*
 * Copyright (C) 2011 Neil McGill
 *
 * See the README file for license.
 */

#include <math.h>
#include <stdlib.h>

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

double FSIN[RAD_MAX];
double FCOS[RAD_MAX];

static void fsin_init (void)
{
    uint16_t i;

    for (i = 0; i < RAD_MAX; i++) {
        FSIN[i] = (double)sin(RAD_STEP * (double) i);
    }
}

static void fcos_init (void)
{
    uint16_t i;

    for (i = 0; i < RAD_MAX; i++) {
        FCOS[i] = (double)cos(RAD_STEP * (double) i);
    }
}

void math_init (void)
{
    fsin_init();
    fcos_init();
}

double gauss (const double m, const double s)
{
    static int use_last = 0;
    static double y2;
    double x1, x2, w, y1;

    if (use_last) {
        y1 = y2;
    } else {
        do { // ming don't have random
            x1 = 2.0 * ((double)rand() / (double)RAND_MAX) - 1.0;
            x2 = 2.0 * ((double)rand() / (double)RAND_MAX) - 1.0;
            w = x1 * x1 + x2 * x2;
        } while (w >= 1.0);

        w = sqrt((-2.0 * log(w)) / w);
        y1 = x1 * w;
        y2 = x2 * w;
    }

    use_last = !use_last;

    return (m + y1 * s);
}
