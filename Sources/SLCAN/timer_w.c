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
 *  @remarks     Windows compatible variant (_WIN32 and _WIN64)
 *
 *  @author      $Author: makemake $
 *
 *  @version     $Rev: 822 $
 *
 *  @addtogroup  timer
 *  @{
 */

/*static const char _id[] = "$Id: timer_w.c 822 2024-08-27 09:23:18Z makemake $";*/


/*  -----------  includes  ----------------------------------------------
 */

#include "timer.h"                      /* interface prototypes */

#include <stdio.h>                      /* standard I/O routines */
#include <errno.h>                      /* system wide error numbers */
#include <string.h>                     /* string manipulation functions */
#include <stdlib.h>                     /* commonly used library functions */
#include <stdbool.h>                    /* standard library boolean data type */
#include <windows.h>                    /* master include file for Windows */

#include <math.h>                       /* mathematical functions */


/*  -----------  options  -----------------------------------------------
 */

#define TIMER_WAITABLE_TIMER  1  /* set to zero to poll the HR timer */


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
    LARGE_INTEGER largeFrequency;       // high-resolution timer frequency
    LARGE_INTEGER largeCounter;         // high-resolution performance counter

    LONGLONG llUntilStop = (LONGLONG)0; // counter value for the desired time-out

    // retrieve the frequency of the high-resolution performance counter
    if (!QueryPerformanceFrequency(&largeFrequency))
        return (timer_obj_t)0;
    // retrieve the current value of the high-resolution performance counter
    if (!QueryPerformanceCounter(&largeCounter))
        return (timer_obj_t)0;
    // calculate the counter value for the desired time-out
    llUntilStop = largeCounter.QuadPart + ((largeFrequency.QuadPart * (LONGLONG)microseconds)
                                                                    / (LONGLONG)1000000);
    return (timer_obj_t)llUntilStop;
}

int timer_restart(timer_obj_t *self, uint64_t microseconds) {
    LARGE_INTEGER largeFrequency;       // high-resolution timer frequency
    LARGE_INTEGER largeCounter;         // high-resolution performance counter

    LONGLONG llUntilStop = 0;           // counter value when to stop (time-out)

    // retrieve the frequency of the high-resolution performance counter
    if (!QueryPerformanceFrequency(&largeFrequency))
        return 0;
    // retrieve the current value of the high-resolution performance counter
    if (!QueryPerformanceCounter(&largeCounter))
        return 0;
    // calculate the counter value for the desired time-out
    llUntilStop = largeCounter.QuadPart + ((largeFrequency.QuadPart * (LONGLONG)microseconds)
                                                                    / (LONGLONG)1000000);
    // update the timer instance or the general purpose timer (NULL)
    if (self)
        *self = (uint64_t)llUntilStop;
    else
         gpt0 = (uint64_t)llUntilStop;
    return 1;
}

int timer_timeout(timer_obj_t *self) {
    LARGE_INTEGER largeFrequency;       // high-resolution timer frequency
    LARGE_INTEGER largeCounter;         // high-resolution performance counter

    LONGLONG llUntilStop = self ? (LONGLONG)*self : (LONGLONG)gpt0;

    // retrieve the frequency of the high-resolution performance counter
    if (!QueryPerformanceFrequency(&largeFrequency))
        return 0;
    // retrieve the current value of the high-resolution performance counter
    if (!QueryPerformanceCounter(&largeCounter))
        return 0;
    // a time-out occurred, if the counter overruns the time-out value
    if (largeCounter.QuadPart < llUntilStop)
        return 0;
    else
        return 1;
}

int timer_delay(uint64_t microseconds) {
#if (TIMER_WAITABLE_TIMER != 0)
    HANDLE timer;
    LARGE_INTEGER ft;

    ft.QuadPart = -(10 * (LONGLONG)microseconds); // Convert to 100 nanosecond interval, negative value indicates relative time

    if (microseconds >= 100) {  // FIXME: Who made this decision?
        if ((timer = CreateWaitableTimer(NULL, TRUE, NULL)) != NULL) {
            SetWaitableTimer(timer, &ft, 0, NULL, NULL, 0);
            WaitForSingleObject(timer, INFINITE);
            CloseHandle(timer);
        }
    }
    else {
        // According to MSDN's documentation for Sleep:
        // | A value of zero causes the thread to relinquish the remainder of its time slice to any other
        // | thread that is ready to run.If there are no other threads ready to run, the function returns
        // | immediately, and the thread continues execution.
        Sleep(0);
    }
    return 1;
#else
    timer_obj_t local = timer_new(microseconds);

    while (!timer_timeout(&local)) {
        Sleep(0);
    }
    return 1;
#endif
}

struct timespec timer_get_time(void) {
    struct timespec now = { 0, 0 };
    static bool fInitialied = false;         // initialization flag
    static struct timespec tsStartTime;      // time at first call (UTC)
    static LARGE_INTEGER largeFrequency;     // frequency in counts per second
    static LARGE_INTEGER largeStartCounter;  // high-resolution performance counter
    LARGE_INTEGER largeCurrentCounter;

    if (!fInitialied) {
        // retrieve the frequency of the high-resolution performance counter
        if (!QueryPerformanceFrequency(&largeFrequency))
            return now;
        // retrieve the value of the high-resolution performance counter
        if (!QueryPerformanceCounter(&largeStartCounter))
            return now;
        // retrieve the time as struct timespec
        if (!timespec_get(&tsStartTime, TIME_UTC))
            return now;
        fInitialied = true;
    }
    // retrieve the current value of the high-resolution performance counter
    if (!QueryPerformanceCounter(&largeCurrentCounter))
        return now;
    // calculate the current time with nanosecond resolution
    largeCurrentCounter.QuadPart -= largeStartCounter.QuadPart;
    time_t sec = largeCurrentCounter.QuadPart / largeFrequency.QuadPart;
    long nsec = (long)(((largeCurrentCounter.QuadPart - (sec * largeFrequency.QuadPart))
              * 1000000000UL) / largeFrequency.QuadPart);
    now.tv_sec = tsStartTime.tv_sec + sec;
    now.tv_nsec = tsStartTime.tv_nsec + nsec;
    if (now.tv_nsec >= 1000000000UL) {
        now.tv_sec += 1;
        now.tv_nsec -= 1000000000UL;
    }
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
