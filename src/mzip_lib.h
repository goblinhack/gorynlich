/*
 * Copyright (C) 2011-2015 goblinhack@gmail.com
 *
 * See the README file for license info.
 */

#pragma once

unsigned char *miniz_compress(unsigned char *in, int32_t *len);
unsigned char *miniz_compress2(unsigned char *in, int32_t *len, int level);
unsigned char *miniz_uncompress(unsigned char *in, int32_t *len);
void miniz_fini(void);
