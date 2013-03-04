/*
 * Copyright (C) 2011 Neil McGill
 *
 * See the README file.
 */

#define  ENABLE_PTRCHECK            // Pointer sanity
#define nENABLE_ASSERT              // DIE on errors
#define nENABLE_TREE_SANITY         // Slow tree sanity
#define nENABLE_DEBUG               // More debugs
#define nENABLE_VIDEO_SYNC          // Vertical sync
#define nENABLE_INVERTED_DISPLAY    // For running on a Mac with inverted gfx
#define nENABLE_GENERATE_TTF        // Edit RUNME to add -lSDL_ttf
#define nENABLE_DEMARSHAL_DEBUG     // Demarshalling, prints structures
#define nENABLE_MAP_DEBUG           // Debug AI by printing the map
#define nENABLE_WID_DEBUG           // Widget debug

#ifdef ENABLE_GENERATE_TTF
#define DISABLE_SDL_WINDOW          // SDL_ttf is incompatible with window mode
#endif

