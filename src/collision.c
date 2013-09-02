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
static double GRAVITY                   = 0.04;
static double COLLISION_ELASTICITY      = 0.5;
static double FRICTION                  = 0.5;
static double TANGENT_ELASTICITY        = 1.0;
static double MAX_VELOCITY              = 5.0;
static double MAX_TIMESTEP              = 5.0;
static const uint32_t OBJ_MIN_RADIUS    = 10;
static const uint32_t OBJ_MAX_RADIUS    = 60;
static const uint32_t OBJ_MAX           = 50;

int pause = 0;
typedef struct {
    double width;
    double height;
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
    double rot;
    union {
        box b;
        circle c;
    } shape;
    boolean box:1;
    boolean moved:1;
    boolean is_stationary:1;
    boolean is_debug:1;
    boolean collided:1;
} object;

static uint32_t obj_max;
static object objects[OBJ_MAX];

/*
 * Prototypes.
 */
static boolean collision_check_single_object(object *, object *, boolean);

/*
 * OpenGLES workarounds for missing glBegin glEnd
 */
#define GL_MAX_BUFFER_SIZE 16000
static GLfloat xy[GL_MAX_BUFFER_SIZE];
static GLfloat *xyp = xy;
static GLfloat *end_of_xyp = xy + GL_MAX_BUFFER_SIZE;
static GLsizei gl_state;

static int x, y;
float scale = 1;

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

static void
box_to_coords (object *obj, fpoint *P0, fpoint *P1, fpoint *P2, fpoint *P3)
{
    box *box = &obj->shape.b;

    double w = box->width / 2;
    double h = box->height / 2;

    fpoint p0 = { -w, -h };
    fpoint p1 = {  w, -h };
    fpoint p2 = {  w,  h };
    fpoint p3 = { -w,  h };

    double theta = obj->rot; 

    p0 = fpoint_rotate(p0, theta);
    p1 = fpoint_rotate(p1, theta);
    p2 = fpoint_rotate(p2, theta);
    p3 = fpoint_rotate(p3, theta);

    P0->x = p0.x + obj->at.x;
    P0->y = p0.y + obj->at.y;

    P1->x = p1.x + obj->at.x;
    P1->y = p1.y + obj->at.y;

    P2->x = p2.x + obj->at.x;
    P2->y = p2.y + obj->at.y;

    P3->x = p3.x + obj->at.x;
    P3->y = p3.y + obj->at.y;
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
            obj->rot = (RAD_360 / 360.0) * ((rand() % 360));

            if ((rand() % 100) < 20) {
                obj->box = 1;
            } else {
                obj->box = 0;
            }

            if (obj->box) {
                double w = 
                    (rand() % (OBJ_MAX_RADIUS - OBJ_MIN_RADIUS)) +
                    OBJ_MIN_RADIUS;
                double h = 
                    (rand() % (OBJ_MAX_RADIUS - OBJ_MIN_RADIUS)) +
                    OBJ_MIN_RADIUS;

                w = w * 2;
                h = h * 2;

                obj->shape.b.width = w;
                obj->shape.b.height = h;
                obj->mass = w * h;
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

            if (!collision_check_single_object(obj, 0, true)) {
                break;
            }

            obj_max--;
        }

        /*
         * Make some of the objects stationary.
         */
        if ((rand() % 100) < 60) {
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
    obj->shape.b.width = OBJ_MIN_RADIUS;
    obj->shape.b.height = H;
    obj->at.x = OBJ_MIN_RADIUS / 2;
    obj->at.y = H / 2;
    obj->velocity.x = 0;
    obj->velocity.y = 0;
    obj->is_stationary = true;

    obj++;
    obj->box = true;
    obj->shape.b.width = OBJ_MIN_RADIUS;
    obj->shape.b.height = H;
    obj->at.x = W - OBJ_MIN_RADIUS / 2;
    obj->at.y = H / 2;
    obj->velocity.x = 0;
    obj->velocity.y = 0;
    obj->is_stationary = true;

    obj++;
    obj->box = true;
    obj->shape.b.width = W;
    obj->shape.b.height = OBJ_MIN_RADIUS;
    obj->at.x = W / 2;
    obj->at.y = OBJ_MIN_RADIUS / 2;
    obj->velocity.x = 0;
    obj->velocity.y = 0;
    obj->is_stationary = true;

    obj++;
    obj->box = true;
    obj->shape.b.width = W;
    obj->shape.b.height = OBJ_MIN_RADIUS;
    obj->at.x = W / 2;
    obj->at.y = H - OBJ_MIN_RADIUS / 2;
    obj->velocity.x = 0;
    obj->velocity.y = 0;
    obj->is_stationary = true;
}

/*
 * Draw all objects
 */
static void collision_draw (void)
{
    uint32_t o;

    glLineWidth(0.01);
    glcolor(WHITE);

    SDL_GetMouseState(&x, &y);

    glPushMatrix();
//    glTranslatef((float) -x*scale, (float) -y*scale, 0);
//    glScalef(scale, scale, 0);

    for (o = 0; o < OBJ_MAX; o++) {
        object *obj;

        obj = &objects[o];

        if (obj->is_debug || obj->is_stationary) {
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
            fpoint P0, P1, P2, P3;

            box_to_coords(obj, &P0, &P1, &P2, &P3);

            Vertex2f(P0.x, P0.y);
            Vertex2f(P1.x, P1.y);
            Vertex2f(P2.x, P2.y);
            Vertex2f(P3.x, P3.y);
        } else {
            /*
             * Circle.
             */
            const double rad_step = (RAD_360 / 360.0) * 10;
 
            for (int i = 0; i < 360; i++) {
                double rad = ((double) i) * rad_step;
                double radius = obj->shape.c.radius;

                Vertex2f(obj->at.x + (cos(rad) * radius),
                         obj->at.y + (sin(rad) * radius));
            }
        }

        End();

#if 1
        char tmp[10];
        sprintf(tmp, "%d", o);

        ttf_puts(small_font, tmp, obj->at.x, obj->at.y, 1.0, 1.0, true);
        glBindTexture(GL_TEXTURE_2D, 0);
#endif
    }

    glPopMatrix();
}

static boolean circle_box_collision (object *C, object *B,
                                     fpoint *normal,
                                     fpoint *intersect,
                                     boolean check_only)
{
    circle *circle = &C->shape.c;
    fpoint P0, P1, P2, P3;
    double dist;

    box_to_coords(B, &P0, &P1, &P2, &P3);

    fpoint P = { C->at.x, C->at.y };

    if (fdist(P, P0) < circle->radius) {
        goto collided;
    }

    if (fdist(P, P1) < circle->radius) {
        goto collided;
    }

    if (fdist(P, P2) < circle->radius) {
        goto collided;
    }

    if (fdist(P, P3) < circle->radius) {
        goto collided;
    }

    if (fpoint_dist_line(P, P0, P1, &dist)) {
        if (dist < circle->radius) {
            goto collided;
        }
    }

    if (fpoint_dist_line(P, P1, P2, &dist)) {
        if (dist < circle->radius) {
            goto collided;
        }
    }

    if (fpoint_dist_line(P, P2, P3, &dist)) {
        if (dist < circle->radius) {
            goto collided;
        }
    }

    if (fpoint_dist_line(P, P3, P0, &dist)) {
        if (dist < circle->radius) {
            goto collided;
        }
    }

    return (false);

collided:

    if (check_only) {
        return (true);
    }

    fpoint delta;

    if (get_line_intersection(P, B->at, P0, P1, intersect)) {
        delta.x = P0.x - P1.x;
        delta.y = P0.y - P1.y;
        normal->x = -delta.y;
        normal->y = delta.x;
        return (true);
    }

    if (get_line_intersection(P, B->at, P1, P2, intersect)) {
        delta.x = P1.x - P2.x;
        delta.y = P1.y - P2.y;
        normal->x = -delta.y;
        normal->y = delta.x;
        return (true);
    }

    if (get_line_intersection(P, B->at, P2, P3, intersect)) {
        delta.x = P2.x - P3.x;
        delta.y = P2.y - P3.y;
        normal->x = -delta.y;
        normal->y = delta.x;
        return (true);
    }

    if (get_line_intersection(P, B->at, P3, P0, intersect)) {
        delta.x = P3.x - P0.x;
        delta.y = P3.y - P0.y;
        normal->x = -delta.y;
        normal->y = delta.x;
        return (true);
    }

    /*
     * Sphere may be inside box.
     */
    return (false);
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
    double dist_squared = n.x*n.x + n.y*n.y;

    if (dist_squared - touching_dist * touching_dist > 0) {
        /*
         * Circles are not touching
         */
        return (false);
    }
 
    /*
     * Circles are centered on each other
     */
    if (dist_squared == 0.0) {
        return (true);
    }

    return (true);
}

static void collision_objects_try_to_move (object *A, fpoint dVa,
                                           object *B, fpoint dVb)
{
    if (!A->moved) {
        if (!A->is_stationary) {
            dVa = fdiv(MAX_TIMESTEP, dVa);

            A->at.x += dVa.x;
            A->at.y += dVa.y;
        }
    }

    if (!B->moved) {
        if (!B->is_stationary) {
            /*
             * Try B out in the new position.
             */
            dVb = fdiv(MAX_TIMESTEP, dVb);

            B->at.x += dVb.x;
            B->at.y += dVb.y;
        }
    }

    if (!A->moved) {
        if (!A->is_stationary) {
            if (collision_check_single_object(A, B, true)) {
                /*
                 * Hit another object. Move back.
                 */
                A->at = A->old_at;
            } else {
                A->old_at = A->at;
                A->moved = true;
            }
        }
    }

    if (!B->moved) {
        if (!B->is_stationary) {
            if (collision_check_single_object(B, A, true)) {
                /*
                 * Hit another object. Move back.
                 */
                B->at = B->old_at;
            } else {
                B->old_at = B->at;
                B->moved = true;
            }
        }
    }

    if (!A->moved && A->box) {
        A->rot += (RAD_360 / 360.0);

        if (!A->is_stationary) {
            if (collision_check_single_object(A, B, true)) {
                /*
                 * Hit another object. Move back.
                 */
                A->at = A->old_at;

                A->rot -= (RAD_360 / 360.0);
            } else {
                A->old_at = A->at;
                A->moved = true;
            }
        }
    }

    if (!B->moved && B->box) {
        B->rot += (RAD_360 / 360.0);

        if (!B->is_stationary) {
            if (collision_check_single_object(B, A, true)) {
                /*
                 * Hit another object. Move back.
                 */
                B->at = B->old_at;

                B->rot -= (RAD_360 / 360.0);
            } else {
                B->old_at = B->at;
                B->moved = true;
            }
        }
    }

    if (!A->moved && A->box) {
        A->rot -= (RAD_360 / 360.0);

        if (!A->is_stationary) {
            if (collision_check_single_object(A, B, true)) {
                /*
                 * Hit another object. Move back.
                 */
                A->at = A->old_at;

                A->rot += (RAD_360 / 360.0);
            } else {
                A->old_at = A->at;
                A->moved = true;
            }
        }
    }

    if (!B->moved && B->box) {
        B->rot -= (RAD_360 / 360.0);

        if (!B->is_stationary) {
            if (collision_check_single_object(B, A, true)) {
                /*
                 * Hit another object. Move back.
                 */
                B->at = B->old_at;

                B->rot += (RAD_360 / 360.0);
            } else {
                B->old_at = B->at;
                B->moved = true;
            }
        }
    }
}

/*
 * See if this object collides with anything.
 */
static boolean collision_check_single_object (object *A,
                                              object *ignore,
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
        A->velocity.y *= COLLISION_ELASTICITY;
        A->velocity.x *= TANGENT_ELASTICITY;
        hit++;
    }

    if (A->at.y + radius > H - OBJ_MIN_RADIUS) {
        if (check_only) {
            return (true);
        }

        A->at.y = A->old_at.y;
        A->velocity.y = -A->velocity.y;
        A->velocity.y *= COLLISION_ELASTICITY;
        A->velocity.x *= TANGENT_ELASTICITY;
        hit++;
    }

    if (A->at.x + radius > W - OBJ_MIN_RADIUS) {
        if (check_only) {
            return (true);
        }

        A->at.x = A->old_at.x;
        A->velocity.x = -A->velocity.x;
        A->velocity.x *= COLLISION_ELASTICITY;
        A->velocity.y *= TANGENT_ELASTICITY;
        hit++;
    }

    if (A->at.x - radius < OBJ_MIN_RADIUS) {
        if (check_only) {
            return (true);
        }

        A->at.x = A->old_at.x;
        A->velocity.x = -A->velocity.x;
        A->velocity.x *= COLLISION_ELASTICITY;
        A->velocity.y *= TANGENT_ELASTICITY;
        hit++;
    }

    for (b = 0; b < obj_max; b++) {
        object *B;

        B = &objects[b];

        if (A == B) {
            continue;
        }

        if (A == ignore) {
            continue;
        }

        boolean collided = false;
        fpoint intersect = {0,0};
        fpoint normal = {0,0};

        if (A->box && B->box) {
            /*
             * Box v box.
             */
//            hit++;
            collided = false;
        } else if (!A->box && B->box) {
            /*
             * Circle v box.
             */
            if (circle_box_collision(A, /* circle */
                                     B, /* box */
                                     &normal, &intersect, check_only)) {
                hit++;
                collided = true;
            }
        } else if (A->box && !B->box) {
            /*
             * Box v circle.
             */
            if (circle_box_collision(B, /* circle */
                                     A, /* box */
                                     &normal, &intersect, check_only)) {
                normal = fsub(intersect, B->at);
                hit++;
                collided = true;
            }
        } else if (!A->box && !B->box) {

            if (circle_circle_collision(A, B)) {
                normal = fsub(B->at, A->at);
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

        A->at = A->old_at;

        /*
         * Normal vector is a line between the two center of masses.
         * Tangent vector is at 90 degrees to this.
         */
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

        B->at.y += 0.1;
        if (collision_check_single_object(B, 0, true)) {
            mB = mA;
            vB.y += -vA.y;

        }
        B->at.y -= 0.1;

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

#if 0
glcolor(ORANGE);
Begin(GL_LINES);
    Vertex2f(A->at.x, A->at.y);
    Vertex2f(A->at.x + normal_velocity_A.x*OBJ_MAX_RADIUS, A->at.y + normal_velocity_A.y*OBJ_MAX_RADIUS);
End();
glcolor(CYAN);
Begin(GL_LINES);
    Vertex2f(A->at.x, A->at.y);
    Vertex2f(A->at.x + tangent_velocity_A.x*OBJ_MAX_RADIUS, A->at.y + tangent_velocity_A.y*OBJ_MAX_RADIUS);
End();
#endif
        normal_velocity_A = fmul(COLLISION_ELASTICITY, normal_velocity_A);
        normal_velocity_B = fmul(COLLISION_ELASTICITY, normal_velocity_B);
        tangent_velocity_A = fmul(TANGENT_ELASTICITY, tangent_velocity_A);
        tangent_velocity_B = fmul(TANGENT_ELASTICITY, tangent_velocity_B);

        if (!A->is_stationary) {
            A->velocity.x = normal_velocity_A.x + tangent_velocity_A.x;
            A->velocity.y = normal_velocity_A.y + tangent_velocity_A.y;
        }

        if (!B->is_stationary) {
            B->velocity.x = normal_velocity_B.x + tangent_velocity_B.x;
            B->velocity.y = normal_velocity_B.y + tangent_velocity_B.y;
        }

        A->moved = false;
        B->moved = false;

        collision_objects_try_to_move(A, A->velocity,
                                      B, B->velocity);
        collision_objects_try_to_move(A, tangent_velocity_A,
                                      B, tangent_velocity_B);
        collision_objects_try_to_move(A, normal_velocity_A,
                                      B, normal_velocity_B);

        if (!A->moved) {
            A->velocity.x *= FRICTION;
            A->velocity.y *= FRICTION;
        }

        if (!B->moved) {
            B->velocity.x *= FRICTION;
            B->velocity.y *= FRICTION;
        }

#if 0
glcolor(ORANGE);
Begin(GL_LINES);
    Vertex2f(A->at.x, A->at.y);
    Vertex2f(A->at.x + A->velocity.x*OBJ_MIN_RADIUS, A->at.y + A->velocity.y*OBJ_MIN_RADIUS);
End();
#endif

#if 0
        glcolor(RED);

        Begin(GL_LINES);
            Vertex2f(A->at.x, A->at.y);

            Vertex2f(A->at.x + tangent_unit.x * (double)OBJ_MAX_RADIUS,
                     A->at.y + tangent_unit.y * (double)OBJ_MAX_RADIUS);
        End();

        Begin(GL_LINES);
            Vertex2f(A->at.x, A->at.y);

            Vertex2f(A->at.x + normal.x * (double)OBJ_MAX_RADIUS,
                     A->at.y + normal.y * (double)OBJ_MAX_RADIUS);
        End();
#endif
#if 0
        glPushMatrix();
//        glTranslatef((float) -x*scale, (float) -y*scale, 0);
//        glScalef(scale, scale, 0);

        glBindTexture(GL_TEXTURE_2D, 0);

        glLineWidth(1.1);


        glcolor(GREEN);

        Begin(GL_LINES);
            Vertex2f(A->at.x, A->at.y);

            Vertex2f(A->at.x + A->velocity.x * (double)OBJ_MAX_RADIUS,
                     A->at.y + A->velocity.y * (double)OBJ_MAX_RADIUS);
        End();

#if 0
        glcolor(RED);

        Begin(GL_LINES);
            Vertex2f(B->at.x, B->at.y);

            Vertex2f(B->at.x + tangent_unit.x * (double)OBJ_MAX_RADIUS,
                     B->at.y + tangent_unit.y * (double)OBJ_MAX_RADIUS);
        End();
#endif

        glcolor(BLUE);

        Begin(GL_LINES);
            Vertex2f(B->at.x, B->at.y);

            Vertex2f(B->at.x + B->velocity.x * (double)OBJ_MAX_RADIUS,
                     B->at.y + B->velocity.y * (double)OBJ_MAX_RADIUS);
        End();

#endif
        glPopMatrix();

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
            if (collision_check_single_object(A, 0, false)) {
            }
        }
    }
}

void collision_test (void)
{
    static boolean first = true;
pause = 0;
    if (first) {
        first = false;
        collision_init();
        return;
    }

    collision_all_check();
    collision_draw();
if (pause) {
    SDL_Delay(1000);
}

    SDL_Delay(10);
}
