/*
 * Copyright (C) 2011 Neil McGill
 *
 * See the LICENSE file.
 */

#pragma once

typedef struct {
    int8_t walls[TILES_MAP_WIDTH][TILES_MAP_HEIGHT];
} level_walls;

typedef struct level_t_ {
    uint32_t level_no;

    /*
     * Someone is still using this level.
     */
    uint8_t locked;

    /*
     * This is for monsters. One map where they try to go through doors and 
     * another where they give up. This way in a closed room with a door the
     * monster will wait by the door. If there is an open wall it will hit the
     * door and then decide to go for the opening.
     */
    level_walls monst_map_treat_doors_as_passable;
    level_walls monst_map_treat_doors_as_walls;

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
     * When the level began being played.
     */
    int32_t timestamp_started;

    /*
     * Send an update to the client, the level map has changed.
     */
    uint8_t need_map_update:1;

    /*
     * Things cannot move but can be animated.
     */
    uint8_t is_paused:1;

    /*
     * Things cannot move at all.
     */
    uint8_t is_frozen:1;

    /*
     * Being used in level editor.
     */
    uint8_t is_editor:1;

    /*
     * Make things go faster.
     */
    uint8_t is_hurryup:1;
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
    uint8_t exit_reached_when_open:1;
    uint8_t warned_exit_not_open:1;
    uint8_t is_exit_open:1;

} level_t;

