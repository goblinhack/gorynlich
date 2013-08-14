/*
 * Copyright (C) 2011 Neil McGill
 *
 * See the README file.
 */

typedef struct level_t_ {
    uint32_t level_no;

    /*
     * A quick cache of where the walls are.
     */
    char walls[TILES_MAP_EDITABLE_WIDTH][TILES_MAP_EDITABLE_HEIGHT];
    char monst_walls[TILES_MAP_EDITABLE_WIDTH][TILES_MAP_EDITABLE_HEIGHT];
    char roads[TILES_MAP_EDITABLE_WIDTH][TILES_MAP_EDITABLE_HEIGHT];
    char pipes[TILES_MAP_EDITABLE_WIDTH][TILES_MAP_EDITABLE_HEIGHT];
    char end_pipe[TILES_MAP_EDITABLE_WIDTH][TILES_MAP_EDITABLE_HEIGHT];

    /*
     * Is the bonus timer running?
     */
    timerp bonus_timer;

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
    mapp map;

    /*
     * When the level began being played.
     */
    int32_t timestamp_started;

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

