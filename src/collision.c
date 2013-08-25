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

/*
 * Settings.
 */
static double GRAVITY                = 0.5;
static double LOSS_OF_ENERGY         = 0.8;
static double MAX_VELOCITY           = 2.0;
static double MAX_TIMESTEP           = 2.0;
static const uint32_t OBJ_MIN_RADIUS = 5;
static const uint32_t OBJ_MAX_RADIUS = 100;
static const uint32_t OBJ_MAX        = 200;

typedef struct {
    fpoint br;
    fpoint tl;
} box;

typedef struct {
    double radius;
} circle;

typedef struct {
    fpoint at;
    fpoint old_at;
    fpoint velocity;
    fpoint impulse;
    fpoint accel;
    union {
        box b;
        circle c;
    } shape;
    boolean box:1;
    boolean is_stationary:1;
    boolean is_debug:1;
} object;

static uint32_t obj_max;
static object objects[OBJ_MAX];

/*
 * Prototypes.
 */
static boolean collision_check_single_object(object *, boolean *overlap);

/*
 * OpenGLES workarounds for missing glBegin glEnd
 */
#define GL_MAX_BUFFER_SIZE 16000
static GLfloat xy[GL_MAX_BUFFER_SIZE];
static GLfloat *xyp = xy;
static GLfloat *end_of_xyp = xy + GL_MAX_BUFFER_SIZE;
static GLsizei gl_state;

/*
 * OpenGLES workarounds for missing glBegin 
 */
static void Begin (GLsizei gl_enum)
{
    gl_state = gl_enum;
}

/*
 * OpenGLES workarounds for missing glEnd 
 */
static void End (void)
{
    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(2, GL_FLOAT, 0, xy);
    glDrawArrays(gl_state, 0, (int32_t)((xyp - xy) / 2));
    glDisableClientState(GL_VERTEX_ARRAY);
    xyp = xy;

    if (xyp >= end_of_xyp) {
        DIE("overflow");
    }
}

/*
 * Place lots of test objects.
 */
static void collision_init (void)
{
    /*
     * Level bounds.
     */
    int32_t W = global_config.video_gl_width;
    int32_t H = global_config.video_gl_height;
    int32_t w = W - OBJ_MAX_RADIUS * 4;
    int32_t h = H - OBJ_MAX_RADIUS * 4;

    object *obj;
    uint32_t o;

    for (o = 0; o < OBJ_MAX - 4; o++) {
        /*
         * Repeat placing objects until there are no collisions.
         */
        for (;;) {
            obj = &objects[o];

            /*
             * Random placement.
             */
            fpoint at;

            at.x = rand() % w;
            at.y = rand() % h;
            at.x += OBJ_MAX_RADIUS * 2;
            at.y += OBJ_MAX_RADIUS * 2;

            /*
             * Random velocity.
             */
            fpoint velocity;

            velocity.x = ((double)((rand() % w) - (w / 2))) / (double)w;
            velocity.y = ((double)((rand() % h) - (h / 2))) / (double)h;

            /*
             * Random box or circle.
             */
            obj->box = o & 1;
            obj->box = 0;

            if (obj->box) {
                obj->shape.b.tl.x = at.x - OBJ_MAX_RADIUS / 2;
                obj->shape.b.tl.y = at.y - OBJ_MAX_RADIUS / 2;
                obj->shape.b.br.x = at.x + OBJ_MAX_RADIUS / 2;
                obj->shape.b.br.y = at.y + OBJ_MAX_RADIUS / 2;
            } else {
                obj->shape.c.radius =
                    (rand() % (OBJ_MAX_RADIUS - OBJ_MIN_RADIUS)) +
                    OBJ_MIN_RADIUS;
            }

            obj->at = at;
            obj->velocity = velocity;

            obj_max++;

            boolean overlap;

            if (!collision_check_single_object(obj, &overlap) && !overlap) {
                break;
            }

            obj_max--;
        }

        /*
         * Make some of the objects stationary.
         */
        if (!(rand() % 3)) {
            obj->is_stationary = true;
            obj->is_debug = true;
        }
    }

    /*
     * Walls.
     */
    obj++;
    obj->box = true;
    obj->shape.b.tl.x = 0;
    obj->shape.b.tl.y = H - OBJ_MIN_RADIUS;
    obj->shape.b.br.x = W;
    obj->shape.b.br.y = H;
    obj->at.x = (obj->shape.b.tl.x + obj->shape.b.br.x) / 2.0;
    obj->at.y = (obj->shape.b.tl.y + obj->shape.b.br.y) / 2.0;
    obj->is_stationary = true;

    obj++;
    obj->box = true;
    obj->shape.b.tl.x = 0;
    obj->shape.b.tl.y = 0;
    obj->shape.b.br.x = OBJ_MIN_RADIUS;
    obj->shape.b.br.y = H;
    obj->at.x = (obj->shape.b.tl.x + obj->shape.b.br.x) / 2.0;
    obj->at.y = (obj->shape.b.tl.y + obj->shape.b.br.y) / 2.0;
    obj->is_stationary = true;

    obj++;
    obj->box = true;
    obj->shape.b.tl.x = W - OBJ_MIN_RADIUS;
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
    obj->shape.b.br.y = OBJ_MIN_RADIUS;
    obj->at.x = (obj->shape.b.tl.x + obj->shape.b.br.x) / 2.0;
    obj->at.y = (obj->shape.b.tl.y + obj->shape.b.br.y) / 2.0;
    obj->is_stationary = true;
}

/*
 * Draw all objects
 */
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
            /*
             * Box.
             */
            Vertex2f(obj->shape.b.tl.x, obj->shape.b.tl.y);
            Vertex2f(obj->shape.b.br.x, obj->shape.b.tl.y);
            Vertex2f(obj->shape.b.br.x, obj->shape.b.br.y);
            Vertex2f(obj->shape.b.tl.x, obj->shape.b.br.y);
        } else {
            /*
             * Circle.
             */
            const double rad_step = RAD_360 / 360.0;
 
            for (int i = 0; i < 360; i++) {
                double rad = ((double) i) * rad_step;
                double radius = obj->shape.c.radius;

                Vertex2f(obj->at.x + (cos(rad) * radius),
                         obj->at.y + (sin(rad) * radius));
            }
        }

        End();
    }
}

/*
 * If two circles collide, the resultant direction is along the normal between
 * the two center of masses of the circles.
 */
static boolean circle_circle_collision (object *A, object *B, fpoint *normal)
{
    circle *CA = &A->shape.c;
    circle *CB = &B->shape.c;

    fpoint n = fsub(B->at, A->at);
 
    double touching_dist = CA->radius + CB->radius;
    double dist = flength(n);

    /*
     * Circles are not touching
     */
    if (dist > touching_dist) {
        return (false);
    }
 
    /*
     * Circles are centered on each other
     */
    if (dist == 0.0) {
        /*
         * Velocity should be limited to prevent this.
         */
        normal->x = 0;
        normal->y = -1;

        return (true);
    }

    *normal = fdiv(touching_dist, n);

    return (true);
}

/*
 * See if this object collides with anything.
 */
static boolean collision_check_single_object (object *A, boolean *overlap)
{
    int32_t W = global_config.video_gl_width;
    int32_t H = global_config.video_gl_height;
    double radius = A->shape.c.radius;
    double vA = flength(A->velocity);
    boolean collision;
    uint32_t b;

    collision = false;

    if (overlap) {
        *overlap = false;
    }

    A->impulse.x = 0;
    A->impulse.y = 0;

    /*
     * Edge hits ?
     */
    if (A->at.y + radius > H - OBJ_MIN_RADIUS) {
        A->impulse.y += -A->velocity.y * LOSS_OF_ENERGY;
        collision = true;
    }

    if (A->at.x + radius > W - OBJ_MIN_RADIUS) {
        A->impulse.x += -A->velocity.x * LOSS_OF_ENERGY;
        collision = true;
    }

    if (A->at.x - radius < OBJ_MIN_RADIUS) {
        A->impulse.x += -A->velocity.x * LOSS_OF_ENERGY;
        collision = true;
    }

    for (b = 0; b < obj_max; b++) {
        object *B;

        B = &objects[b];

        if (A == B) {
            continue;
        }

        fpoint normal;

        if (A->box && B->box) {
            /*
             * Box v box.
             */
        } else if (!A->box && B->box) {
            /*
             * Circle v box.
             */
        } else if (A->box && !B->box) {
            /*
             * Box v circle.
             */
        } else if (!A->box && !B->box) {
            if (circle_circle_collision(A, B, &normal)) {
                if (overlap) {
                    *overlap = true;
                }

#if 0
                glcolor(RED);

                Begin(GL_LINES);
                    Vertex2f(A->at.x, A->at.y);

                    Vertex2f(A->at.x + normal.x * (double)OBJ_RADIUS*10,
                             A->at.y + normal.y * (double)OBJ_RADIUS*10);

                    Vertex2f(A->at.x, A->at.y);

                    Vertex2f(A->at.x + A->velocity.x * (double)OBJ_RADIUS*10,
                             A->at.y + A->velocity.y * (double)OBJ_RADIUS*10);
                End();
#endif
                fpoint relative_velocity = fsub(B->at, A->at);

                double cross = fcross(relative_velocity, normal);
                if (cross > 0.0) {
                    /*
                     * Heading apart.
                     */
                    continue;
                }

                A->impulse.x += -normal.x * vA * LOSS_OF_ENERGY;
                A->impulse.y += -normal.y * vA * LOSS_OF_ENERGY;

                collision = true;
            }
        }
    }

    if (collision) {
        if (flength(A->impulse) > 0.0) {
            A->impulse = funit(A->impulse);

            A->impulse = fmul(vA * LOSS_OF_ENERGY, A->impulse);
        }
    }

    return (collision);
}

/*
 * Check for collisions for all objects. If none, then move according to 
 * velocity.
 */
static void collision_all_check (void)
{
    /*
     * How many small steps we will take for this object.
     */
    uint32_t timestep;
    uint32_t a;

    for (timestep = 0; timestep < MAX_TIMESTEP; timestep++) {

        for (a = 0; a < obj_max; a++) {
            object *A;

            A = &objects[a];

            if (A->is_stationary) {
                continue;
            }

            /*
             * Gravity.
             */
            A->accel.x = 0;
            A->accel.y = GRAVITY;

            /*
             * Gravity in small steps.
             */
            fpoint dA = A->accel;
            fdiv(MAX_TIMESTEP, dA);

            A->velocity.x += dA.x;
            A->velocity.y += dA.y;

            /*
             * Check for maxium air speed.
             */
            double len_velocity = flength(A->velocity);
            if (len_velocity > MAX_VELOCITY) {
                fpoint unit_velocity = funit(A->velocity);

                A->velocity = fmul(MAX_VELOCITY, unit_velocity);
            }

            /*
             * Velocity in small steps.
             */
            fpoint dV = A->velocity;
            fdiv(MAX_TIMESTEP, dV);

            /*
             * Save the old location.
             */
            A->old_at = A->at;

            /*
             * Creep forward.
             */
            A->at.x += dV.x;
            A->at.y += dV.y;
        }

        for (a = 0; a < obj_max; a++) {
            object *A;

            A = &objects[a];

            if (A->is_stationary) {
                continue;
            }

            /*
             * If we impact at the new location, move back to where we were.
             */
            boolean overlap;

            if (collision_check_single_object(A, &overlap)) {
                A->at = A->old_at;

                A->velocity = A->impulse;
            }

            if (overlap ) {
                A->at = A->old_at;
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

    collision_all_check();
    collision_draw();

    SDL_Delay(5);
}
