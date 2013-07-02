/*
 * Copyright(C) 2011 Neil McGill
 *
 * See the README file for license.
 */

const unsigned char *ramdisk_load(const char *filename, int32_t *outlen);
unsigned char *ramdisk_load_copy(const char *filename, int32_t *outlen);

typedef struct ramdisk_t_ {
    const char *filename;
    const unsigned char *data;
    int32_t orig_len;
    int32_t len;
    uint8_t uncompressed:1;
} ramdisk_t;

void ramdisk_init(void);
