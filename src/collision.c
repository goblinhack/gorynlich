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
static double GRAVITY                   = 0.03;
static double COLLISION_ELASTICITY      = 0.7;
static double FRICTION                  = 0.995;
static double TANGENT_ELASTICITY        = 1.0;
#else
static double GRAVITY                   = 0.00;
static double COLLISION_ELASTICITY      = 1.0;
static double FRICTION                  = 1.0;
static double TANGENT_ELASTICITY        = 1.0;
#endif
static double MAX_VELOCITY              = 5.0;
static double MAX_TIMESTEP              = 5.0;
static const uint32_t OBJ_MIN_RADIUS    = 10;
static const uint32_t OBJ_MAX_RADIUS    = 80;
static const uint32_t OBJ_MAX           = 42;

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

            if ((rand() % 100) < 70) {
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
        if ((rand() % 100) < 30) {
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
        char tmp[10];
        sprintf(tmp, "%d", o);

        ttf_puts(small_font, tmp, obj->at.x, obj->at.y, 1.0, 1.0, true);
        glBindTexture(GL_TEXTURE_2D, 0);
#endif
    }

    glPopMatrix();
}

static boolean box_box_collision (object *A, object *B,
                                  fpoint *normal,
                                  boolean check_only)
{
    fpoint A_norm[4];
    fpoint B_norm[4];
    fpoint A_vec[4];
    fpoint B_vec[4];
    fpoint A_mid[4];
    fpoint B_mid[4];

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
    double mag_overlap;
    double mag_overlap_best = 0;
    int mag_overlap_best_index = -1;

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

            if (max_mag > 1) {
                max_mag = 1;
            }

            if (min_mag < 0) {
                min_mag = 0;
            }

            mag_overlap = max_mag - min_mag;

            if (mag_overlap > mag_overlap_best) {
                mag_overlap_best = mag_overlap;
                mag_overlap_best_index = i;
            }
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

#if 0

    glcolor(WHITE);
    for (j = 0; j < 4; j++) {
if (j == 3) {
    glcolor(RED);
} else {
    glcolor(WHITE);
}
    Begin(GL_LINES);
        Vertex2f(AP[j].x+200,AP[j].y);
        Vertex2f(AP[(j+1)%4].x+200,AP[(j+1)%4].y);
    End();
    }

    glcolor(CYAN);
    for (j = 0; j < 4; j++) {
if (j == 3) {
    glcolor(RED);
} else {
    glcolor(WHITE);
}
    Begin(GL_LINES);
        Vertex2f(BP[j].x+200,BP[j].y);
        Vertex2f(BP[(j+1)%4].x+200,BP[(j+1)%4].y);
    End();
    }
#endif

    if (hit <= 3) {
        return (false);
    }

    if (check_only) {
        return (true);
    }

    if (mag_overlap_best_index == -1) {
        DIE("overlap bug");
    }

#if 0
    double angle_1 = fpoint_angle(A->velocity,
                                  B_norm[mag_overlap_best_index]);
    double angle_2 = fpoint_angle(A->velocity,
                                  B_norm[mag_overlap_best_index+2]);
    if (angle_2 > angle_1) {
        mag_overlap_best_index = mag_overlap_best_index + 2;
    }
if (B-objects == 1) {
LOG("%f %f",angle_1,angle_2);
}
#endif

    double dist_1;
    double dist_2;

    fpoint_dist_line(A->at, BP[mag_overlap_best_index],
                            BP[mag_overlap_best_index+1], &dist_1);
    fpoint_dist_line(A->at, BP[mag_overlap_best_index+2],
                            BP[(mag_overlap_best_index+3) % 4], &dist_2);
    if (dist_2 < dist_1) {
        mag_overlap_best_index = mag_overlap_best_index + 2;
    }

    normal->x = B_norm[mag_overlap_best_index].x;
    normal->y = B_norm[mag_overlap_best_index].y;

#if 0
    glPushMatrix();
//    glTranslatef((float) -x*scale, (float) -y*scale, 0);
    glScalef(scale, scale, 0);
        glBindTexture(GL_TEXTURE_2D, 0);

if (A-objects == 1) {
//LOG("mag_overlap_best_index %d norm y %f",mag_overlap_best_index,normal->y);
}
    for (j = 0; j < 4; j++) {
        if (j == mag_overlap_best_index) {
    glcolor(GREEN);
        Begin(GL_LINES);
            Vertex2f(B_mid[j].x,B_mid[j].y);
            Vertex2f(B_mid[j].x+B_norm[j].x,B_mid[j].y+B_norm[j].y);
        End();
        }
    }
#endif

    glPopMatrix();

    return (true);
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

static boolean collision_objects_try_to_move (object *A, fpoint dVa,
                                              object *B, fpoint dVb)
{
    if (!A->is_stationary) {
        dVa = fdiv(MAX_TIMESTEP, dVa);

        A->at.x += dVa.x;
        A->at.y += dVa.y;
    }

    if (!B->is_stationary) {
        /*
         * Try B out in the new position.
         */
        dVb = fdiv(MAX_TIMESTEP, dVb);

        B->at.x += dVb.x;
        B->at.y += dVb.y;
    }

    if (!A->is_stationary) {
        if (collision_check_single_object(A, B, true)) {
            /*
             * Hit another object. Move back.
             */
            return (false);
        }
    }

    if (!B->is_stationary) {
        if (collision_check_single_object(B, A, true)) {
            /*
             * Hit another object. Move back.
             */
            return (false);
        }
    }

    return (true);
}

/*
 * See if this object collides with anything.
 */
static boolean collision_check_single_object (object *A,
                                              object *ignore,
                                              boolean check_only)
{
    uint32_t hit;
    uint32_t b;

    if (A->is_stationary) {
        return (false);
    }

    hit = 0;

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
            if (box_box_collision(A, B, &normal, check_only)) {
                hit++;
                collided = true;
            }
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

#if 1
        B->at.y += 0.1;
        if (collision_check_single_object(B, 0, true)) {
            mB = mA;
            vB.y = max(vA.y, vB.y);
            vB.y = -vB.y;

        }
        B->at.y -= 0.1;
#endif

        /*
         * Project the velocity onto the normal vectors.
         */
        double normal_A_len = fdot(normal_unit, vA);
if (A-objects == 1) {
//LOG("norm unit %f %f vA %f %f dot %f normal_A_len %f", vA.x,vA.y, normal_unit.x,normal_unit.y, fdot(normal_unit, vA), normal_A_len);
}
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

glcolor(RED);
Begin(GL_LINES);
    Vertex2f(A->at.x, A->at.y);
    Vertex2f(A->at.x + normal_velocity_A.x*OBJ_MAX_RADIUS*100, A->at.y + normal_velocity_A.y*OBJ_MAX_RADIUS*100);
End();

glcolor(CYAN);
Begin(GL_LINES);
    Vertex2f(A->at.x, A->at.y);
    Vertex2f(A->at.x + tangent_velocity_A.x*OBJ_MAX_RADIUS*100, A->at.y + tangent_velocity_A.y*OBJ_MAX_RADIUS*100);
End();

#if 0
glcolor(WHITE);
Begin(GL_LINES);
    Vertex2f(A->at.x, A->at.y);
    Vertex2f(A->at.x + A->velocity.x*OBJ_MAX_RADIUS*10, A->at.y + A->velocity.y*OBJ_MAX_RADIUS*10);
End();
#endif
#endif
        normal_velocity_A = fmul(COLLISION_ELASTICITY, normal_velocity_A);
        normal_velocity_B = fmul(COLLISION_ELASTICITY, normal_velocity_B);
        tangent_velocity_A = fmul(TANGENT_ELASTICITY, tangent_velocity_A);
        tangent_velocity_B = fmul(TANGENT_ELASTICITY, tangent_velocity_B);

        A->old_at = A->at;
        B->old_at = B->at;

        for (;;) {
            A->velocity.x = normal_velocity_A.x + tangent_velocity_A.x;
            A->velocity.y = normal_velocity_A.y + tangent_velocity_A.y;
            if (!B->is_stationary) {
                B->velocity.x = normal_velocity_B.x + tangent_velocity_B.x;
                B->velocity.y = normal_velocity_B.y + tangent_velocity_B.y;
            }

            if (collision_objects_try_to_move(A, A->velocity, B, B->velocity)) {
                break;
            }

            A->at = A->old_at;
            B->at = B->old_at;
            A->velocity.x = -(normal_velocity_A.x + tangent_velocity_A.x);
            A->velocity.y = -(normal_velocity_A.y + tangent_velocity_A.y);
            if (!B->is_stationary) {
                B->velocity.x = normal_velocity_B.x + tangent_velocity_B.x;
                B->velocity.y = normal_velocity_B.y + tangent_velocity_B.y;
            }

            if (collision_objects_try_to_move(A, A->velocity, B, B->velocity)) {
                break;
            }

            A->at = A->old_at;
            B->at = B->old_at;
            A->velocity.x = (normal_velocity_A.x + tangent_velocity_A.x);
            A->velocity.y = (normal_velocity_A.y + tangent_velocity_A.y);
            if (!B->is_stationary) {
                B->velocity.x = -(normal_velocity_B.x + tangent_velocity_B.x);
                B->velocity.y = -(normal_velocity_B.y + tangent_velocity_B.y);
            }

            if (collision_objects_try_to_move(A, A->velocity, B, B->velocity)) {
                break;
            }

            A->at = A->old_at;
            B->at = B->old_at;
            A->velocity.x = -(normal_velocity_A.x + tangent_velocity_A.x);
            A->velocity.y = -(normal_velocity_A.y + tangent_velocity_A.y);
            if (!B->is_stationary) {
                B->velocity.x = -(normal_velocity_B.x + tangent_velocity_B.x);
                B->velocity.y = -(normal_velocity_B.y + tangent_velocity_B.y);
            }

            if (collision_objects_try_to_move(A, A->velocity, B, B->velocity)) {
                break;
            }

            A->at = A->old_at;
            B->at = B->old_at;
            break;
        }

        if ((A->at.x != A->old_at.x) || (A->at.y != A->old_at.y)) {
            A->velocity.x *= FRICTION;
            A->velocity.y *= FRICTION;
        }

        if ((B->at.x != B->old_at.x) || (B->at.y != B->old_at.y)) {
            B->velocity.x *= FRICTION;
            B->velocity.y *= FRICTION;
        }

if (A-objects == 4) {
//pause = 1;
}
    }

    glPushMatrix();
//    glTranslatef((float) -x*scale, (float) -y*scale, 0);
    glScalef(scale, scale, 0);
#if 0
glBindTexture(GL_TEXTURE_2D, 0);
glcolor(ORANGE);
Begin(GL_LINES);
    Vertex2f(A->at.x, A->at.y);
    Vertex2f(A->at.x + A->velocity.x*OBJ_MAX_RADIUS, A->at.y + A->velocity.y*OBJ_MAX_RADIUS);
End();
#endif
    glPopMatrix();

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
if (A-objects != 2)  {
//continue;
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
if (A-objects == 1) {
//LOG("at.y %f vel %f dV.y %f",A->at.y,A->velocity.y, dV.y);
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
    SDL_Delay(500);
}

    SDL_Delay(5);
}
