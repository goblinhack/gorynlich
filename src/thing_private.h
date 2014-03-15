/*
 * Copyright (C) 2011 Neil McGill
 *
 * See the LICENSE file.
 */

typedef struct thing_ {
    tree_key_int tree;

    /*
     * Pointer to common settings for this thing.
     */
    thing_templatep thing_template;

    /*
     * Name for debugging.
     */
    char *logname;

    /*
     * Which tree are we on, server active? client boring? etc...
     */
    tree_rootp client_or_server_tree;

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
     * Last death reason.
     */
    char *dead_reason;

    /*
     * Scoring
     */
    uint32_t score;
    uint32_t score_pump;

    /*
     * Thing health.
     */
    uint32_t health;

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
     * How many and of what we are carrying.
     */
    uint16_t carrying[THING_MAX];

    /*
     * Grid coordinates.
     */
    double x;
    double y;
    double last_x;
    double last_y;
    double target_x;
    double target_y;

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

    /*
     * A counter to indicate the number of times we want to update the client 
     * with the state of this thing. To compensate for udp drops we might want 
     * to send a couple of updates for important events like death.
     */
    uint8_t updated;

    /*
     * To keep a thing around for a while so we can clean it up once the 
     * client has been notified of the server death.
     */
    uint8_t destroy_delay;

    uint8_t dir:4;

    uint32_t is_qqq1:1;
    uint32_t is_qqq2:1;
    uint32_t is_qqq3:1;
    uint32_t is_qqq4:1;
    uint32_t is_qqq5:1;
    uint32_t is_qqq6:1;
    uint32_t is_qqq7:1;
    uint32_t got_to_exit_first:1;
    uint32_t redo_maze_search:1;
    uint32_t opened_exit:1;
    uint32_t is_open:1;
    uint32_t is_buried:1;
    uint32_t is_dead:1;
    uint32_t on_active_list:1;

    /*
     * Force client to server postion.
     */
    uint32_t resync:1;
} thing;

