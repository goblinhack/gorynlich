/*
 * Copyright (C) 2011 Neil McGill
 *
 * See the LICENSE file for license.
 */

uint8_t wid_thing_stats_init(void);
void wid_player_stats_fini(void);
void wid_player_stats_hide(int fast);
void wid_player_stats_visible(thing_statsp, int fast);
void wid_player_stats_redraw(int fast);
extern widp wid_player_stats;

typedef struct player_stats_row_ {
    const char *col1;
    const char *col2;
    uint32_t increment;
    const char *tooltip;
} player_stats_row;

enum {
    STAT_SPENDING_POINTS,
    STAT_EXPERIENCE,
    STAT_LEVEL,
    STAT_MAX_HP,
    STAT_MAX_MAGIC,
    STAT_ATTACK_MELEE,
    STAT_ATTACK_RANGED,
    STAT_ATTACK_MAGICAL,
    STAT_DEFENSE,
    STAT_SPEED,
    STAT_VISION,
    STAT_HEALING,
};

#define PLAYER_STATS_MAX 12 

extern player_stats_row player_stats_arr[PLAYER_STATS_MAX];
