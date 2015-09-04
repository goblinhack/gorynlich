/*
 * Copyright (C) 2011-2015 goblinhack@gmail.com
 *
 * See the README file.
 */

#pragma once

#undef ENABLE_CONSOLE
#undef ENABLE_LOGFILE             // Log stdout and err to a file

#undef ENABLE_LEAKCHECK           // Memory leak check
#undef ENABLE_PTRCHECK            // Check validity of pointers too

#undef ENABLE_MAP_SANITY          // Extra map checks
#define ENABLE_ASSERT             // DIE on errors
#undef ENABLE_TREE_SANITY         // Slow tree sanity
#undef ENABLE_DEBUG               // More debugs
#undef ENABLE_DEMARSHAL_DEBUG     // Demarshalling, prints structures
#undef ENABLE_MAP_DEBUG           // Debug AI by printing the map
#undef ENABLE_WID_DEBUG           // Widget debug

/*
 * Keep loading files simple and in the dir we expect.
 */
#undef ENABLE_DIR_TRAVERSAL_IN_WIDGETS

/*
 * Other windows are darker when popups appear
 */
#undef ENABLE_FADE_OUT_OTHER_WIDGETS_WHEN_FOCUS_LOCKED

/*
 * Retro text
 */
#define ENABLE_LARGE_TEXT_OUTLINE

/*
 * How much the wheel mouse moves.
 */
#define ENABLE_WHEEL_SCROLL_SPEED_SCALE         1.2
#define ENABLE_WHEEL_MAX_SCROLL_SPEED_SCALE     15

#define ENABLE_JOY_SCROLL_SPEED_SCALE           1.0
#define ENABLE_JOY_MAX_SCROLL_SPEED_SCALE       1.5

/*
 * Tooltip appears above mouse. I prefer it to zoom into the top
 */
#undef ENABLE_TOOLTIP_OVER_MOUSE

/*
 * Tries to batch calls to draw arrays for more speed.
 */
#define ENABLE_GL_BULK_DRAW_ARRAYS 

/*
 * So we don't hog all the CPU when not in video sync mode
 */
#define MAIN_LOOP_DELAY             5

/*
 * Max long string size
 */
#define MAXSTR                      1024

/*
 * Light sources
 */
#define MAX_LIGHT_RAYS              360
#define MAX_LIGHTS                  8000

/*
 * Screen sizes.
 */
#define DEFAULT_VIDEO_WIDTH         1008
#define DEFAULT_VIDEO_HEIGHT        672

/*
 * Map. How many tiles across and down.
 */
#define LEVELS_ACROSS               11
#define LEVELS_DOWN                 11

/*
 * How many tiles per map
 */
#define MAP_WIDTH                   96
#define MAP_HEIGHT                  96
#define MAP_CHUNK_WIDTH             32
#define MAP_CHUNK_HEIGHT            32

/*
 * The number of tiles per screen.
 */
#define TILES_SCREEN_WIDTH          26
#define TILES_SCREEN_HEIGHT         17

/*
 * Torchlight flickering parameters.
 */
#define MAP_LIGHT_CANDLELIGHT_FLICKER_RATE      20

/*
 * Any more than this and you can peek over walls.
 */
#define MAP_LIGHT_CANDLELIGHT_FLICKER_RADIUS    20
#define MAP_LIGHT_CANDLELIGHT_RADIUS            30

#define MAP_THINGS_PER_CELL         128

/*
 * Tile pixel width.
 */
#define TILE_WIDTH                  32
#define TILE_HEIGHT                 32

/*
 * The width of wall blocks and floor tiles not counting shadow pixels
 */
#define TILE_PIX_WIDTH              24
#define TILE_PIX_HEIGHT             24

/*
 * The width including shadow pixels.
 */
#define TILE_PIX_WITH_SHADOW_WIDTH  TILE_WIDTH
#define TILE_PIX_WITH_SHADOW_HEIGHT TILE_HEIGHT

/*
 * Padding for tiles on the edge of the smooth scroll. Has to be quite wide so 
 * that we can see the light from other players when off screen.
 */
#define TILES_SCREEN_WIDTH_PAD      TILES_SCREEN_WIDTH
#define TILES_SCREEN_HEIGHT_PAD     TILES_SCREEN_HEIGHT

/*
 * What level 1.x do we start on?
 */
#define LEVEL_INITIAL_RANDOM        5

/*
 * How many levels per exit can we jump?
 */
#define LEVEL_RANDOM_JUMP           3

/*
 * Console
 */
#define CONSOLE_CURSOR_COLOR        PURPLE
#define CONSOLE_CURSOR_OTHER_COLOR  PLUM
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
#undef ENABLE_GENERATE_TTF         // Edit RUNME to add -lSDL_ttf

#define FIXED_FONT                  "data/ttf/emulogic.ttf"
#define VSMALL_FONT                 "data/ttf/emulogic.ttf"
#define SMALL_FONT                  "data/ttf/emulogic.ttf"
#define MED_FONT                    "data/ttf/emulogic.ttf"
#define LARGE_FONT                  "data/ttf/emulogic.ttf"
#define VLARGE_FONT                 "data/ttf/emulogic.ttf"
#define VVLARGE_FONT                "data/ttf/emulogic.ttf"

#define VSMALL_FONT_SIZE            6
#define SMALL_FONT_SIZE             8
#define FIXED_FONT_SIZE             10
#define MED_FONT_SIZE               14
#define LARGE_FONT_SIZE             18
#define VLARGE_FONT_SIZE            24
#define VVLARGE_FONT_SIZE           30
#define MAX_FONT_SIZE               60

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

/*
 * Maximum pre-compression packet size
 */
#define MAX_PACKET_SIZE             4000

/*
 * Debug check to enable sequence numbers and checksum
 */
#undef ENABLE_PAK_EXTRA_HEADER

/*
 * Hex dumps
 */
#undef ENABLE_PACKET_DUMP

/*
 * Above what lenght it is worth compressing
 */
#define PACKET_LEN_COMPRESS_THRESHOLD 200

/*
 * How many tx packets we pull off of the tx queue per frame. This has
 * to be fairly low as windows can't handle sending too much to a
 * socket that is not yet listening.
 */
#define MAX_SOCKET_TX_PACKETS_PER_LOOP 4

/*
 * How many pings to consider for quality
 */
#define SOCKET_PING_SEQ_NO_RANGE    40

/* 
 * 90 percent of ping fails means down
 */
#define SOCKET_PING_FAIL_THRESHOLD  5

/*
 * Maximum number of thing types. This cannot be exceeded without changing
 * the message format in tx updates.
 */
#define THING_MAX                   512

/*
 * Thumb in air. Needs to be big enough so wrap arounds and id reuse is less 
 * common. Max is 64K for tx map sending from server to client.
 *
 * 0  ... 24 * 1024 (walls and obstacles)
 * 24 ... 48 * 1024 (monsters and players)
 * 48 ... 64 * 1024 (client local)
 */
#define THING_ID_MAX                (48*1024)

/*
 * Local client things like explosions we use a separate space beyond the 
 * above.
 */
#define THING_CLIENT_ID_MIN         (THING_ID_MAX + 1)
#define THING_CLIENT_ID_MAX         (64*1024)

/*
 * When converting from thing co-ords where 1.0 is one grid, we must convert
 * to int when sending over the network. This is the granularity.
 */
#define THING_COORD_SCALE                                   100.0

/*
 * How many tiles difference between the client and server do we allow.
 * This allows local moves to occur without the lag of waiting for the
 * server. Set this too high and the client can push through walls. Too
 * low and it will be jerky, jumping back when we have high latency.
 */
#define THING_MAX_SERVER_DISCREPANCY                        5.0

/*
 * Timers
 */
#define DELAY_TENTHS_THING_COLLISION_TEST                   1

/*
 * Client <-> server keepalives. We want the ping to be short as we use that 
 * to drive the start of the game, detecting client and server presence.
 */
#define DELAY_TENTHS_PING                                   10
#define DELAY_TENTHS_PING_WHEN_NO_RESPONSE                  20

/*
 * How often we look at all sockets for quality
 */
#define DELAY_TENTHS_SOCKET_QUALITY_CHECK                   10

/*
 * Client updates of player position to the server
 */
#define DELAY_HUNDREDTHS_CLIENT_TO_SERVER_PLAYER_POSITION   5

/*
 * Updates of client status to server
 */
#define DELAY_TENTHS_CLIENT_TO_SERVER_STATUS                1

/*
 * How often we look at the keyboards
 */
#define DELAY_THOUSANDTHS_PLAYER_POLL                       15

/*
 * Map and player updates from the server
 */
#define DELAY_HUNDREDTHS_SERVER_TO_CLIENT_MAP_UPDATE_SLOW   5
#define DELAY_HUNDREDTHS_SERVER_TO_CLIENT_MAP_UPDATE_FAST   1
#define DELAY_HUNDREDTHS_SERVER_TO_CLIENT_PLAYER_UPDATE     50

/*
 * How frequently certain things get updated to the client.
 */
#define DELAY_THOUSANDTHS_TX_MAP_UPDATE_FAST                50
#define DELAY_THOUSANDTHS_TX_MAP_UPDATE_SLOW                200

#define DELAY_LEVEL_END_HIDE                                (ONESEC * 1)
#define DELAY_LEVEL_END_DESTROY                             (ONESEC * 4)

/*
 * Players start to croak it at 0 and tick down to this and then really croak 
 * it. 8(
 */
#define THING_MIN_HEALTH                                    (-10)
