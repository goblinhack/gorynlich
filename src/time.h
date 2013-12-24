/*
 * Copyright (C) 2011 Neil McGill
 *
 * See the README file for license.
 */

#include <sys/time.h>
#include "sdl.h"

uint32_t time_get_time_cached(void);
boolean time_have_x_tenths_passed_since(uint32_t tenths, uint32_t since);
boolean time_have_x_secs_passed_since(uint32_t tenths, uint32_t since);
const char *time2str(uint32_t ms, char *buf, int32_t len);
const char *timestamp(char *buf, int32_t len);
const char *unixtime2str(time_t *time);
long long
timeval_diff(struct timeval *difference,
             struct timeval *end_time,
             struct timeval *start_time);

static inline uint32_t time_get_time_milli (void)
{
    extern uint32_t time_now;
    extern uint32_t SDL_GetTicks(void);

    if (!sdl_init_video || HEADLESS) {
        static uint32_t base_time_in_mill;
        struct timeval  tv;

        gettimeofday(&tv, NULL);

        uint32_t time_in_mill = 
                (tv.tv_sec) * 1000 + (tv.tv_usec) / 1000;

        if (!base_time_in_mill) {
            base_time_in_mill = time_in_mill;
        }

        return (time_in_mill - base_time_in_mill);
    }

    time_now = SDL_GetTicks();

    return (time_now);
}
