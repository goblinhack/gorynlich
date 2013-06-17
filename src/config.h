/*
 * Copyright (C) 2011 Neil McGill
 *
 * See the README file.
 */

#define  ENABLE_PTRCHECK            // Pointer sanity
#define nENABLE_ASSERT              // DIE on errors
#define nENABLE_TREE_SANITY         // Slow tree sanity
#define nENABLE_DEBUG               // More debugs
#define  ENABLE_VIDEO_SYNC          // Vertical sync
#define nENABLE_INVERTED_DISPLAY    // For running on a Mac with inverted gfx
#define nENABLE_GENERATE_TTF        // Edit RUNME to add -lSDL_ttf
#define nENABLE_DEMARSHAL_DEBUG     // Demarshalling, prints structures
#define nENABLE_MAP_DEBUG           // Debug AI by printing the map
#define nENABLE_WID_DEBUG           // Widget debug

/*
 * Screen sizes.
 */
#define DEFAULT_VIDEO_WIDTH         1024 // See gl.h for more
#define DEFAULT_VIDEO_HEIGHT        800

/*
 * If you change the below you need to re-enable ENABLE_GEN_LIGHT_MAP_FILE,
 * rebuild and rerun. Then disable the flag and run RUNME.ramdisk so the 
 * updated light map file is built into the image.
 */
#define nENABLE_GEN_LIGHT_MAP_FILE  // Recreate the ray-traced light map file
#define DEFAULT_LIGHT_RAY_LENGTH    35  // how far we can cast light

#define MAP_WIDTH_LOG               8
#define MAP_WIDTH                   (1 << MAP_WIDTH_LOG)
#define MAP_HEIGHT                  MAP_WIDTH
#define MAP_DEPTH                   16 

#ifdef ENABLE_GENERATE_TTF
#define DISABLE_SDL_WINDOW          // SDL_ttf is incompatible with window mode
#endif

