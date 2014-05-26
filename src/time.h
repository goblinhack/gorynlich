/*
 * Copyright (C) 2011 Neil McGill
 *
 * See the README file for license.
 */

#pragma once

#include <assert.h>
#include <CoreServices/CoreServices.h>
#include <mach/mach.h>
#include <mach/mach_time.h>
#include <unistd.h>

#include <sys/time.h>
#include "sdl.h"

uint8_t time_have_x_thousandths_passed_since(uint32_t tenths, uint32_t since);
uint8_t time_have_x_hundredths_passed_since(uint32_t tenths, uint32_t since);
uint8_t time_have_x_tenths_passed_since(uint32_t tenths, uint32_t since);
uint8_t time_have_x_secs_passed_since(uint32_t tenths, uint32_t since);
const char *time2str(uint32_t ms, char *buf, int32_t len);
const char *timestamp(char *buf, int32_t len);
const char *unixtime2str(time_t *time);
long long
timeval_diff(struct timeval *difference,
             struct timeval *end_time,
             struct timeval *start_time);

extern uint32_t time_now;
extern uint32_t SDL_GetTicks(void);

static inline uint32_t time_get_time_cached (void)
{
    return (time_now);
}

static inline uint32_t time_update_time_milli (void)
{
    static uint32_t base_time_in_mill;

#if 0
    //
    // Some macos specific way of getting the time that looks like it could
    // be useful, so leaving around
    //
    uint64_t abs_time = mach_absolute_time();
    Nanoseconds nano_time = AbsoluteToNanoseconds( *(AbsoluteTime *) &abs_time );
    uint64_t nano_val = * (uint64_t *) &nano_time;;
    uint32_t time_in_mill = nano_val / 1000000LLU;

    if (!base_time_in_mill) {
        base_time_in_mill = time_in_mill;
    }

    time_now = (time_in_mill - base_time_in_mill);

    return (time_now);
#endif

    if (!sdl_init_video || HEADLESS) {
        struct timeval  tv;

        gettimeofday(&tv, NULL);

        uint32_t time_in_mill = 
                (tv.tv_sec) * 1000 + (tv.tv_usec) / 1000;

        if (!base_time_in_mill) {
            base_time_in_mill = time_in_mill;
        }

        time_now = (time_in_mill - base_time_in_mill);

        return (time_now);
    }

    time_now = SDL_GetTicks();

    return (time_now);
}

static inline uint32_t time_get_time_milli (void)
{
    return (time_get_time_cached());
}
