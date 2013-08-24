/*
 * Copyright (C) 2011 Neil McGill
 *
 * See the README file for license.
 */

#include <SDL.h>
#include "glapi.h"

#include "main.h"
#include "sdl.h"
#include "math.h"
#include "collision.h"
#include "gl.h"
#include "color.h"
#include "geo.h"
#include "time.h"

typedef struct {
    fpoint br;
    fpoint tl;
} box;

typedef struct {
    float radius;
} circle;

typedef struct {
    boolean box;
    fpoint at;
    fpoint old_at;
    fpoint velocity;
    fpoint impact_velocity;
    fpoint accel;
    uint32_t last_tick;
    union {
        box b;
        circle c;
    } shape;
    boolean is_stationary:1;
    boolean is_debug:1;
} object;

typedef struct {
    object *A;
    object *B;
    float penetration;
    fpoint normal;
    fpoint velocity;
} collision_t;

static float GRAVITY            = 0.5;
static float FRICTION           = 1.1;
static float MAX_VELOCITY       = 2.0;
static const uint32_t OBJ_RADIUS = 20;
static const uint32_t OBJ_MAX = 100;
static uint32_t obj_max;
static object objects[OBJ_MAX];

#define GL_MAX_BUFFER_SIZE 16000

static GLfloat xy[GL_MAX_BUFFER_SIZE];
static GLfloat *xyp = xy;
static GLsizei gl_state;

static boolean collision_resolve_obj(object *);

static void Begin (GLsizei gl_enum)
{
    gl_state = gl_enum;
}

static void End (void)
{
    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(2, GL_FLOAT, 0, xy);
    glDrawArrays(gl_state, 0, (int32_t)((xyp - xy) / 2));
    glDisableClientState(GL_VERTEX_ARRAY);

    xyp = xy;
}

static void collision_init (void)
{
    int32_t W = global_config.video_gl_width;
    int32_t H = global_config.video_gl_height;
    int32_t w = W - OBJ_RADIUS * 4;
    int32_t h = H - OBJ_RADIUS * 4;
    uint32_t o;

    object *obj;
    fpoint velocity;
    fpoint at;

    for (o = 0; o < OBJ_MAX - 4; o++) {
        /*
         * Repeat placing objects until there are no collisions.
         */
        for (;;) {
            obj = &objects[o];

            at.x = rand() % w;
            at.y = rand() % h;
            at.x += OBJ_RADIUS * 2;
            at.y += OBJ_RADIUS * 2;

            velocity.x = ((float)((rand() % w) - (w / 2))) / (float)w;
            velocity.y = ((float)((rand() % h) - (h / 2))) / (float)h;

            obj->box = o & 1;
            obj->box = 0;

            if (obj->box) {
                obj->shape.b.tl.x = at.x - OBJ_RADIUS;
                obj->shape.b.tl.y = at.y - OBJ_RADIUS;
                obj->shape.b.br.x = at.x + OBJ_RADIUS;
                obj->shape.b.br.y = at.y + OBJ_RADIUS;
            } else {
                obj->shape.c.radius = OBJ_RADIUS;
            }

            obj->at = at;
            obj->velocity = velocity;

            obj_max++;

            if (!collision_resolve_obj(obj)) {
                break;
            }

            obj_max--;
        }

        if (!(rand() % 3)) {
            obj->is_stationary = true;
            obj->is_debug = true;
        }
    }

    obj++;
    obj->box = true;
    obj->shape.b.tl.x = 0;
    obj->shape.b.tl.y = H - OBJ_RADIUS;
    obj->shape.b.br.x = W;
    obj->shape.b.br.y = H;
    obj->at.x = (obj->shape.b.tl.x + obj->shape.b.br.x) / 2.0;
    obj->at.y = (obj->shape.b.tl.y + obj->shape.b.br.y) / 2.0;
    obj->is_stationary = true;

    obj++;
    obj->box = true;
    obj->shape.b.tl.x = 0;
    obj->shape.b.tl.y = 0;
    obj->shape.b.br.x = OBJ_RADIUS;
    obj->shape.b.br.y = H;
    obj->at.x = (obj->shape.b.tl.x + obj->shape.b.br.x) / 2.0;
    obj->at.y = (obj->shape.b.tl.y + obj->shape.b.br.y) / 2.0;
    obj->is_stationary = true;

    obj++;
    obj->box = true;
    obj->shape.b.tl.x = W - OBJ_RADIUS;
    obj->shape.b.tl.y = 0;
    obj->shape.b.br.x = W;
    obj->shape.b.br.y = H;
    obj->at.x = (obj->shape.b.tl.x + obj->shape.b.br.x) / 2.0;
    obj->at.y = (obj->shape.b.tl.y + obj->shape.b.br.y) / 2.0;
    obj->is_stationary = true;

    obj++;
    obj->box = true;
    obj->shape.b.tl.x = 0;
    obj->shape.b.tl.y = 0;
    obj->shape.b.br.x = W;
    obj->shape.b.br.y = OBJ_RADIUS;
    obj->at.x = (obj->shape.b.tl.x + obj->shape.b.br.x) / 2.0;
    obj->at.y = (obj->shape.b.tl.y + obj->shape.b.br.y) / 2.0;
    obj->is_stationary = true;
}

//
// Draw all objects
//
static void collision_draw (void)
{
    uint32_t o;

    glBindTexture(GL_TEXTURE_2D, 0);
    glLineWidth(2.0);
    glcolor(WHITE);

    for (o = 0; o < OBJ_MAX; o++) {
        object *obj;

        obj = &objects[o];

        if (obj->is_debug) {
            glcolor(RED);
        } else {
            glcolor(WHITE);
        }

        Begin(GL_LINE_LOOP);
            
        if (obj->box) {
            Vertex2f(obj->shape.b.tl.x, obj->shape.b.tl.y);
            Vertex2f(obj->shape.b.br.x, obj->shape.b.tl.y);
            Vertex2f(obj->shape.b.br.x, obj->shape.b.br.y);
            Vertex2f(obj->shape.b.tl.x, obj->shape.b.br.y);
        } else {
            const float rad_step = RAD_360 / 360.0;
 
            for (int i = 0; i < 360; i++) {
                float rad = ((float) i) * rad_step;
                float radius = obj->shape.c.radius;

                Vertex2f(obj->at.x + (cos(rad) * radius),
                         obj->at.y + (sin(rad) * radius));
            }
        }

        End();
    }
}

//
// If two circles collide, the resultant direction is along the normal between
// the two center of masses of the circles.
//
static boolean circle_circle_collision (object *A, object *B, collision_t *m)
{
    circle *CA = &A->shape.c;
    circle *CB = &B->shape.c;

    fpoint n = fsub(B->at, A->at);
 
    float touching_dist = CA->radius + CB->radius;
    float dist = flength(n);

    //
    // Circles are not touching
    //
    if (dist > touching_dist) {
        return (false);
    }
 
    //
    // Circles are centered on each other
    //
    if (dist == 0.0) {
        m->penetration = CA->radius;
        m->normal.x = 0;
        m->normal.y = -1;

        return (true);
    }

    m->penetration = touching_dist - dist;
    m->normal = fdiv(touching_dist, n);

    return (true);
}

static boolean collision_resolve_obj (object *A)
{
    int32_t W = global_config.video_gl_width;
    int32_t H = global_config.video_gl_height;
    boolean collision;
    uint32_t b;

    collision = false;

    if (A->at.y > H - OBJ_RADIUS * 2) {
        A->velocity.y += -(A->velocity.y * FRICTION);
        collision = true;
    }

    if (A->at.x > W - OBJ_RADIUS * 2) {
        A->velocity.x += -(A->velocity.x * FRICTION);
        collision = true;
    }

    if (A->at.x < OBJ_RADIUS * 2) {
        A->velocity.x += -(A->velocity.x * FRICTION);
        collision = true;
    }

    for (b = 0; b < obj_max; b++) {
        object *B;

        B = &objects[b];

        if (A == B) {
            continue;
        }

        if (!A->box && !B->box) {
            collision_t m;

            if (circle_circle_collision(A, B, &m)) {

                collision = true;

#if 0
                glcolor(RED);

                Begin(GL_LINES);
                    Vertex2f(A->at.x, A->at.y);

                    Vertex2f(A->at.x + m.normal.x * (float)OBJ_RADIUS,
                                A->at.y + m.normal.y * (float)OBJ_RADIUS);
                End();
#endif

                fpoint relative_velocity = fsub(B->at, A->at);

                float cross = fcross(relative_velocity, m.normal);
                if (cross > 0.0) {
                    /*
                     * Heading apart.
                     */
                    continue;
                }

                float vA = flength(A->velocity);

                A->velocity.x += -m.normal.x * vA * FRICTION;
                A->velocity.y += -m.normal.y * vA * FRICTION;
            }
        }
    }

    return (collision);
}

static void collision_resolve (void)
{
    uint32_t a;

    for (a = 0; a < obj_max; a++) {
        object *A;

        A = &objects[a];

        if (A->is_stationary) {
            continue;
        }

        /*
         * How many small steps we will take for this object.
         */
        float timestep = ceil(flength(A->velocity));

        /*
         * Gravity.
         */
        A->accel.x = 0;
        A->accel.y = GRAVITY;

        /*
         * Gravity in small steps.
         */
        fpoint dA = A->accel;
        fdiv(timestep, dA);

        for (;;) {
            A->velocity.x += dA.x;
            A->velocity.y += dA.y;

            /*
             * Velocity in small steps.
             */
            fpoint dV = A->velocity;
            fdiv(timestep, dV);

            /*
             * Check for maxium air speed.
             */
            float len_velocity = flength(A->velocity);
            if (len_velocity > MAX_VELOCITY) {
                fpoint unit_velocity = funit(A->velocity);

                A->velocity = fmul(MAX_VELOCITY, unit_velocity);
            }

            /*
             * Save the old location.
             */
            A->old_at = A->at;

            /*
             * Creep forward.
             */
            A->at.x += dV.x;
            A->at.y += dV.y;

            /*
             * If we impact at the new location, move back to where we were.
             */
            if (collision_resolve_obj(A)) {
                A->at = A->old_at;
            }

            /*
             * Step forward in time.
             */
            timestep -= 1.0;
            if (timestep <= 0.0) {
                break;
            }
        }
    }
}

void collision_test (void)
{
    static boolean first = true;

    if (first) {
        first = false;
        collision_init();
        return;
    }

    collision_resolve();
    collision_draw();
    SDL_Delay(5);
}
