/*  SPDX-License-Identifier: BSD-2-Clause OR GPL-3.0-or-later */
/*
 *  Software for Industrial Communication, Motion Control and Automation
 *
 *  Copyright (c) 2002-2024 Uwe Vogt, UV Software, Berlin (info@uv-software.com)
 *  All rights reserved.
 *
 *  Module 'timer'
 *
 *  This module is dual-licensed under the BSD 2-Clause "Simplified" License
 *  and under the GNU General Public License v3.0 (or any later version).
 *  You can choose between one of them if you use this module.
 *
 *  BSD 2-Clause "Simplified" License:
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are met:
 *  1. Redistributions of source code must retain the above copyright notice, this
 *     list of conditions and the following disclaimer.
 *  2. Redistributions in binary form must reproduce the above copyright notice,
 *     this list of conditions and the following disclaimer in the documentation
 *     and/or other materials provided with the distribution.
 *
 *  THIS MODULE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 *  DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 *  FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 *  DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 *  SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 *  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 *  OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *  OF THIS MODULE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *  GNU General Public License v3.0 or later:
 *  This module is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This module is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this module.  If not, see <https://www.gnu.org/licenses/>.
 */
/** @file        timer.c
 *
 *  @brief       A high-resolution Timer.
 *
 *  @remarks     POSIX compatible variant (Linux, macOS)
 *
 *  @author      $Author: makemake $
 *
 *  @version     $Rev: 822 $
 *
 *  @addtogroup  timer
 *  @{
 */

/*static const char _id[] = "$Id: timer_p.c 822 2024-08-27 09:23:18Z makemake $";*/


/*  -----------  includes  ----------------------------------------------
 */

#include "timer.h"                      /* interface prototypes */

#include <stdio.h>                      /* standard I/O routines */
#include <errno.h>                      /* system wide error numbers */
#include <string.h>                     /* string manipulation functions */
#include <stdlib.h>                     /* commonly used library functions */
#include <unistd.h>                     /* standard symbolic constants and types */
#include <sys/time.h>                   /* time types */

#include <math.h>                       /* mathematical functions */


/*  -----------  options  -----------------------------------------------
 */

#define POSIX_DEPRECATED  0  /* set to non-zero value to use 'gettimeofday' and 'usleep'*/


/*  -----------  defines  -----------------------------------------------
 */


/*  -----------  types  -------------------------------------------------
 */


/*  -----------  prototypes  --------------------------------------------
 */


/*  -----------  variables  ---------------------------------------------
 */

static timer_obj_t gpt0 = 0;            /**< general purpose timer (GPT0) */


/*  -----------  functions  ---------------------------------------------
 */

timer_obj_t timer_new(uint64_t microseconds) {
#if (POSIX_DEPRECATED != 0)
    struct timeval tv;
    gettimeofday(&tv, NULL);
    uint64_t llUntilStop = ((uint64_t)tv.tv_sec * (uint64_t)1000000) + (uint64_t)tv.tv_usec
                         + ((uint64_t)microseconds);
#else
    struct timespec now = { 0, 0 };
    clock_gettime(CLOCK_MONOTONIC, &now);
    uint64_t llUntilStop = ((uint64_t)now.tv_sec * (uint64_t)1000000)
                         + ((uint64_t)now.tv_nsec / (uint64_t)1000)
                         + ((uint64_t)microseconds);
#endif
    return (timer_obj_t)llUntilStop;
}

int timer_restart(timer_obj_t *self, uint64_t microseconds) {
#if (POSIX_DEPRECATED != 0)
    struct timeval tv;
    gettimeofday(&tv, NULL);
    uint64_t llUntilStop = ((uint64_t)tv.tv_sec * (uint64_t)1000000) + (uint64_t)tv.tv_usec
                         + ((uint64_t)microseconds);
#else
    struct timespec now = { 0, 0 };
    clock_gettime(CLOCK_MONOTONIC, &now);
    uint64_t llUntilStop = ((uint64_t)now.tv_sec * (uint64_t)1000000)
                         + ((uint64_t)now.tv_nsec / (uint64_t)1000)
                         + ((uint64_t)microseconds);
#endif
    // update the timer instance or the general purpose timer (NULL)
    if (self)
        *self = (uint64_t)llUntilStop;
    else
         gpt0 = (uint64_t)llUntilStop;
    return 1;
}

int timer_timeout(timer_obj_t *self) {
    uint64_t llNow;
    uint64_t llUntilStop = self ? (uint64_t)*self : (uint64_t)gpt0;
#if (POSIX_DEPRECATED != 0)
    struct timeval tv;
    gettimeofday(&tv, NULL);
    llNow = ((uint64_t)tv.tv_sec * (uint64_t)1000000) + (uint64_t)tv.tv_usec;
#else
    struct timespec now = { 0, 0 };
    clock_gettime(CLOCK_MONOTONIC, &now);
    llNow = ((uint64_t)now.tv_sec * (uint64_t)1000000)
          + ((uint64_t)now.tv_nsec / (uint64_t)1000);
#endif
    if (llNow < llUntilStop)
        return 0;
    else
        return 1;
}

int timer_delay(uint64_t microseconds) {
#if (POSIX_DEPRECATED != 0)
    return (usleep((useconds_t)microseconds) != 0) ? 0 : 1;
#else
    int rc;
    struct timespec delay;
    delay.tv_sec = (time_t)(microseconds / TIMER_SEC(1));
    delay.tv_nsec = (long)((microseconds % TIMER_SEC(1)) * (uint64_t)1000);
    errno = 0;
    while ((rc = nanosleep(&delay, &delay))) {
        if (errno != EINTR)
            break;
    }
    return (rc != 0) ? 0 : 1;
#endif
}

struct timespec timer_get_time(void) {
    struct timespec now = { 0, 0 };
    clock_gettime(CLOCK_REALTIME, &now);
    return now;
}

double timer_diff_time(struct timespec *start, struct timespec *stop) {
    if (!start || !stop)
        return INFINITY;
    return (((double)stop->tv_sec  + ((double)stop->tv_nsec  / 1000000000.f)) -
            ((double)start->tv_sec + ((double)start->tv_nsec / 1000000000.f)));
}

/*  -----------  local functions  ---------------------------------------
 */


/** @}
 */
/*  ----------------------------------------------------------------------
 *  Uwe Vogt,  UV Software,  Chausseestrasse 33 A,  10115 Berlin,  Germany
 *  Tel.: +49-30-46799872,  Fax: +49-30-46799873,  Mobile: +49-170-3801903
 *  E-Mail: uwe.vogt@uv-software.de,  Homepage: http://www.uv-software.de/
 */
