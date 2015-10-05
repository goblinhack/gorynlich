/*
 * Copyright (C) 2011-2015 goblinhack@gmail.com
 *
 * See the LICENSE file.
 */

#pragma once

#include "thing_template.h"

typedef struct {
    int8_t walls[MAP_WIDTH][MAP_HEIGHT];
} level_walls;

typedef struct level_map_tile_ {
    tpp tp;

    /*
     * Data associated with individual tiles.
     */
    thing_template_data data;
} level_map_tile;

typedef struct level_map_grid_ {
    level_map_tile tile[MAP_WIDTH][MAP_HEIGHT][MAP_DEPTH_MAX];
    uint8_t lit[MAP_WIDTH][MAP_HEIGHT];
} level_map_grid;

typedef struct level_trigger_ {
    const char *name;
    int activate_exists;
    int activated;
} level_trigger;

typedef struct level_t_ {
    level_pos_t level_pos;
    uint32_t seed;

    /*
     * Quick look op for initial things on the map and for finding what to 
     * spawn on a trigger for ex.
     */
    level_map_grid map_grid;

    /*
     * This is for monsters. One map where they try to go through doors and 
     * another where they give up. This way in a closed room with a door the
     * monster will wait by the door. If there is an open wall it will hit the
     * door and then decide to go for the opening.
     */
    level_walls map_player_target_treat_doors_as_passable;
    level_walls map_player_target_treat_doors_as_walls;

    /*
     * For ghosts who float over acid
     */
    level_walls map_player_target_treat_levitating_over_and_doors_as_passable;
    level_walls map_player_target_treat_levitating_over_and_doors_as_walls;

    level_walls map_treasure_target_treat_doors_as_passable;
    level_walls map_treasure_target_treat_doors_as_walls;

    level_walls walls;
    level_walls doors;

    /*
     * Zero memory from here on only for speed as the map_grid is large
     */
    uint8_t memzero_start;

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
     * Last time we fixed up the walls
     */
    int32_t timestamp_last_fixup;

#define MAX_TRIGGERS 256
    level_trigger trigger[MAX_TRIGGERS];

    /*
     * Things cannot move but can be animated.
     */
    uint8_t is_paused;

    /*
     * Level has a shop on it
     */
    uint8_t has_shop;

    /*
     * Exit has been reached.
     */
    uint8_t exit_has_been_reached:1;

    uint8_t game_over:1;

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
     * Walls or doors modified implies AI rethinking
     */
    uint8_t needs_updating:1;

    /*
     * Death...
     */
    uint8_t death_is_coming_soon:1;
    uint8_t death_is_coming:1;

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
    uint8_t zzz16:1;
    uint8_t is_death_summoned:1;
    uint8_t is_jesus_summoned:1;
    uint8_t is_blit_y_offset:1;
    uint8_t on_server:1;
    uint8_t exit_reached_when_open:1;
    uint8_t warned_exit_not_open:1;
    uint8_t is_exit_open:1;

    /*
     * Some walls have changed. Need to redo AI algorithims.
     */
    uint8_t needs_fixup:1;

    /*
     * Someone is still using this level.
     */
    uint8_t locked:1;

    /*
     * Ask the AI thread to finish.
     */
    uint8_t exit_request:1;

    /*
     * Zero memory from here on only for speed as the map_grid is large
     */
    uint8_t memzero_end;

} level_t;
