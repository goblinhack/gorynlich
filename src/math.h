/*
 * Copyright (C) 2011 Neil McGill
 *
 * See the README file.
 */

#include <math.h>

#undef SQUARED
#define SQUARED(x) ((x) * (x))

#undef DISTANCE
#define DISTANCE(xa,ya,xb,yb) \
    sqrt((float)(SQUARED((xb)-(xa))+SQUARED((yb)-(ya))))

#undef DISTANCE3f
#define DISTANCE3f(xa,ya,za,xb,yb,zb) \
    sqrt((float)(SQUARED((xb)-(xa))+\
                 SQUARED((yb)-(ya))+\
                 SQUARED((zb)-(za))))

static inline fpoint fadd (const fpoint a, const fpoint b)
{
    fpoint r = {a.x + b.x, a.y + b.y};
    return (r);
}

static inline fpoint3d fadd3d (const fpoint3d a, const fpoint3d b)
{
    fpoint3d r = {a.x + b.x, a.y + b.y, a.z + b.z};

    return (r);
}

static inline fpoint fsub (const fpoint a, const fpoint b)
{
    fpoint r = {a.x - b.x, a.y - b.y};
    return (r);
}

static inline fpoint3d fsub3d (const fpoint3d a, const fpoint3d b)
{
    fpoint3d r = {a.x - b.x, a.y - b.y, a.z - b.z};

    return (r);
}

static inline fpoint fmul (float m, const fpoint b)
{
    fpoint r = {m * b.x, m * b.y};

    return (r);
}

static inline fpoint3d fmul3d (float m, const fpoint3d b)
{
    fpoint3d r = {m * b.x, m * b.y, m * b.z};

    return (r);
}

static inline fpoint fdiv (float m, const fpoint b)
{
    fpoint r = {b.x / m, b.y / m};

    return (r);
}

static inline fpoint3d fdiv3d (const float m, const fpoint3d b)
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

static inline float fdot3d (const fpoint3d a, const fpoint3d b)
{
    return (a.x * b.x + a.y * b.y + a.z * b.z);
}
 
static inline float fcross (const fpoint a, const fpoint b)
{
    float c = a.x*b.y - a.y*b.x;

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

    float length = sqrt((v.x * v.x) + (v.y * v.y));

    n.x = v.x / length;
    n.y = v.y / length;

    return (n);
}

static inline fpoint3d fnormal3d (fpoint3d v)
{
    fpoint3d n = v;

    float length = sqrt((v.x * v.x) + (v.y * v.y) + (v.z * v.z));

    n.x = v.x / length;
    n.y = v.y / length;
    n.z = v.z / length;

    return (n);
}

static inline float flength (const fpoint a)
{
    return ((float)sqrt(a.x*a.x + a.y*a.y));
}

static inline float flength3d (const fpoint3d a, const fpoint3d b)
{
    return (DISTANCE3f(a.x, a.y, a.z, b.x, b.y, b.z));
}

static inline fpoint funit (const fpoint p)
{
    const float length = sqrt(p.x*p.x + p.y*p.y);
    fpoint o;

    o.x = p.x / length;
    o.y = p.y / length;

    return (o);
}

static inline void unit (fpoint *p)
{
    const float length = sqrt(p->x*p->x + p->y*p->y);

    p->x = p->x / length;
    p->y = p->y / length;
}

static inline void unit3d (fpoint3d *p)
{
    const float length = sqrt(p->x*p->x + p->y*p->y + p->z*p->z);

    p->x = p->x / length;
    p->y = p->y / length;
    p->z = p->z / length;
}

uint32_t nextpoweroftwo(uint32_t val);

#define RAD_45  0.78539816339744830962f /* pi/4 */
#define RAD_90  1.57079632679489661923f /* pi/2 */
#define RAD_180 3.14159265358979323846f /* pi */
#define RAD_360 (2.0f * 3.14159265358979323846f) /* 2pi */
#define RAD_270 (RAD_180 + RAD_90)

static inline float angle2radians (const float angle)
{
    return (angle * (RAD_360 / 360.0));
}

static inline float radians2angle (const float rad)
{
    return ((360.0 * rad) / RAD_360);
}

void math_init(void);

/*
 * Fast cached sin/cos routines.
 */
#define RAD_MAX 0xFFF
#define RAD_STEP (RAD_360 / (float)RAD_MAX)

static inline float fsin (float rad)
{
    const uint16_t index = (uint16_t)(rad / RAD_STEP);
    extern float FSIN[RAD_MAX];

    return (FSIN[index & (RAD_MAX-1)]);
}

static inline float fcos (float rad)
{
    const uint16_t index = (uint16_t)(rad / RAD_STEP);
    extern float FCOS[RAD_MAX];

    return (FCOS[index & (RAD_MAX-1)]);
}

static inline float anglerot (fpoint p)
{
    float theta = asin(p.y / flength(p));

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

float gauss(const float m, const float s);
