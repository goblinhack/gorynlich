/*
 * Copyright (C) 2011-2015 goblinhack@gmail.com
 *
 * See the README file for license info.
 */

#pragma once

#include <math.h>

#include "main.h"

#undef SQUARED
#define SQUARED(x) ((x) * (x))

#undef DISTANCE
#define DISTANCE(xa,ya,xb,yb) \
    sqrt((double)(SQUARED((xb)-(xa))+SQUARED((yb)-(ya))))

#undef DISTANCE3f
#define DISTANCE3f(xa,ya,za,xb,yb,zb) \
    sqrt((double)(SQUARED((xb)-(xa))+\
                 SQUARED((yb)-(ya))+\
                 SQUARED((zb)-(za))))

static inline fpoint my_fadd (const fpoint a, const fpoint b)
{
    fpoint r = {a.x + b.x, a.y + b.y};
    return (r);
}

static inline fpoint3d my_fadd3d (const fpoint3d a, const fpoint3d b)
{
    fpoint3d r = {a.x + b.x, a.y + b.y, a.z + b.z};

    return (r);
}

static inline fpoint my_fsub (const fpoint a, const fpoint b)
{
    fpoint r = {a.x - b.x, a.y - b.y};
    return (r);
}

static inline fpoint3d my_fsub3d (const fpoint3d a, const fpoint3d b)
{
    fpoint3d r = {a.x - b.x, a.y - b.y, a.z - b.z};

    return (r);
}

static inline fpoint my_fmul (double m, const fpoint b)
{
    fpoint r = {m * b.x, m * b.y};

    return (r);
}

static inline fpoint3d my_fmul3d (double m, const fpoint3d b)
{
    fpoint3d r = {m * b.x, m * b.y, m * b.z};

    return (r);
}

static inline fpoint my_fdiv (double m, const fpoint b)
{
    fpoint r = {b.x / m, b.y / m};

    return (r);
}

static inline fpoint3d my_fdiv3d (const double m, const fpoint3d b)
{
    fpoint3d r = {b.x / m, b.y / m, b.z / m};

    return (r);
}

static inline int fcmp (const fpoint3d a, const fpoint3d b)
{
    return ((a.x == b.x) && (a.y == b.y));
}

static inline int fcmp3d (const fpoint3d a, const fpoint3d b)
{
    return ((a.x == b.x) && (a.y == b.y) && (a.z == b.z));
}

static inline double fdot (const fpoint a, const fpoint b)
{
    return (a.x * b.x + a.y * b.y);
}

static inline double fdot3d (const fpoint3d a, const fpoint3d b)
{
    return (a.x * b.x + a.y * b.y + a.z * b.z);
}
 
static inline double fcross (const fpoint a, const fpoint b)
{
    double c = a.x*b.y - a.y*b.x;

    return (c);
}

static inline fpoint3d fpoint3d_cross (const fpoint3d a, const fpoint3d b)
{
    fpoint3d c = {a.y*b.z - a.z*b.y, a.z*b.x - a.x*b.z, a.x*b.y - a.y*b.x};

    return (c);
}

static inline fpoint fnormal (const fpoint v)
{
    fpoint n = v;

    double length = sqrt((v.x * v.x) + (v.y * v.y));

    n.x = v.x / length;
    n.y = v.y / length;

    return (n);
}

static inline fpoint3d fnormal3d (fpoint3d v)
{
    fpoint3d n = v;

    double length = sqrt((v.x * v.x) + (v.y * v.y) + (v.z * v.z));

    n.x = v.x / length;
    n.y = v.y / length;
    n.z = v.z / length;

    return (n);
}

static inline double flength (const fpoint a)
{
    return ((double)sqrt(a.x*a.x + a.y*a.y));
}

static inline double fdist (const fpoint a, const fpoint b)
{
    return (DISTANCE(a.x, a.y, b.x, b.y));
}

static inline double fdist3d (const fpoint3d a, const fpoint3d b)
{
    return (DISTANCE3f(a.x, a.y, a.z, b.x, b.y, b.z));
}

static inline fpoint funit (const fpoint p)
{
    const double length = sqrt(p.x*p.x + p.y*p.y);
    fpoint o;

    o.x = p.x / length;
    o.y = p.y / length;

    return (o);
}

static inline void unit (fpoint *p)
{
    const double length = sqrt(p->x*p->x + p->y*p->y);

    p->x = p->x / length;
    p->y = p->y / length;
}

static inline void unit3d (fpoint3d *p)
{
    const double length = sqrt(p->x*p->x + p->y*p->y + p->z*p->z);

    p->x = p->x / length;
    p->y = p->y / length;
    p->z = p->z / length;
}

uint32_t nextpoweroftwo(uint32_t val);

#define PI      3.14159265358979323846f /* pi */
#define RAD_45  0.78539816339744830962f /* pi/4 */
#define RAD_90  1.57079632679489661923f /* pi/2 */
#define RAD_180 3.14159265358979323846f /* pi */
#define RAD_360 (2.0f * 3.14159265358979323846f) /* 2pi */
#define RAD_270 (RAD_180 + RAD_90)

static inline double angle2radians (const double angle)
{
    return (angle * (RAD_360 / 360.0));
}

static inline double radians2angle (const double rad)
{
    return ((360.0 * rad) / RAD_360);
}

void math_init(void);

/*
 * Fast cached sin/cos routines.
 */
#define RAD_MAX 0xFFFF
#define RAD_STEP (RAD_360 / (double)RAD_MAX)

static inline double fsin (double rad)
{
    const uint16_t index = (uint16_t)(rad / RAD_STEP);
    extern double FSIN[RAD_MAX];

    return (FSIN[index & (RAD_MAX-1)]);
}

static inline double fasin (double rad)
{
    const uint16_t index = (uint16_t)(rad / RAD_STEP);
    extern double FASIN[RAD_MAX];

    return (FASIN[index & (RAD_MAX-1)]);
}

static inline double fcos (double rad)
{
    const uint16_t index = (uint16_t)(rad / RAD_STEP);
    extern double FCOS[RAD_MAX];

    return (FCOS[index & (RAD_MAX-1)]);
}

static inline double anglerot (fpoint p)
{
    double theta;
    double v = p.y / flength(p);

    if (v < 0) {
        theta = -asin(-v);
    } else {
        theta = asin(v);
    }

    if (p.x > 0) {
        if (p.y > 0) {
            return (theta);
        } else {
            return (RAD_360 + theta);
        }
    } else {
        return (RAD_180 - theta);
    }
}

static inline
fpoint fpoint_rotate (fpoint p, double theta)
{
    fpoint r;

    r.x = p.x * cos(theta) - p.y * sin(theta);
    r.y = p.x * sin(theta) + p.y * cos(theta);

    return (r);
}

/*
 * Yields an angle between 0 and 180 deg radians
 */
double fpoint_angle(const fpoint A, const fpoint B);

double fpoint_angle_clockwise(const fpoint A, const fpoint B);

/*
 * true if perpendicular line from point is in line segment.
 */
uint8_t 
fpoint_dist_line(fpoint P0, fpoint L0, fpoint L1, double *dist,
                 fpoint *intersect);

/*
 * true if lines intersect
 */
uint8_t get_line_intersection(fpoint p0,
                              fpoint p1,
                              fpoint p2,
                              fpoint p3,
                              fpoint *intersect);
uint8_t get_line_known_intersection(fpoint p0,
                              fpoint p1,
                              fpoint p2,
                              fpoint p3,
                              fpoint *intersect);

double gauss(const double m, const double s);

double fpoint_project_onto_line(fpoint P0, fpoint L0, fpoint L1);

double gaussrand(double mean, double stddev);
