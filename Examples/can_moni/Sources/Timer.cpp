//
//  Software for Industrial Communication, Motion Control, and Automation
//
//  Copyright (C) 2002-2020  Uwe Vogt, UV Software, Berlin (info@uv-software.com)
//
//  This class is part of the SourceMedley repository.
//
//  This class is free software: you can redistribute it and/or modify
//  it under the terms of the GNU Lesser General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  This class is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU Lesser General Public License for more details.
//
//  You should have received a copy of the GNU Lesser General Public License
//  along with this class.  If not, see <http://www.gnu.org/licenses/>.
//
#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

#include "Timer.h"

#if !defined(_WIN32) && !defined(_WIN64)
#include <unistd.h>
#include <sys/time.h>
#endif

// TODO: replace `gettimeofday' by `clock_gettime' and `usleep' by `clock_nanosleep'
CTimer::CTimer(uint32_t u32Microseconds) {
#if !defined(_WIN32) && !defined(_WIN64)
    struct timeval tv;
    gettimeofday(&tv, NULL);
    m_u64UntilStop = ((uint64_t)tv.tv_sec * (uint64_t)1000000) + (uint64_t)tv.tv_usec \
                   + ((uint64_t)u32Microseconds);
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
    struct timeval tv;
    gettimeofday(&tv, NULL);
    m_u64UntilStop = ((uint64_t)tv.tv_sec * (uint64_t)1000000) + (uint64_t)tv.tv_usec \
                   + ((uint64_t)u32Microseconds);
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
    struct timeval tv;
    gettimeofday(&tv, NULL);
    u64Now = ((uint64_t)tv.tv_sec * (uint64_t)1000000) + (uint64_t)tv.tv_usec;
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
    return (usleep((useconds_t)u32Microseconds) != 0) ? false : true;
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
        timer = CreateWaitableTimer(NULL, TRUE, NULL);
        SetWaitableTimer(timer, &ft, 0, NULL, NULL, 0);
        WaitForSingleObject(timer, INFINITE);
        CloseHandle(timer);
    }
    return true;
# endif
#endif
}

// $Id: Timer.cpp 579 2020-03-14 16:08:36Z haumea $  Copyright (C) UV Software //
