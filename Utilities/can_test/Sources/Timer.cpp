//  SPDX-License-Identifier: BSD-2-Clause OR GPL-3.0-or-later
//
//  Software for Industrial Communication, Motion Control and Automation
//
//  Copyright (c) 2002-2023 Uwe Vogt, UV Software, Berlin (info@uv-software.com)
//  All rights reserved.
//
//  This class is dual-licensed under the BSD 2-Clause "Simplified" License and
//  under the GNU General Public License v3.0 (or any later version).
//  You can choose between one of them if you use this class.
//
//  BSD 2-Clause "Simplified" License:
//  Redistribution and use in source and binary forms, with or without
//  modification, are permitted provided that the following conditions are met:
//  1. Redistributions of source code must retain the above copyright notice, this
//     list of conditions and the following disclaimer.
//  2. Redistributions in binary form must reproduce the above copyright notice,
//     this list of conditions and the following disclaimer in the documentation
//     and/or other materials provided with the distribution.
//
//  THIS CLASS IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
//  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
//  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
//  DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
//  FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
//  DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
//  SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
//  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
//  OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
//  OF THIS CLASS, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
//  GNU General Public License v3.0 or later:
//  This class is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  This class is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this class.  If not, see <http://www.gnu.org/licenses/>.
//
#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

#include "Timer.h"

#if !defined(_WIN32) && !defined(_WIN64)
#include <errno.h>
#include <unistd.h>
#include <sys/time.h>
#endif
#define POSIX_DEPRECATED  0  /* set to non-zero value to use 'gettimeofday' and 'usleep'*/

CTimer::CTimer(uint32_t u32Microseconds) {
#if !defined(_WIN32) && !defined(_WIN64)
#if (POSIX_DEPRECATED != 0)
    struct timeval tv;
    gettimeofday(&tv, NULL);
    m_u64UntilStop = ((uint64_t)tv.tv_sec * (uint64_t)1000000) + (uint64_t)tv.tv_usec
                   + ((uint64_t)u32Microseconds);
#else
    struct timespec now = { 0, 0 };
    clock_gettime(CLOCK_MONOTONIC, &now);
    m_u64UntilStop = ((uint64_t)now.tv_sec * (uint64_t)1000000)
                   + ((uint64_t)now.tv_nsec / (uint64_t)1000)
                   + ((uint64_t)u32Microseconds);
#endif
#else
    LARGE_INTEGER largeCounter;  // high-resolution performance counter

    // retrieve the frequency of the high-resolution performance counter
    if(!QueryPerformanceFrequency(&m_largeFrequency))
        return;
    // retrieve the current value of the high-resolution performance counter
    if(!QueryPerformanceCounter(&largeCounter))
        return;
    // calculate the counter value for the desired time-out
    m_llUntilStop = largeCounter.QuadPart + ((m_largeFrequency.QuadPart * (LONGLONG)u32Microseconds)
                                                                        / (LONGLONG)1000000);
#endif
}

bool CTimer::Restart(uint32_t u32Microseconds) {
#if !defined(_WIN32) && !defined(_WIN64)
#if (POSIX_DEPRECATED != 0)
    struct timeval tv;
    gettimeofday(&tv, NULL);
    m_u64UntilStop = ((uint64_t)tv.tv_sec * (uint64_t)1000000) + (uint64_t)tv.tv_usec
                   + ((uint64_t)u32Microseconds);
#else
    struct timespec now = { 0, 0 };
    clock_gettime(CLOCK_MONOTONIC, &now);
    m_u64UntilStop = ((uint64_t)now.tv_sec * (uint64_t)1000000)
                   + ((uint64_t)now.tv_nsec / (uint64_t)1000)
                   + ((uint64_t)u32Microseconds);
#endif
    return true;
#else
    LARGE_INTEGER largeCounter;  // high-resolution performance counter

    // retrieve the current value of the high-resolution performance counter
    if(!QueryPerformanceCounter(&largeCounter))
        return false;
    // calculate the counter value for the desired time-out
    m_llUntilStop = largeCounter.QuadPart + ((m_largeFrequency.QuadPart * (LONGLONG)u32Microseconds)
                                                                        / (LONGLONG)1000000);
    return true;
#endif
}

bool CTimer::Timeout() {
#if !defined(_WIN32) && !defined(_WIN64)
    uint64_t u64Now;
#if (POSIX_DEPRECATED != 0)
    struct timeval tv;
    gettimeofday(&tv, NULL);
    u64Now = ((uint64_t)tv.tv_sec * (uint64_t)1000000) + (uint64_t)tv.tv_usec;
#else
    struct timespec now = { 0, 0 };
    clock_gettime(CLOCK_MONOTONIC, &now);
    u64Now = ((uint64_t)now.tv_sec * (uint64_t)1000000)
           + ((uint64_t)now.tv_nsec / (uint64_t)1000);
#endif
    if(u64Now < this->m_u64UntilStop)
        return false;
    else
        return true;
#else
    LARGE_INTEGER largeCounter;  // high-resolution performance counter

    // retrieve the current value of the high-resolution performance counter
    if(!QueryPerformanceCounter(&largeCounter))
        return false;
    // a time-out occurred, if the counter overruns the time-out value
    if(largeCounter.QuadPart < m_llUntilStop)
        return false;
    else
        return true;
#endif
}

bool CTimer::Delay(uint32_t u32Microseconds) {
#if !defined(_WIN32) && !defined(_WIN64)
#if (POSIX_DEPRECATED != 0)
    return (usleep((useconds_t)u32Microseconds) != 0) ? false : true;
#else
    int rc;
    struct timespec delay;
    delay.tv_sec = (time_t)(u32Microseconds / CTimer::SEC);
    delay.tv_nsec = (long)((u32Microseconds % CTimer::SEC) * (uint32_t)1000);
    errno = 0;
    while ((rc = nanosleep(&delay, &delay))) {
        if (errno != EINTR)
            break;
    }
    return (rc != 0) ? false : true;
#endif
#else
# ifndef CTIMER_WAITABLE_TIMER
    LARGE_INTEGER largeFrequency;  // frequency in counts per second
    LARGE_INTEGER largeCounter;    // high-resolution performance counter
    LONGLONG      llUntilStop;     // counter value for the desired delay

    // retrieve the current value of the high-resolution performance counter
    if(!QueryPerformanceCounter(&largeCounter))
        return false;
    // retrieve the frequency of the high-resolution performance counter
    if(!QueryPerformanceFrequency(&largeFrequency))
        return false;
    // calculate the counter value for the desired delay
    llUntilStop = largeCounter.QuadPart + ((largeFrequency.QuadPart * (LONGLONG)u32Microseconds)
                                                                    / (LONGLONG)1000000);
    // wait until the counter overruns the delay time
    for(;;)
    {
        if(!QueryPerformanceCounter(&largeCounter))
            return false;
        if(largeCounter.QuadPart >= llUntilStop)
            return true;
    }
# else
    HANDLE timer;
    LARGE_INTEGER ft;

    ft.QuadPart = -(10 * (LONGLONG)u32Microseconds); // Convert to 100 nanosecond interval, negative value indicates relative time

    if(u32Microseconds >= 100) {  // FIXME: Who made this decision?
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
    return true;
# endif
#endif
}

struct timespec CTimer::GetTime() {
    struct timespec now = { 0, 0 };
#if !defined(_WIN32) && !defined(_WIN64)
    clock_gettime(CLOCK_REALTIME, &now);
#else
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
#endif
    return now;
}

double CTimer::DiffTime(struct timespec start, struct timespec stop) {
    return (((double)stop.tv_sec  + ((double)stop.tv_nsec  / 1000000000.f)) -
            ((double)start.tv_sec + ((double)start.tv_nsec / 1000000000.f)));
}

// $Id: Timer.cpp 799 2023-10-07 19:15:23Z makemake $  Copyright (c) UV Software, Berlin //
