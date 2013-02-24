/*
 * Copyright (C) 2011 Neil McGill
 *
 * See the README file.
 */

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
