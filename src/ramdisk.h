/*
 * Copyright(C) 2011 Neil McGill
 *
 * See the README file for license.
 */

unsigned char *ramdisk_load(const char *filename, int32_t *outlen);

typedef struct ramdisk_t_ {
    const char *filename;
    const unsigned char *data;
    int32_t orig_len;
    int32_t len;
} ramdisk_t;

void ramdisk_init(void);
