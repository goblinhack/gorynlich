/*
 * Copyright (C) 2011 Neil McGill
 *
 * See the README file.
 */

typedef struct thing_ {
    tree_key_int tree;

    /*
     * For map linkeage.
     */
    thingp mnext;
    thingp mprev;

    /*
     * Pointer to common settings for this thing.
     */
    thing_templatep thing_template;

    /*
     * Carried items.
     */
    tree_root *carried_items;

    /*
     * If an item, who is carrying me?
     */
    thingp item_owner;

    /*
     * Name for debugging.
     */
    char *logname;

    /*
     * Widget for displaying thing.
     */
    widp wid;

    /*
     * Player current level or level of monster.
     */
    uint32_t level_no;

    /*
     * What level is the thing on?
     */
    levelp level;

    /*
     * How many to go till the end?!
     */
    uint32_t lives;

    /*
     * Last death reason.
     */
    char *dead_reason;

    /*
     * Scoring
     */
    uint32_t score;
    uint32_t score_pump;

    /*
     * Periodic spam message.
     */
    timerp timer_spam;

    /*
     * Powerups
     */
    uint8_t powerup_spam_count;
    uint8_t powerup2_count;
    uint8_t powerup3_count;
    uint8_t powerup4_count;
    uint8_t powerup5_count;
    uint8_t powerup6_count;
    uint8_t powerup7_count;
    uint8_t powerup8_count;
    uint8_t powerup_rocket_count;

    /*
     * Array of which tiles we've been to.
     */
    int32_t visited[TILES_MAP_EDITABLE_WIDTH][TILES_MAP_EDITABLE_HEIGHT];

    /*
     * Grid coordinates.
     */
    int32_t x;
    int32_t y;
    int32_t tl_x;
    int32_t tl_y;
    int32_t br_x;
    int32_t br_y;
    int32_t grid_x;
    int32_t grid_y;
    int32_t last_x;
    int32_t last_y;
    int32_t target_x;
    int32_t target_y;

    /*
     * For animation.
     */
    thing_tilep current_tile;

    /*
     * When to change frame.
     */
    uint32_t timestamp_change_to_next_frame;
    uint32_t timestamp_teleport;;

    /*
     * When do I die? 8(
     */
    uint32_t timestamp_lifestamp;

    uint32_t is_qqq1:1;
    uint32_t is_qqq2:1;
    uint32_t is_qqq3:1;
    uint32_t is_qqq4:1;
    uint32_t is_qqq5:1;
    uint32_t is_qqq6:1;
    uint32_t is_qqq7:1;
    uint32_t got_to_exit_first:1;
    uint32_t redo_maze_search:1;
    uint32_t is_follows_owner:1;
    uint32_t is_dir_down:1;
    uint32_t is_dir_up:1;
    uint32_t is_dir_left:1;
    uint32_t is_dir_right:1;
    uint32_t opened_exit:1;
    uint32_t is_open:1;
    uint32_t is_buried:1;
    uint32_t is_left_as_corpse_on_death:1;
    uint32_t is_dead:1;
} thing;
