/*
 * Copyright (C) 2011 Neil McGill
 *
 * See the README file.
 */

unsigned char *mzip_file_read(const char *filename, int32_t *len);
int32_t mzip_file_write(const char *filename, unsigned char *buf, int32_t *len);
