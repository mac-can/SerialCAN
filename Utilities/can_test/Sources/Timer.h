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
#ifndef TIMER_H_INCLUDED
#define TIMER_H_INCLUDED

#if _MSC_VER > 1000
#pragma once
#endif

#include <stdint.h>
#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>
#endif

#if defined(_WIN32) || defined(_WIN64)
#define CTIMER_WAITABLE_TIMER  // a Windows alternative for usleep()
#endif

class CTimer {
public:
    static const uint32_t USEC = 1U;  // 1 microsecond
    static const uint32_t MSEC = 1000U;  // 1 millisecond
    static const uint32_t SEC = 1000000U;  // 1 second
    static const uint32_t MIN = 60000000U;  // 1 minute
private:
#if !defined(_WIN32) && !defined(_WIN64)
    uint64_t m_u64UntilStop;  // counter value for the desired time-out
#else
    LARGE_INTEGER m_largeFrequency;  // frequency in counts per second
    LONGLONG      m_llUntilStop;     // counter value for the desired time-out
#endif
public:
    CTimer(uint32_t u32Microseconds = 0);
    virtual ~CTimer() {};

    bool Restart(uint32_t u32Timeout);  // restart the timer!
    bool Timeout();                     // time-out occurred?

    static bool Delay(uint32_t u32Delay); // delay timer
};

#endif // TIMER_H_INCLUDED

// $Id: Timer.h 579 2020-03-14 16:08:36Z haumea $  Copyright (C) UV Software //
