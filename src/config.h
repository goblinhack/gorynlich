/*
 * Copyright (C) 2011 Neil McGill
 *
 * See the README file.
 */

#undef ENABLE_DEV_HACKS           // Remove for final build
#undef ENABLE_PTRCHECK            // Pointer sanity
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

/*
 * Screen sizes.
 */
#define DEFAULT_VIDEO_WIDTH         1024 // See gl.h for more
#define DEFAULT_VIDEO_HEIGHT        800

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
#define MAP_DEPTH                   1 

/*
 * This is multiplied by jigsaw piece size.
 */
#define MAP_JIGSAW_PIECE_WIDTH      13
#define MAP_JIGSAW_PIECE_HEIGHT     5

#ifdef ENABLE_GENERATE_TTF
#define DISABLE_SDL_WINDOW          // SDL_ttf is incompatible with window mode
#endif
