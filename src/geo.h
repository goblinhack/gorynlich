/*
 * Copyright (C) 2011 Neil McGill
 *
 * See the README file for license.
 */

#include <math.h>

typedef struct {
    fpoint3d P0;
    fpoint3d P1;
} line;

typedef struct {
    fpoint3d V0;
    fpoint3d V1;
    fpoint3d V2;
} triangle;

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

static inline fpoint3d fpoint3d_sub (fpoint3d a, fpoint3d b)
{
    fpoint3d r = {a.x - b.x, a.y - b.y, a.z - b.z};

    return (r);
}

static inline fpoint3d fpoint3d_add (fpoint3d a, fpoint3d b)
{
    fpoint3d r = {a.x + b.x, a.y + b.y, a.z + b.z};

    return (r);
}

static inline fpoint3d fpoint3d_mul (float m, fpoint3d b)
{
    fpoint3d r = {m * b.x, m * b.y, m * b.z};

    return (r);
}

static inline int fpoint3d_cmp (fpoint3d a, fpoint3d b)
{
    return ((a.x == b.x) && (a.y == b.y) && (a.z == b.z));
}

static inline float fpoint3d_dot (fpoint3d a, fpoint3d b)
{
    return (a.x * b.x + a.y * b.y + a.z * b.z);
}
 
static inline fpoint3d fpoint3d_cross (fpoint3d a, fpoint3d b)
{
    fpoint3d c = {a.y*b.z - a.z*b.y, a.z*b.x - a.x*b.z, a.x*b.y - a.y*b.x};

    return (c);
}

static inline float fpoint3d_dist (fpoint3d a, fpoint3d b)
{
    return (DISTANCE3f(a.x, a.y, a.z, b.x, b.y, b.z));
}

static inline fpoint3d fpoint3d_normal (fpoint3d v)
{
    fpoint3d n = v;

    float length = sqrt((v.x * v.x) + (v.y * v.y) + (v.z * v.z));

    n.x = v.x / length;
    n.y = v.y / length;
    n.z = v.z / length;

    return (n);
}

/*
 * triangle_line_intersect(): find the 3D intersection of a line with a 
 * triangle
 *
 * -1 = triangle is degenerate (a segment or point)
 *  0 =  disjoint (no intersect)
 *  1 =  intersect in unique point I1
 *  2 =  are in the same plane
 */
int
triangle_line_intersect(const line ray,
                        const triangle triangle,
                        fpoint3d *intersection);

/*
 * true on intersection.
 */
boolean
cube_line_intersect(const line ray, 
                    const fpoint3d p0,
                    const fpoint3d p1,
                    const fpoint3d p2,
                    const fpoint3d p3,
                    const fpoint3d p4,
                    const fpoint3d p5,
                    const fpoint3d p6,
                    const fpoint3d p7,
                    fpoint3d *best_intersection,
                    float *best_distance);

/*
 * true if perpendicular line from point is in line segment.
 */
boolean 
dist_point_line(fpoint3d P0, fpoint3d L0, fpoint3d L1, float *dist);
