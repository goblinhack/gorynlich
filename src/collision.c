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
#if 1
static double GRAVITY                   = 1.0;
static double COLLISION_ELASTICITY      = 0.5;
static double TANGENT_ELASTICITY        = 0.95;
#else
static double GRAVITY                   = 0.00;
static double COLLISION_ELASTICITY      = 1.0;
static double TANGENT_ELASTICITY        = 1.0;
#endif
static double SPIN_LOSS                 = 1.0;
static double MAX_VELOCITY              = 200.0;
static uint32_t MAX_TIMESTEP            = 8;
static const uint32_t OBJ_MIN_RADIUS    = 20;
static const uint32_t OBJ_MAX_RADIUS    = 160;
static const uint32_t OBJ_MAX           = 8;

static uint32_t PERCENTAGE_STATIONARY   = 0;
static uint32_t PERCENTAGE_BOXES        = 50;

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
    double old_rot;
    fpoint old_velocity;
    fpoint velocity;
    fpoint accel;
    double mass;
    double rot;
    double spin;
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
static boolean collision_check_single_object(object *, boolean);

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

static void
rotate_about (object *obj, fpoint P0, double theta)
{
    obj->rot += theta;

    fpoint at = obj->at;

    at.x -= P0.x;
    at.y -= P0.y;

    at = fpoint_rotate(at, theta);

    at.x += P0.x;
    at.y += P0.y;

    obj->at = at;
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
            velocity.x *= 50;
            velocity.y *= 50;

            /*
             * Random box or circle.
             */
            obj->rot = (RAD_360 / 360.0) * ((rand() % 360));

            if ((rand() % 100) < PERCENTAGE_BOXES) {
                obj->box = 1;
            } else {
                obj->box = 0;
            }
                obj->box = 1;

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

            if (!collision_check_single_object(obj, true)) {
                break;
            }

            obj_max--;
        }

        /*
         * Make some of the objects stationary.
         */
        if ((rand() % 100) < PERCENTAGE_STATIONARY) {
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
    obj_max++;

    obj++;
    obj->box = true;
    obj->shape.b.width = OBJ_MIN_RADIUS;
    obj->shape.b.height = H;
    obj->at.x = W - OBJ_MIN_RADIUS / 2;
    obj->at.y = H / 2;
    obj->velocity.x = 0;
    obj->velocity.y = 0;
    obj->is_stationary = true;
    obj_max++;

    obj++;
    obj->box = true;
    obj->shape.b.width = W;
    obj->shape.b.height = OBJ_MIN_RADIUS;
    obj->at.x = W / 2;
    obj->at.y = OBJ_MIN_RADIUS / 2;
    obj->velocity.x = 0;
    obj->velocity.y = 0;
    obj->is_stationary = true;
    obj_max++;

    obj++;
    obj->box = true;
    obj->shape.b.width = W;
    obj->shape.b.height = OBJ_MIN_RADIUS;
    obj->at.x = W / 2;
    obj->at.y = H - OBJ_MIN_RADIUS / 2;
    obj->velocity.x = 0;
    obj->velocity.y = 0;
    obj->is_stationary = true;
    obj_max++;
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
    glScalef(scale, scale, 0);

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
        char tmp[40];
//        sprintf(tmp, "%d %f %f", o, obj->at.x, obj->at.y);
//        sprintf(tmp, "%d %f", o, obj->at.x);
        sprintf(tmp, "%d", o);

        ttf_puts(small_font, tmp, obj->at.x, obj->at.y, 1.0, 1.0, true);
        glBindTexture(GL_TEXTURE_2D, 0);
#endif
    }

    glPopMatrix();
}

static boolean box_box_collision (object *A, object *B,
                                  fpoint *normal,
                                  fpoint *normal_2,
                                  fpoint *intersect,
                                  boolean check_only)
{
    fpoint A_norm[4];
    fpoint B_norm[4];
    fpoint A_vec[4];
    fpoint B_vec[4];
    fpoint A_mid[4];
    fpoint B_mid[4];
    boolean B_hit[4];
    fpoint B_intersect[4];

    fpoint AP[4];
    fpoint BP[4];

    box_to_coords(A, &AP[0], &AP[1], &AP[2], &AP[3]);

    box_to_coords(B, &BP[0], &BP[1], &BP[2], &BP[3]);

    A_mid[0].x = (AP[1].x + AP[0].x) / 2;
    A_mid[0].y = (AP[1].y + AP[0].y) / 2;
    A_mid[1].x = (AP[2].x + AP[1].x) / 2;
    A_mid[1].y = (AP[2].y + AP[1].y) / 2;
    A_mid[2].x = (AP[3].x + AP[2].x) / 2;
    A_mid[2].y = (AP[3].y + AP[2].y) / 2;
    A_mid[3].x = (AP[0].x + AP[3].x) / 2;
    A_mid[3].y = (AP[0].y + AP[3].y) / 2;

    B_mid[0].x = (BP[1].x + BP[0].x) / 2;
    B_mid[0].y = (BP[1].y + BP[0].y) / 2;
    B_mid[1].x = (BP[2].x + BP[1].x) / 2;
    B_mid[1].y = (BP[2].y + BP[1].y) / 2;
    B_mid[2].x = (BP[3].x + BP[2].x) / 2;
    B_mid[2].y = (BP[3].y + BP[2].y) / 2;
    B_mid[3].x = (BP[0].x + BP[3].x) / 2;
    B_mid[3].y = (BP[0].y + BP[3].y) / 2;

    A_vec[0].x = AP[1].x - AP[0].x;
    A_vec[0].y = AP[1].y - AP[0].y;
    A_vec[1].x = AP[2].x - AP[1].x;
    A_vec[1].y = AP[2].y - AP[1].y;
    A_vec[2].x = AP[3].x - AP[2].x;
    A_vec[2].y = AP[3].y - AP[2].y;
    A_vec[3].x = AP[0].x - AP[3].x;
    A_vec[3].y = AP[0].y - AP[3].y;

    B_vec[0].x = BP[1].x - BP[0].x;
    B_vec[0].y = BP[1].y - BP[0].y;
    B_vec[1].x = BP[2].x - BP[1].x;
    B_vec[1].y = BP[2].y - BP[1].y;
    B_vec[2].x = BP[3].x - BP[2].x;
    B_vec[2].y = BP[3].y - BP[2].y;
    B_vec[3].x = BP[0].x - BP[3].x;
    B_vec[3].y = BP[0].y - BP[3].y;

    A_norm[2].x = -A_vec[0].y;
    A_norm[2].y =  A_vec[0].x;
    A_norm[3].x = -A_vec[1].y;
    A_norm[3].y =  A_vec[1].x;
    A_norm[0].x = -A_vec[2].y;
    A_norm[0].y =  A_vec[2].x;
    A_norm[1].x = -A_vec[3].y;
    A_norm[1].y =  A_vec[3].x;

    B_norm[2].x = -B_vec[0].y;
    B_norm[2].y =  B_vec[0].x;
    B_norm[3].x = -B_vec[1].y;
    B_norm[3].y =  B_vec[1].x;
    B_norm[0].x = -B_vec[2].y;
    B_norm[0].y =  B_vec[2].x;
    B_norm[1].x = -B_vec[3].y;
    B_norm[1].y =  B_vec[3].x;
  
int hit = 0;
int i;
int j;

    double mag;
    double min_mag;
    double max_mag;

    for (i = 0; i < 2; i++) {
        min_mag = 1;
        max_mag = 0;

        for (j = 0; j < 4; j++) {
            mag = fpoint_project_onto_line(AP[j], BP[(i + 1) % 4], BP[i]);
            min_mag = min(min_mag, mag);
            max_mag = max(max_mag, mag);
        }

        if (((0 < min_mag) && (min_mag < 1)) ||
            ((0 < max_mag) && (max_mag < 1)) ||
            ((min_mag <= 0) && (max_mag >= 1))) {
            hit++;
        }
    }

    for (i = 0; i < 2; i++) {
        min_mag = 1;
        max_mag = 0;

        for (j = 0; j < 4; j++) {
            mag = fpoint_project_onto_line(BP[j], AP[(i + 1) % 4], AP[i]);
            min_mag = min(min_mag, mag);
            max_mag = max(max_mag, mag);
        }

        if (((0 < min_mag) && (min_mag < 1)) ||
            ((0 < max_mag) && (max_mag < 1)) ||
            ((min_mag <= 0) && (max_mag >= 1))) {
            hit++;
        }
    }

    if (hit <= 3) {
        return (false);
    }

    if (check_only) {
        return (true);
    }

    for (j = 0; j < 4; j++) {
        B_hit[j] = false;
    }

    for (i = 0; i < 4; i++) {
        for (j = 0; j < 4; j++) {
            fpoint intersect;

            if (get_line_intersection(AP[i], AP[(i + 1) % 4],
                                      BP[j], BP[(j + 1) % 4],
                                      &intersect)) {
                if (B_hit[j] > 1) {
                    DIE("bug");
                }

                if (B_hit[j]) {
                    B_intersect[j].x += intersect.x;
                    B_intersect[j].y += intersect.y;
                    B_intersect[j].x /= 2.0;
                    B_intersect[j].y /= 2.0;
                } else {
                    B_intersect[j] = intersect;
                }

                B_hit[j]++;
            }
        }
    }

    hit = 0;
    normal->x = 0;
    normal->y = 0;

    for (j = 0; j < 4; j++) {
        if (B_hit[j]) {
            if (B_hit[(j + 2) % 4]) {
                *normal = fmul(-1, A->velocity);
                intersect->x = BP[j].x + BP[(j + 3) % 4].x;
                intersect->y = BP[j].y + BP[(j + 3) % 4].y;
                return (true);
            }

            if (hit) {
                *normal_2 = B_norm[j];
            } else {
                *normal = B_norm[j];
            }

            *intersect = B_intersect[j];

            hit++;
        }
    }

    if (hit == 2) {
        for (j = 0; j < 4; j++) {
            if (B_hit[j]) {
                if (B_hit[(j + 1) % 4]) {
                    *intersect = BP[(j + 1) % 4];
                    break;
                }
            }
        }
    }

    if (hit > 2) {
        DIE("bug");
    }

    if (!hit) {
        DIE("bug");
    }

    if (hit == 2) {
        if (fpoint_angle(A->velocity, *normal) < 
            fpoint_angle(A->velocity, *normal_2)) {
            fpoint tmp;

            tmp = *normal_2;
            *normal_2 = *normal;
            *normal = tmp;
#if 0
                glBindTexture(GL_TEXTURE_2D, 0);
                glcolor(GREEN);

                Begin(GL_LINES);
                    Vertex2f(BP[j].x,BP[j].y);
                    Vertex2f(BP[j].x+B_norm[j].x*OBJ_MIN_RADIUS,BP[j].y+B_norm[j].y*OBJ_MIN_RADIUS);
                End();
#endif
        }
    }

    return (hit);
}

static uint32_t circle_box_collision (object *A, object *B,
                                      fpoint *normal,
                                      fpoint *normal_2,
                                      fpoint *intersect,
                                      boolean check_only)
{
    circle *circle = &A->shape.c;
    fpoint P = { A->at.x, A->at.y };
    box *box = &B->shape.b;
    double dist;
    boolean B_hit[4];
    fpoint B_intersect[4];
    fpoint B_norm[4];
    fpoint BP[4];
    uint32_t j;
    uint32_t hit;
    fpoint delta;

    box_to_coords(B, &BP[0], &BP[1], &BP[2], &BP[3]);

    hit = 0;
    for (j = 0; j < 4; j++) {
        B_hit[j] = false;
    }

    fpoint_dist_line(P, BP[0], BP[1], &dist, &B_intersect[0]);
    if (dist < circle->radius) {
        if (check_only) {
            return (true);
        }

        delta.x = BP[0].x - BP[1].x;
        delta.y = BP[0].y - BP[1].y;
        B_hit[0] = true;
        B_norm[0].x = -delta.y;
        B_norm[0].y = delta.x;
        hit++;
    }

    fpoint_dist_line(P, BP[1], BP[2], &dist, &B_intersect[1]);
    if (dist < circle->radius) {
        if (check_only) {
            return (true);
        }

        delta.x = BP[1].x - BP[2].x;
        delta.y = BP[1].y - BP[2].y;
        B_hit[1] = true;
        B_norm[1].x = -delta.y;
        B_norm[1].y = delta.x;
        hit++;
    }

    fpoint_dist_line(P, BP[2], BP[3], &dist, &B_intersect[2]);
    if (dist < circle->radius) {
        if (check_only) {
            return (true);
        }

        delta.x = BP[2].x - BP[3].x;
        delta.y = BP[2].y - BP[3].y;
        B_hit[2] = true;
        B_norm[2].x = -delta.y;
        B_norm[2].y = delta.x;
        hit++;
    }

    fpoint_dist_line(P, BP[3], BP[0], &dist, &B_intersect[3]);
    if (dist < circle->radius) {
        if (check_only) {
            return (true);
        }

        delta.x = BP[3].x - BP[0].x;
        delta.y = BP[3].y - BP[0].y;
        B_hit[3] = true;
        B_norm[3].x = -delta.y;
        B_norm[3].y = delta.x;
        hit++;
    }

    /*
     * Rotate the circle center in the opposite direction of the box
     * rotation. Then we can treat the box as axis aligned by using
     * it's co-ords as (-w,-h) .. (w,h)
     */
    if (!hit) {
        double theta = -B->rot; 

        P.x -= B->at.x;
        P.y -= B->at.y;

        P = fpoint_rotate(P, theta);

        double w = box->width / 2;
        double h = box->height / 2;

        if ((P.x >= -w) && (P.x <= w) && (P.y >= -h) && (P.y <= h)) {
            /*
             * Circle is inside box.
             */
            if (check_only) {
                return (true);
            }

            normal->x = 0;
            normal->y = -1;

            return (true);
        }
    }

    hit = 0;
    normal->x = 0;
    normal->y = 0;

    for (j = 0; j < 4; j++) {
        if (B_hit[j]) {
            if (B_hit[(j + 2) % 4]) {
                *normal = fmul(-1, A->velocity);
                intersect->x = BP[j].x + BP[(j + 3) % 4].x;
                intersect->y = BP[j].y + BP[(j + 3) % 4].y;
                return (true);
            }

            if (hit) {
                *normal_2 = B_norm[j];
            } else {
                *normal = B_norm[j];
            }

            *intersect = B_intersect[j];

            hit++;
        }
    }

    if (hit == 2) {
        for (j = 0; j < 4; j++) {
            if (B_hit[j]) {
                if (B_hit[(j + 1) % 4]) {
                    *intersect = BP[(j + 1) % 4];
                    break;
                }
            }
        }
    }

    if (hit > 2) {
        DIE("bug");
    }

    if (hit == 2) {
        if (fpoint_angle(A->velocity, *normal) < 
            fpoint_angle(A->velocity, *normal_2)) {
            fpoint tmp;

            tmp = *normal_2;
            *normal_2 = *normal;
            *normal = tmp;
        }
    }

    return (hit);
}

/*
 * If two circles collide, the resultant direction is along the normal between
 * the two center of masses of the circles.
 */
static boolean circle_circle_collision (object *A, object *B,
                                        fpoint *intersect)
{
    circle *CA = &A->shape.c;
    circle *CB = &B->shape.c;

    fpoint n = fsub(B->at, A->at);
    double touching_dist = CA->radius + CB->radius;
    double dist_squared = n.x*n.x + n.y*n.y;

    double diff = dist_squared - touching_dist * touching_dist;
    if (diff >= 0.0) {
        /*
         * Circles are not touching
         */
        return (false);
    }

    diff = sqrt(fabs(diff));
    diff /= 2.0;
 
    n = funit(n);
    n = fmul(CA->radius - diff, n);
    n = fadd(A->at, n);
    *intersect = n;

    return (true);
}

static boolean collision_objects_try_to_move (object *A, fpoint dVa)
{
    if (A->is_stationary) {
        return (true);
    }

    dVa = fdiv(MAX_VELOCITY, dVa);

    fpoint old_at = A->at;

    A->at.x += dVa.x;
    A->at.y += dVa.y;

    if (collision_check_single_object(A, true)) {
        A->at = old_at;
        return (false);
    }

    return (true);
}

/*
 * See if this object collides with anything.
 */
static boolean collision_check_single_object (object *A,
                                              boolean check_only)
{
static fpoint dbg[1000];
static int dbgat;
    uint32_t hit;
    uint32_t b;

    if (A->is_stationary) {
        return (false);
    }

    hit = 0;

    fpoint old_at;

    if (check_only) {
        old_at = A->at;
    }

    for (b = 0; b < obj_max; b++) {
        object *B;

        B = &objects[b];

        if (A == B) {
            continue;
        }

        boolean collided = false;
        fpoint normal = {0,0};
        fpoint normal_2 = {0,0};
        fpoint intersect = {0,0};
        boolean normal_2_valid = false;
        double B_rot;
        double A_rot;
        uint32_t hit;

        A_rot = 0;
        B_rot = 0;

        if (A->box && B->box) {
            /*
             * Box v box.
             */
            if ((hit = box_box_collision(A, B,
                                         &normal,
                                         &normal_2,
                                         &intersect, 
                                         check_only))) {
                fpoint normal = {0,0};
                fpoint normal_2 = {0,0};
                fpoint intersect = {0,0};

                collided = true;

                box_box_collision(B, A,
                                  &normal,
                                  &normal_2,
                                  &intersect, 
                                  check_only);
            }
        } else if (!A->box && B->box) {
            /*
             * Circle v box.
             */
            if ((hit = circle_box_collision(A, /* circle */
                                            B, /* box */
                                            &normal,
                                            &normal_2,
                                            &intersect, 
                                            check_only))) {
                collided = true;
            }
        } else if (A->box && !B->box) {
            /*
             * Box v circle.
             */
            if ((hit = circle_box_collision(B, /* circle */
                                            A, /* box */
                                            &normal,
                                            &normal_2,
                                            &intersect, 
                                            check_only))) {
                normal = fmul(-1, normal);
                collided = true;
            }
        } else if (!A->box && !B->box) {

            if (circle_circle_collision(A, B, &intersect)) {
                normal = fsub(B->at, A->at);
                hit = 1;
                collided = true;
            }
        } else {
            DIE("bug");
        }

        if (!collided) {
            continue;
        }

        A->at = A->old_at;
        A->rot = A->old_rot;
        A->velocity = A->old_velocity;

        if (check_only) {
            return (true);
        }

#if 1
        dbgat++;
        if (dbgat >= ARRAY_SIZE(dbg)) {
            dbgat = 0;
        }

        dbg[dbgat].x = intersect.x;
        dbg[dbgat].y = intersect.y;
#endif

        normal_2_valid = (hit > 1);

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
            mB = 9999999999;
            vB.x = 0;
            vB.y = 0;
        } else {
#if 0
            B->at.y += 0.1;
            if (collision_check_single_object(B, true)) {
                mB = mA;
                vB.y += -GRAVITY;
            }
            B->at.y -= 0.1;
#endif
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

#if 0
glBindTexture(GL_TEXTURE_2D, 0);

#if 1
glcolor(RED);
Begin(GL_LINES);
    Vertex2f(A->at.x, A->at.y);
    Vertex2f(A->at.x + normal_velocity_A.x*OBJ_MAX_RADIUS, A->at.y + normal_velocity_A.y*OBJ_MAX_RADIUS);
End();
#endif

glcolor(GREEN);
Begin(GL_LINES);
    Vertex2f(A->at.x, A->at.y);
    Vertex2f(A->at.x + tangent_velocity_A.x*OBJ_MAX_RADIUS, A->at.y + tangent_velocity_A.y*OBJ_MAX_RADIUS);
End();

#if 1
glcolor(WHITE);
Begin(GL_LINES);
    Vertex2f(A->at.x, A->at.y);
    Vertex2f(A->at.x + A->velocity.x*OBJ_MAX_RADIUS, A->at.y + A->velocity.y*OBJ_MAX_RADIUS);
End();
#endif
#endif
        A->velocity.x = normal_velocity_A.x + tangent_velocity_A.x;
        A->velocity.y = normal_velocity_A.y + tangent_velocity_A.y;

        B->velocity.x = normal_velocity_B.x + tangent_velocity_B.x;
        B->velocity.y = normal_velocity_B.y + tangent_velocity_B.y;

#if 0
        A->spin += spin;
        if (!B->is_stationary) {
            B->spin -= spin;
        }
#endif
        fpoint P = { intersect.x + tangent_unit.x,
                     intersect.y + tangent_unit.y };

        double a_mag = fpoint_project_onto_line(A->at, intersect, P);
        double b_mag = fpoint_project_onto_line(B->at, intersect, P);
        double rot = RAD_360 / 360.0;

        if (a_mag < 0) {
            A_rot = -rot;
        } else {
            A_rot = rot;
        }

        if (b_mag < 0) {
            B_rot = -rot;
        } else {
            B_rot = rot;
        }

//        A_rot /= 10.0;
//        B_rot /= 10.0;
//        A_rot *= flength(A->velocity);;
//        B_rot *= flength(B->velocity);;
//        A_rot += (a_mag / 1000.0);
//        B_rot += (b_mag / 1000.0);

        boolean A_moved;
        boolean B_moved;

        A_moved = false;
        B_moved = false;

        fpoint dV;

        dV.x = A->velocity.x;
        dV.y = A->velocity.y;
        if (collision_objects_try_to_move(A, dV)) {
            A_moved = true;
        }

        dV.x = B->velocity.x;
        dV.y = B->velocity.y;
        if (collision_objects_try_to_move(B, dV)) {
            B_moved = true;
        }

        if (!A_moved) {
            dV.x = tangent_velocity_A.x;
            dV.y = tangent_velocity_A.y;
            if (collision_objects_try_to_move(A, dV)) {
                A_moved = true;
            }
        }

        if (!B_moved) {
            dV.x = tangent_velocity_B.x;
            dV.y = tangent_velocity_B.y;
            if (collision_objects_try_to_move(B, dV)) {
                B_moved = true;
            }
        }

#if 0
        if (!A_moved) {
            dV.x = tangent_velocity_A.x;
            dV.y = 0;
            if (collision_objects_try_to_move(A, dV)) {
                A_moved = true;
            }
        }

        if (!B_moved) {
            dV.x = tangent_velocity_B.x;
            dV.y = 0;
            if (collision_objects_try_to_move(B, dV)) {
                B_moved = true;
            }
        }

        if (!A_moved) {
            dV.x = 0;
            dV.y = tangent_velocity_A.y;
            if (collision_objects_try_to_move(A, dV)) {
                A_moved = true;
            }
        }

        if (!B_moved) {
            dV.x = 0;
            dV.y = tangent_velocity_B.y;
            if (collision_objects_try_to_move(B, dV)) {
                B_moved = true;
            }
        }
#endif

#if 0
        if (!A_moved) {
            dV.x = normal_velocity_A.x;
            dV.y = normal_velocity_A.y;
            if (collision_objects_try_to_move(A, dV)) {
                A_moved = true;
            }
        }

        if (!B_moved) {
            dV.x = normal_velocity_B.x;
            dV.y = normal_velocity_B.y;
            if (collision_objects_try_to_move(B, dV)) {
                B_moved = true;
            }
        }
#endif
        while (fabs(A_rot) > 0.0001) {
            fpoint old_at = A->at;
            double old_rot = A->rot;

            rotate_about(A, intersect, A_rot);

            if (collision_check_single_object(A, true)) {
                A->at = old_at;
                A->rot = old_rot;
                A_rot /= 2.0;
            } else {
                A_moved = true;
                A->spin += A_rot;
                break;
            }
        }

        if (!B->is_stationary) {
            while (fabs(B_rot) > 0.0001) {
                fpoint old_at = B->at;
                double old_rot = B->rot;

                rotate_about(B, intersect, B_rot);

                if (collision_check_single_object(B, true)) {
                    B->at = old_at;
                    B->rot = old_rot;
                    B_rot /= 2.0;
                } else {
                    A_moved = true;
                    B->spin += B_rot;
                    break;
                }
            }
        }

        normal_velocity_A = fmul(COLLISION_ELASTICITY, normal_velocity_A);
        normal_velocity_B = fmul(COLLISION_ELASTICITY, normal_velocity_B);
        tangent_velocity_A = fmul(TANGENT_ELASTICITY, tangent_velocity_A);
        tangent_velocity_B = fmul(TANGENT_ELASTICITY, tangent_velocity_B);

        A->velocity.x = normal_velocity_A.x + tangent_velocity_A.x;
        A->velocity.y = normal_velocity_A.y + tangent_velocity_A.y;

        B->velocity.x = normal_velocity_B.x + tangent_velocity_B.x;
        B->velocity.y = normal_velocity_B.y + tangent_velocity_B.y;
    }

    A->old_at = A->at;
    A->old_rot = A->rot;
    A->old_velocity = A->velocity;

glPointSize(10.0);
int i;
glBindTexture(GL_TEXTURE_2D, 0);
glcolor(ORANGE);
Begin(GL_POINTS);
for (i=0;i<ARRAY_SIZE(dbg);i++) {
    Vertex2f(dbg[i].x,dbg[i].y);
}
End();

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

            A->old_at = A->at;
            A->old_rot = A->rot;
            A->old_velocity = A->velocity;

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
            dV = fdiv(MAX_VELOCITY, dV);

            /*
             * Save the old location.
             */
            A->old_at = A->at;
            A->old_rot = A->rot;

            /*
             * Creep forward.
             */
            A->at.x += dV.x;
            A->at.y += dV.y;

            if (fabs(A->spin) > 0) {
                A->rot += A->spin / MAX_VELOCITY;
                A->spin *= SPIN_LOSS;
            }

            A->collided = false;

            /*
             * If we impact at the new location, move back to where we were.
             */
            if (collision_check_single_object(A, false)) {
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
    }

    collision_all_check();
    collision_draw();
if (pause) {
    SDL_Delay(50);
}

    SDL_Delay(20);
}
