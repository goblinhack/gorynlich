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
    /*
     * Texture co-ordinates within the image.
     */
    double x1;
    double y1;
    double x2;
    double y2;
    /*
     * Percentage points that indicate the start of the pixels within the tile 
     * texture for use in collisions.
     */
    double px1;
    double py1;
    double px2;
    double py2;
    int32_t gl_surface_binding;
    texp tex;
} tile;

extern tree_root *tiles;
