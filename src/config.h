/*
 * Copyright (C) 2011 Neil McGill
 *
 * See the README file.
 */

#undef  ENABLE_LOGFILE            // Log stdout and err to a file
#define ENABLE_LEAKCHECK           // Memory leak check
#define ENABLE_PTRCHECK            // Check validity of pointers too
#define ENABLE_CONSOLE              // Enable terminal console all the time
#undef ENABLE_ASSERT              // DIE on errors
#undef ENABLE_TREE_SANITY         // Slow tree sanity
#undef ENABLE_DEBUG               // More debugs
#define ENABLE_VIDEO_SYNC          // Vertical sync
#undef ENABLE_GENERATE_TTF        // Edit RUNME to add -lSDL_ttf
#undef ENABLE_DEMARSHAL_DEBUG     // Demarshalling, prints structures
#define ENABLE_MAP_DEBUG           // Debug AI by printing the map
#undef ENABLE_WID_DEBUG           // Widget debug
#undef ENABLE_GEN_LIGHT_MAP_FILE  // Recreate the raytrace light map file
#undef ENABLE_GENERATE_TTF        // Remake TTF TGA files

/*
 * Tries to batch calls to draw arrays for more speed.
 */
#define ENABLE_GL_BULK_DRAW_ARRAYS 

/*
 * Frames per sec counter.
 */
#define ENABLE_FPS_COUNTER          1

/*
 * Screen sizes.
 */
#define DEFAULT_VIDEO_WIDTH         1080 // See gl.h for more
#define DEFAULT_VIDEO_HEIGHT        720

/*
 * This is multiplied by jigsaw piece size.
 */
#define MAP_JIGSAW_PIECE_WIDTH      5
#define MAP_JIGSAW_PIECE_HEIGHT     3

#define JIGPIECE_WIDTH              13
#define JIGPIECE_HEIGHT             13

#define TILE_WIDTH                  32
#define TILE_HEIGHT                 32

/*
 * Tiles
 */
#define TILES_MAP_WIDTH                 32
#define TILES_MAP_HEIGHT                32

/*
 * The number of tiles per screen.
 */
#define TILES_SCREEN_WIDTH              21
#define TILES_SCREEN_HEIGHT             14

#define TILES_SERVER_SCALE              4
#define TILES_CLIENT_SCALE              1

/*
 * Console
 */
#define CONSOLE_CURSOR_COLOR        GREEN
#define CONSOLE_CURSOR_OTHER_COLOR  DARKGREEN
#define CONSOLE_TEXT_COLOR          GREEN
#define CONSOLE_HEIGHT              100

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
#define SMALL_STRING_LEN_MAX        30
#define PLAYER_MSG_MAX              100

/*
 * Connections.
 */
#define DEFAULT_PORT                10000
#define SERVER_DEFAULT_HOST         "localhost"
#define SERVER_DEFAULT_PORT         DEFAULT_PORT
#define MAX_PACKET_SIZE             4096

/*
 * Frequency of client and server pings
 */
#define PING_DELAY_TENTHS           5

/*
 * How many pings to consider for quality
 */
#define SOCKET_PING_SEQ_NO_RANGE    20

/* 
 * 90 percent of ping fails means down
 */
#define SOCKET_PING_FAIL_THRESHOLD  10

/*
 * Maximum number of thing types. Dicates message sizes etc...
 */
#define THING_MAX 128

/*
 * When converting from thing co-ords where 1.0 is one grid, we must convert
 * to int when sending over the network. This is the granularity.
 */
#define THING_COORD_SCALE           100.0

/*
 * How much we allow the player to move per step. Fraction of one grid.
 */
#define THING_PLAYER_COORD_MOVE     0.10
#define THING_MONST_COORD_MOVE      1.00

/*
 * How many tiles difference between the client and server do we allow.
 * This allows local moves to occur without the lag of waiting for the
 * server. Set this too high and the client can push through walls. Too
 * low and it will be jerky, jumping back when we have high latency.
 */
#define THING_MAX_SERVER_DISCREPANCY 2.0
#define THING_PLAYER_POLL_SPEED      1 // hundredths
