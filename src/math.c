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

/*
 * true if perpendicular line from point is in line segment.
 */
boolean get_line_intersection (fpoint p0,
                               fpoint p1,
                               fpoint p2,
                               fpoint p3,
                               fpoint *intersect)
{
    fpoint s1, s2;

    s1.x = p1.x - p0.x;
    s1.y = p1.y - p0.y;
    s2.x = p3.x - p2.x;
    s2.y = p3.y - p2.y;

    double s, t;

    s = (-s1.y * (p0.x - p2.x) + s1.x * (p0.y - p2.y)) /
            (-s2.x * s1.y + s1.x * s2.y);

    t = ( s2.x * (p0.y - p2.y) - s2.y * (p0.x - p2.x)) /
            (-s2.x * s1.y + s1.x * s2.y);

    if (s >= 0 && s <= 1 && t >= 0 && t <= 1) {
        // Collision detected
        intersect->x = p0.x + (t * s1.x);
        intersect->y = p0.y + (t * s1.y);
        return (true);
    }

    return (false); // No collision
}

boolean 
fpoint_dist_line (fpoint P0, fpoint L0, fpoint L1, double *dist)
{
    fpoint intersect;
    float mag;
    float U;
 
    /*
     * Can get the squared distance to avoid this.
     */
    mag = fdist(L1, L0);
 
    /*
     * Project point P onto the line and then calc the dot product.
     */
    U = (((P0.x - L0.x) * (L1.x - L0.x)) +
         ((P0.y - L0.y) * (L1.y - L0.y))) /
         (mag * mag);
 
    if (U < 0.0f) {
        intersect = L0;
    } else if (U > 1.0f) {
        intersect = L1;
    } else {
        intersect.x = L0.x + U * (L1.x - L0.x);
        intersect.y = L0.y + U * (L1.y - L0.y);
    }
 
    *dist = fdist(P0, intersect);
 
    if ((U < 0.0f) || (U > 1.0f)) {
        return (0);   // closest P0 does not fall within the line segment
    }
 
    return (1);
}

double 
fpoint_project_onto_line (fpoint P0, fpoint L0, fpoint L1)
{
    float mag;
    float U;
 
    /*
     * Can get the squared distance to avoid this.
     */
    mag = fdist(L1, L0);
 
    /*
     * Project point P onto the line and then calc the dot product.
     */
    U = (((P0.x - L0.x) * (L1.x - L0.x)) +
         ((P0.y - L0.y) * (L1.y - L0.y))) /
         (mag * mag);
 
    return (U);
}

/*
 * Yields an angle between 0 and 180 deg radians
 */
double fpoint_angle (const fpoint A, const fpoint B)
{
    const double a = sqrt(A.x*A.x + A.y*A.y);
    const double b = sqrt(B.x*B.x + B.y*B.y);

    return (acos((A.x*B.x + A.y*B.y) / (a * b)));
}

/*
 * Yields an angle between -180 and 180 deg radians
 */
double fpoint_angle_clockwise (const fpoint A, const fpoint B)
{
    return (atan2(A.x*B.y - A.y*B.x, A.x*B.x+A.y*B.y));
}
