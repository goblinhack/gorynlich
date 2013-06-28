/*
 * Copyright (C) 2011 Neil McGill
 *
 * See the README file.
 */

unsigned char *miniz_compress(unsigned char *in, int32_t *len);
unsigned char *miniz_compress2(unsigned char *in, int32_t *len, int level);
unsigned char *miniz_uncompress(unsigned char *in, int32_t *len);
