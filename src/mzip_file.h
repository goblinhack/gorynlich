/*
 * Copyright (C) 2011-2015 goblinhack@gmail.com
 *
 * See the README file for license info.
 */

#pragma once

unsigned char *mzip_file_read(const char *filename, int32_t *len);
int32_t mzip_file_write(const char *filename, unsigned char *buf,
                        int32_t *len);
int32_t mzip_file_write2(const char *filename, unsigned char *buf,
                         int32_t *len, int level);
