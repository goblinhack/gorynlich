/*
 * Copyright (C) 2011 Neil McGill
 *
 * See the README file.
 */

#undef  ENABLE_LOGFILE            // Log stdout and err to a file
#define ENABLE_LEAKCHECK          // Memory leak check
#define ENABLE_PTRCHECK            // Check validity of pointers too
#undef ENABLE_ASSERT              // DIE on errors
#undef ENABLE_TREE_SANITY         // Slow tree sanity
#undef ENABLE_DEBUG               // More debugs
#undef ENABLE_VIDEO_SYNC          // Vertical sync
#undef ENABLE_INVERTED_DISPLAY    // For running on a Mac with inverted gfx
#undef ENABLE_GENERATE_TTF        // Edit RUNME to add -lSDL_ttf
#undef ENABLE_DEMARSHAL_DEBUG     // Demarshalling, prints structures
#undef ENABLE_MAP_DEBUG           // Debug AI by printing the map
#undef ENABLE_WID_DEBUG           // Widget debug
#undef ENABLE_GEN_LIGHT_MAP_FILE  // Recreate the raytrace light map file
#undef ENABLE_GENERATE_TTF        // Remake TTF TGA files

/*
 * Screen sizes.
 */
#define DEFAULT_VIDEO_WIDTH         1080 // See gl.h for more
#define DEFAULT_VIDEO_HEIGHT        720

/*
 * We zoom the tiles in, by default 8x16 so they look more blocky and retro.
 */
#define DEFAULT_TILE_ZOOM           3

/*
 * How far we can cast light
 */
#define DEFAULT_LIGHT_RAY_LENGTH    35

/*
 * How much solid rock light can permeat through. Too high and you can see
 * traps. Too low and it looks lousy.
 */
#define DEFAULT_LIGHT_PENETRATE     3

/*
 * Map size is 2 ^ MAP_WIDTH_LOG. So MAP_WIDTH_LOG 8 is 256, 10 is 1024
 */
#define MAP_WIDTH_LOG               10
#define MAP_WIDTH                   (1 << MAP_WIDTH_LOG)
#define MAP_HEIGHT                  MAP_WIDTH

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
#define TILES_MAP_WIDTH                 64
#define TILES_MAP_HEIGHT                64

#define TILES_MAP_EDITABLE_WIDTH        64
#define TILES_MAP_EDITABLE_HEIGHT       64

/*
 * The number of tiles per screen.
 */
#define TILES_SCREEN_WIDTH              21
#define TILES_SCREEN_HEIGHT             14

/*
 * Console
 */
#define CONSOLE_CURSOR_COLOR        ORANGE
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
#define SMALL_STRING_LEN_MAX         20
#define PLAYER_MSG_MAX              100

/*
 * Connections.
 */
#define DEFAULT_PORT                10000

#define SERVER_DEFAULT_HOST         "localhost"
#define SERVER_DEFAULT_PORT         DEFAULT_PORT

#define MAX_PACKET_SIZE             1024

#define SOCKET_PING_SEQ_NO_RANGE    20 // how many pings to consider for quality
#define SOCKET_PING_FAIL_THRESHOLD  10 // 90 percent of ping fails means down

