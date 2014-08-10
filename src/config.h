/*
 * Copyright (C) 2011 Neil McGill
 *
 * See the README file.
 */

#pragma once

/*
 * We don't always need this. Like on a laptop.
 */
#undef ENABLE_VIDEO_SYNC

/*
 * More speed, less fancy stuff
 */
#undef MORE_SPEED_LESS_VISUALS

#undef ENABLE_CONSOLE
#define ENABLE_LOGFILE             // Log stdout and err to a file
#define ENABLE_LEAKCHECK           // Memory leak check
#undef ENABLE_PTRCHECK            // Check validity of pointers too
#undef ENABLE_MAP_SANITY          // Extra map checks
#undef ENABLE_ASSERT              // DIE on errors
#undef ENABLE_TREE_SANITY         // Slow tree sanity
#undef ENABLE_DEBUG               // More debugs
#undef ENABLE_GENERATE_TTF        // Edit RUNME to add -lSDL_ttf
#undef ENABLE_DEMARSHAL_DEBUG     // Demarshalling, prints structures
#undef ENABLE_MAP_DEBUG           // Debug AI by printing the map
#undef ENABLE_WID_DEBUG           // Widget debug
#undef ENABLE_GENERATE_TTF        // Remake TTF TGA files


/*
 * Other windows are darker when popups appear
 */
#undef ENABLE_FADE_OUT_OTHER_WIDGETS_WHEN_FOCUS_LOCKED

/*
 * Retro text
 */
#define ENABLE_BIG_RETRO_TEXT_OUTLINE

/*
 * Tooltip appears above mouse. I prefer it to zoom into the top
 */
#undef ENABLE_TOOLTIP_OVER_MOUSE

/*
 * Tries to batch calls to draw arrays for more speed.
 */
#define ENABLE_GL_BULK_DRAW_ARRAYS 

/*
 * So we don't hog all the CPU
 */
#define MAIN_LOOP_DELAY            2

/*
 * Frames per sec counter.
 */
#define ENABLE_FPS_COUNTER          1

/*
 * Screen sizes.
 */
#define DEFAULT_VIDEO_WIDTH         1008 // See gl.h for more
#define DEFAULT_VIDEO_HEIGHT        672

/*
 * This is multiplied by jigsaw piece size.
 */
#define MAP_JIGSAW_PIECE_WIDTH      4
#define MAP_JIGSAW_PIECE_HEIGHT     4

#define JIGPIECE_WIDTH              8
#define JIGPIECE_HEIGHT             8

/*
 * Map
 */
#define MAP_WIDTH                   64
#define MAP_HEIGHT                  64

#define MAP_DEPTH_EDITOR            0
#define MAP_DEPTH_FLOOR             1
#define MAP_DEPTH_WALL              2
#define MAP_DEPTH_MONST             3
#define MAP_DEPTH_PLAYER            4
#define MAP_DEPTH_EXPLOSION         4
#define MAP_DEPTH_EXPLOSION_TOP     5
#define MAP_DEPTH                   6
#define MAP_BORDER                  3

#define MAP_THINGS_PER_CELL         32

/*
 * Tile pixel width.
 */
#define TILE_WIDTH                  64
#define TILE_HEIGHT                 64

/*
 * The width of wall blocks and floor tiles not counting shadow pixels
 */
#define TILE_PIX_WIDTH              48
#define TILE_PIX_HEIGHT             48

/*
 * The width including shadow pixels.
 */
#define TILE_PIX_WITH_SHADOW_WIDTH  TILE_WIDTH
#define TILE_PIX_WITH_SHADOW_HEIGHT TILE_HEIGHT

/*
 * The number of tiles per screen.
 */
#define TILES_SCREEN_WIDTH          21
#define TILES_SCREEN_HEIGHT         14

/*
 * Console
 */
#define CONSOLE_CURSOR_COLOR        GREEN
#define CONSOLE_CURSOR_OTHER_COLOR  DARKGREEN
#define CONSOLE_TEXT_COLOR          GREEN
#define CONSOLE_HEIGHT              100

/*
 * Chat window
 */
#define CHAT_CURSOR_COLOR           PURPLE
#define CHAT_CURSOR_OTHER_COLOR     PURPLE
#define CHAT_TEXT_COLOR             PLUM
#define CHAT_HEIGHT                 20

/*
 * Fonts
 */
#define VSMALL_FONT                 "data/ttf/emulogic.ttf"
#define SMALL_FONT                  "data/ttf/emulogic.ttf"
#define MED_FONT                    "data/ttf/emulogic.ttf"
#define LARGE_FONT                  "data/ttf/emulogic.ttf"
#define VLARGE_FONT                 "data/ttf/emulogic.ttf"

/*
 * Messages.
 */
#define MAX_PLAYERS                 4
#define MAX_HISCORES                10
#define SMALL_STRING_LEN_MAX        30
#define PLAYER_MSG_MAX              100

/*
 * Connections.
 */
#define DEFAULT_PORT                10000
#define SERVER_DEFAULT_HOST         "localhost"
#define SERVER_DEFAULT_PORT         DEFAULT_PORT
#define MAX_PACKET_SIZE             4000

/*
 * How many pings to consider for quality
 */
#define SOCKET_PING_SEQ_NO_RANGE    20

/* 
 * 90 percent of ping fails means down
 */
#define SOCKET_PING_FAIL_THRESHOLD  10

/*
 * Maximum number of thing types. This cannot be exceeded without changing
 * the message format in tx updates.
 */
#define THING_MAX                   128

/*
 * Thumb in air. Needs to be big enough so wrap arounds and id reuse is less 
 * common. Max is 64K for tx map sending from server to client.
 */
#define THING_ID_MAX                (64*1024)

/*
 * Local client things like explosions we use a separate space beyond the 
 * above.
 */
#define THING_CLIENT_ID_MIN         (THING_ID_MAX + 1)
#define THING_CLIENT_ID_MAX         (THING_ID_MAX * 2)

/*
 * When converting from thing co-ords where 1.0 is one grid, we must convert
 * to int when sending over the network. This is the granularity.
 */
#define THING_COORD_SCALE           100.0

/*
 * Map on screen percent width.
 */
#define MAP_WINDOW_WIDTH             0.67

/*
 * How many tiles difference between the client and server do we allow.
 * This allows local moves to occur without the lag of waiting for the
 * server. Set this too high and the client can push through walls. Too
 * low and it will be jerky, jumping back when we have high latency.
 */
#define THING_MAX_SERVER_DISCREPANCY 3.0

/*
 * Timers
 */
#define DELAY_TENTHS_THING_AI                   2
#define DELAY_TENTHS_THING_COLLISION_TEST       1
#define DELAY_TENTHS_PING                       5
#define DELAY_TENTHS_SOCKET_QUALITY_CHECK       10
#define DELAY_HUNDREDTHS_PLAYER_TX_POSITION     5
#define DELAY_THOUSANDTHS_TX_MAP_UPDATE_FAST    50
#define DELAY_THOUSANDTHS_TX_MAP_UPDATE_SLOW    200
#define DELAY_THOUSANDTHS_PLAYER_POLL           15

#define DELAY_LEVEL_END_HIDE                    (ONESEC * 1)
#define DELAY_LEVEL_END_DESTROY                 (ONESEC * 4)
