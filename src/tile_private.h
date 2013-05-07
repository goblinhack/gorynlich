/*
 * Copyright (C) 2011 Neil McGill
 *
 * See the README file.
 */

typedef struct tile_ {
    tree_key_string tree;
    /*
     * Index within the overall texture, left to right, top to bottom.
     */
    uint32_t index;
    uint32_t width;
    uint32_t height;
    float x1;
    float y1;
    float x2;
    float y2;
    int32_t gl_surface_binding;
    texp tex;
} tile;

extern tree_root *tiles;
