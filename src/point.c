/*
 * Copyright (C) 2011 Neil McGill
 *
 * See the README file for license.
 */

#include <math.h>

#include "main.h"

fpoint fadd (fpoint a, fpoint b)
{
    fpoint r = {a.x + b.x, a.y + b.y};
    return (r);
}

fpoint fsub (fpoint a, fpoint b)
{
    fpoint r = {a.x - b.x, a.y - b.y};
    return (r);
}

float flength (fpoint a)
{
    return ((float)sqrt(a.x*a.x + a.y*a.y));
}

point add (point a, point b)
{
    point r = {a.x + b.x, a.y + b.y};
    return (r);
}

point sub (point a, point b)
{
    point r = {a.x - b.x, a.y - b.y};
    return (r);
}

float length (point a)
{
    return ((float)sqrt(a.x*a.x + a.y*a.y));
}
