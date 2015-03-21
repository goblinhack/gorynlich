/*
 * Copyright (C) 2011 Neil McGill
 *
 * See the LICENSE file.
 */

#pragma once

typedef struct {
    int8_t walls[MAP_WIDTH][MAP_HEIGHT];
} level_walls;

typedef struct level_t_ {
    level_pos_t level_pos;

    /*
     * This is for monsters. One map where they try to go through doors and 
     * another where they give up. This way in a closed room with a door the
     * monster will wait by the door. If there is an open wall it will hit the
     * door and then decide to go for the opening.
     */
    level_walls monst_map_treat_doors_as_passable;
    level_walls monst_map_treat_doors_as_walls;

    /*
     * And this is for players to define the range of a flood fill explosion.
     */
    level_walls player_map_treat_doors_as_walls;

    level_walls walls;
    level_walls roads;
    level_walls pipes;
    level_walls end_pipe;

    /*
     * Possible places for players to start.
     */
    fpoint player_start_position[MAX_PLAYERS];
    uint8_t player_start_max;
    uint8_t player_start_at;

    /*
     * On screen name for level.
     */
    char *title;

    /*
     * For debugs.
     */
    char *logname;

    /*
     * Why the level is being destroyed.
     */
    char *destroy_reason;

    /*
     * Grid widget
     */
    widp map;

    /*
     * Timers
     */
    timerp end_level_first_phase_fade_out_timer;
    timerp end_level_second_phase_destroy_timer;
    timerp pause_timer;

    /*
     * When the level began being played.
     */
    int32_t timestamp_started;

    /*
     * Things cannot move but can be animated.
     */
    uint8_t is_paused:1;

    /*
     * Exit has been reached.
     */
    uint8_t exit_has_been_reached:1;

    /*
     * Exit has been reached and a long delay has passed. The level will be 
     * destoyed now.
     */
    uint8_t is_ready_to_be_destroyed:1;

    /*
     * Exit has been reached and a small delay has passed. Hide the level from 
     * the client.
     */
    uint8_t is_ready_to_fade_out:1;

    /*
     * Being used in level editor.
     */
    uint8_t is_editor:1;
    uint8_t is_map_editor:1;

    /*
     * Make things go faster.
     */
    uint8_t is_hurryup:1;

    /*
     * Usually level 66.66
     */
    uint8_t is_test_level:1;

    uint8_t is_zzz1:1;
    uint8_t is_zzz2:1;
    uint8_t is_zzz3:1;
    uint8_t is_zzz4:1;
    uint8_t is_zzz5:1;
    uint8_t is_zzz6:1;
    uint8_t is_zzz7:1;
    uint8_t is_zzz8:1;
    uint8_t is_zzz9:1;
    uint8_t is_zzz10:1;
    uint8_t is_zzz11:1;
    uint8_t is_zzz12:1;
    uint8_t is_zzz13:1;
    uint8_t is_zzz14:1;
    uint8_t is_zzz15:1;
    uint8_t is_zzz16:1;
    uint8_t is_zzz17:1;
    uint8_t on_server:1;
    uint8_t exit_reached_when_open:1;
    uint8_t warned_exit_not_open:1;
    uint8_t is_exit_open:1;

    /*
     * Someone is still using this level.
     */
    uint8_t locked:1;

    /*
     * Ask the AI thread to finish.
     */
    uint8_t exit_request:1;

} level_t;

extern level_walls dmap_player_map_treat_doors_as_walls;
