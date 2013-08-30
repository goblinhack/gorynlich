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
#include "ttf.h"

/*
 * Settings.
 */
static double GRAVITY                   = 0.01;
static double COLLISION_ELASTICITY      = 0.8;
static double MAX_VELOCITY              = 2.0;
static double MAX_TIMESTEP              = 2.0;
static const uint32_t OBJ_MIN_RADIUS    = 5;
static const uint32_t OBJ_MAX_RADIUS    = 100;
static const uint32_t OBJ_MAX           = 8;

typedef struct {
    fpoint br;
    fpoint tl;
} box;

typedef struct {
    double radius;
} circle;

typedef struct object_ {
    fpoint at;
    fpoint old_at;
    fpoint velocity;
    fpoint accel;
    double mass;
    union {
        box b;
        circle c;
    } shape;
    boolean box:1;
    boolean is_stationary:1;
    boolean is_debug:1;
    boolean collided:1;
} object;

static uint32_t obj_max;
static object objects[OBJ_MAX];

/*
 * Prototypes.
 */
static boolean collision_check_single_object(int a, object *,
                                             boolean);

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
                obj->mass = OBJ_MAX_RADIUS * OBJ_MAX_RADIUS;
            } else {
                obj->shape.c.radius =
                    (rand() % (OBJ_MAX_RADIUS - OBJ_MIN_RADIUS)) +
                    OBJ_MIN_RADIUS;

                obj->mass = PI * obj->shape.c.radius * obj->shape.c.radius;
                obj->mass *= obj->mass;
            }

            obj->at = at;
            obj->velocity = velocity;

            obj_max++;

            if (!collision_check_single_object(0, obj, true)) {
                break;
            }

            obj_max--;
        }

        /*
         * Make some of the objects stationary.
         */
        if (!(rand() % 3)) {
            obj->is_stationary = true;
            obj->velocity.x = 0;
            obj->velocity.y = 0;
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
    obj->velocity.x = 0;
    obj->velocity.y = 0;
    obj->is_stationary = true;

    obj++;
    obj->box = true;
    obj->shape.b.tl.x = 0;
    obj->shape.b.tl.y = 0;
    obj->shape.b.br.x = OBJ_MIN_RADIUS;
    obj->shape.b.br.y = H;
    obj->at.x = (obj->shape.b.tl.x + obj->shape.b.br.x) / 2.0;
    obj->at.y = (obj->shape.b.tl.y + obj->shape.b.br.y) / 2.0;
    obj->velocity.x = 0;
    obj->velocity.y = 0;
    obj->is_stationary = true;

    obj++;
    obj->box = true;
    obj->shape.b.tl.x = W - OBJ_MIN_RADIUS;
    obj->shape.b.tl.y = 0;
    obj->shape.b.br.x = W;
    obj->shape.b.br.y = H;
    obj->at.x = (obj->shape.b.tl.x + obj->shape.b.br.x) / 2.0;
    obj->at.y = (obj->shape.b.tl.y + obj->shape.b.br.y) / 2.0;
    obj->velocity.x = 0;
    obj->velocity.y = 0;
    obj->is_stationary = true;

    obj++;
    obj->box = true;
    obj->shape.b.tl.x = 0;
    obj->shape.b.tl.y = 0;
    obj->shape.b.br.x = W;
    obj->shape.b.br.y = OBJ_MIN_RADIUS;
    obj->at.x = (obj->shape.b.tl.x + obj->shape.b.br.x) / 2.0;
    obj->at.y = (obj->shape.b.tl.y + obj->shape.b.br.y) / 2.0;
    obj->velocity.x = 0;
    obj->velocity.y = 0;
    obj->is_stationary = true;

#if 0
    memset(&objects[0], 0, sizeof(obj[0]));
    memset(&objects[1], 0, sizeof(obj[1]));

    objects[0].at.x = W / 2 - W / 4;
    objects[0].at.y = H / 2;
    objects[0].velocity.x = -3;
    objects[0].velocity.x = 0;
    objects[0].velocity.y = 0;
    objects[0].mass = OBJ_MAX_RADIUS/2;
    objects[0].shape.c.radius = OBJ_MAX_RADIUS/2;
    objects[0].is_stationary = true;

    objects[1].at.x = W / 2 + W / 4;
    objects[1].at.y = H / 2;
    objects[1].velocity.x = -3;
    objects[1].velocity.y = 0;
    objects[1].mass = OBJ_MAX_RADIUS;
    objects[1].shape.c.radius = OBJ_MAX_RADIUS;
#endif
}

/*
 * Draw all objects
 */
static void collision_draw (void)
{
    uint32_t o;

    glLineWidth(0.1);
    glcolor(WHITE);

    for (o = 0; o < OBJ_MAX; o++) {
        object *obj;

        obj = &objects[o];

        if (obj->is_debug) {
            glcolor(RED);
        } else {
            glcolor(WHITE);
        }

        glBindTexture(GL_TEXTURE_2D, 0);

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

        char tmp[10];
        sprintf(tmp, "%d", o);

        ttf_puts(small_font, tmp, obj->at.x, obj->at.y, 1.0, 1.0, true);
    }
}

/*
 * If two circles collide, the resultant direction is along the normal between
 * the two center of masses of the circles.
 */
static boolean circle_circle_collision (object *A, object *B)
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
        return (true);
    }

    return (true);
}

/*
 * See if this object collides with anything.
 */
static boolean collision_check_single_object (int a, object *A,
                                              boolean check_only)
{
    int32_t W = global_config.video_gl_width;
    int32_t H = global_config.video_gl_height;
    double radius = A->shape.c.radius;
    uint32_t hit;
    uint32_t b;

    if (A->is_stationary) {
        return (false);
    }

    hit = 0;

    /*
     * Edge hits ?
     */
    if (A->at.y - radius < OBJ_MIN_RADIUS) {
        if (check_only) {
            return (true);
        }

        A->at.y = A->old_at.y;
        A->velocity.y = -A->velocity.y;
        A->velocity = fmul(COLLISION_ELASTICITY, A->velocity);
        hit++;
    }

    if (A->at.y + radius > H - OBJ_MIN_RADIUS) {
        if (check_only) {
            return (true);
        }

        A->at.y = A->old_at.y;
        A->velocity.y = -A->velocity.y;
        A->velocity = fmul(COLLISION_ELASTICITY, A->velocity);
        hit++;
    }

    if (A->at.x + radius > W - OBJ_MIN_RADIUS) {
        if (check_only) {
            return (true);
        }

        A->at.x = A->old_at.x;
        A->velocity.x = -A->velocity.x;
        A->velocity = fmul(COLLISION_ELASTICITY, A->velocity);
        hit++;
    }

    if (A->at.x - radius < OBJ_MIN_RADIUS) {
        if (check_only) {
            return (true);
        }

        A->at.x = A->old_at.x;
        A->velocity.x = -A->velocity.x;
        A->velocity = fmul(COLLISION_ELASTICITY, A->velocity);
        hit++;
    }

    for (b = 0; b < obj_max; b++) {
        object *B;

        B = &objects[b];

        if (A == B) {
            continue;
        }

        boolean collided = false;

        if (A->box && B->box) {
            /*
             * Box v box.
             */
            hit++;
            collided = true;
        } else if (!A->box && B->box) {
            /*
             * Circle v box.
             */
            hit++;
            collided = true;
        } else if (A->box && !B->box) {
            /*
             * Box v circle.
             */
            hit++;
            collided = true;
        } else if (!A->box && !B->box) {

            if (circle_circle_collision(A, B)) {
                hit++;
                collided = true;
            }
        }

        if (!collided) {
            continue;
        }

        if (check_only) {
            return (true);
        }

        /*
         * Normal vector is a line between the two center of masses.
         * Tangent vector is at 90 degrees to this.
         */
        fpoint normal = fsub(B->at, A->at);
        fpoint normal_unit = funit(normal);
        fpoint tangent_unit = { -normal_unit.y, normal_unit.x };

        double mA = A->mass;
        double mB = A->mass;

        fpoint vA = A->velocity;
        fpoint vB = B->velocity;

        if (B->is_stationary) {
            mB = mA;
            vB = fmul(-1, vA);
        }

        /*
         * Project the velocity onto the normal vectors.
         */
        double normal_A_len = fdot(normal_unit, vA);
        double normal_B_len = fdot(normal_unit, vB);

        double tangent_A_len = fdot(tangent_unit, vA);
        double tangent_B_len = fdot(tangent_unit, vB);
        
        /*
         * Tangent velocity doesn't change.after collision.
         */
        double tangent_A_velocity = tangent_A_len;
        double tangent_B_velocity = tangent_B_len;
        
        /*
         * Do one dimensional elastic collision.
         */
        double normal_A_velocity =
            (normal_A_len*(mA - mB) + 2.0 * mB*normal_B_len) / (mA + mB);

        double normal_B_velocity =
            (normal_B_len*(mB - mA) + 2.0 * mA*normal_A_len) / (mA + mB);

        fpoint normal_velocity_A  = fmul(normal_A_velocity, normal_unit);
        fpoint tangent_velocity_A = fmul(tangent_A_velocity, tangent_unit);

        fpoint normal_velocity_B  = fmul(normal_B_velocity, normal_unit);
        fpoint tangent_velocity_B = fmul(tangent_B_velocity, tangent_unit);

        if (!A->is_stationary) {
            A->velocity.x = normal_velocity_A.x + tangent_velocity_A.x;
            A->velocity.y = normal_velocity_A.y + tangent_velocity_A.y;
        }

        if (!B->is_stationary) {
            B->velocity.x = normal_velocity_B.x + tangent_velocity_B.x;
            B->velocity.y = normal_velocity_B.y + tangent_velocity_B.y;
        }

        A->velocity = fmul(COLLISION_ELASTICITY, A->velocity);
        B->velocity = fmul(COLLISION_ELASTICITY, B->velocity);

        /*
         * Try A out in the new position.
         */
        fpoint dVa = A->velocity;
        dVa = fdiv(MAX_TIMESTEP, dVa);

        A->at = A->old_at;
        A->at.x += dVa.x;
        A->at.y += dVa.y;

        if (collision_check_single_object(a, A, true)) {
            /*
             * Hit another object. Move back.
             */
            A->at = A->old_at;
        }

        if (!B->is_stationary) {
            /*
             * Try B out in the new position.
             */
            fpoint dVb = B->velocity;
            dVb = fdiv(MAX_TIMESTEP, dVb);

            B->at = B->old_at;
            B->at.x += dVb.x;
            B->at.y += dVb.y;

            if (collision_check_single_object(b, B, true)) {
                /*
                * Hit another object. Move back.
                */
                B->at = B->old_at;
            }
        }

        glcolor(RED);
        glBindTexture(GL_TEXTURE_2D, 0);

        Begin(GL_LINES);
            Vertex2f(A->at.x, A->at.y);

            Vertex2f(A->at.x + normal_unit.x * (double)OBJ_MAX_RADIUS,
                        A->at.y + normal_unit.y * (double)OBJ_MAX_RADIUS);
        End();

        Begin(GL_LINES);
            Vertex2f(A->at.x, A->at.y);

            Vertex2f(A->at.x + tangent_unit.x * (double)OBJ_MAX_RADIUS,
                        A->at.y + tangent_unit.y * (double)OBJ_MAX_RADIUS);
        End();
    }

    return (hit > 0);
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
            dA = fdiv(MAX_TIMESTEP, dA);

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
            dV = fdiv(MAX_TIMESTEP, dV);

            /*
             * Save the old location.
             */
            A->old_at = A->at;

            /*
             * Creep forward.
             */
            A->at.x += dV.x;
            A->at.y += dV.y;

            A->collided = false;

            /*
             * If we impact at the new location, move back to where we were.
             */
            if (collision_check_single_object(a, A, false)) {
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
