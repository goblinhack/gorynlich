/*
 * Copyright (C) 2011 Neil McGill
 *
 * See the README file.
 */

unsigned char *file_read(const char *filename, int32_t *len);
int32_t file_write(const char *filename, unsigned char *buffer, int32_t len);
boolean file_exists(const char *filename);
unsigned char *file_read_if_exists(const char *filename, int32_t *out_len);
int32_t file_size(const char *filename);
boolean file_non_zero_size_exists(const char *filename);
boolean file_unlink(const char *filename);
double file_age(const char *filename);
boolean file_exists_and_is_newer_than(const char *filename,
                                      const char *filename2);
