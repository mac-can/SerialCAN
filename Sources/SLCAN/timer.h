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
/** @file        timer.h
 *
 *  @brief       A high-resolution Timer.
 *
 *               Including a general purpose timer (GPT0).
 *
 *  @author      $Author: makemake $
 *
 *  @version     $Rev: 825 $
 *
 *  @defgroup    timer A high-resolution Timer
 *  @{
 */
#ifndef TIMER_H_INCLUDED
#define TIMER_H_INCLUDED

/*  -----------  includes  ----------------------------------------------
 */

#include <time.h>
#include <stdio.h>
#include <stdint.h>


/*  -----------  options  -----------------------------------------------
 */


/*  -----------  defines  -----------------------------------------------
 */

#define TIMER_GPT0          NULL        /**< general purpose timer */

#define TIMER_USEC(x)       (uint64_t)((uint64_t)(x) * (uint64_t)1)
#define TIMER_MSEC(x)       (uint64_t)((uint64_t)(x) * (uint64_t)1000)
#define TIMER_SEC(x)        (uint64_t)((uint64_t)(x) * (uint64_t)1000000)
#define TIMER_MIN(x)        (uint64_t)((uint64_t)(x) * (uint64_t)60000000)


/*  -----------  types  -------------------------------------------------
 */

typedef uint64_t timer_obj_t;           /**< timer object */
typedef uint64_t timer_val_t;           /**< timer value (in [usec]) */


/*  -----------  variables  ---------------------------------------------
 */


/*  -----------  prototypes  --------------------------------------------
 */
#ifdef __cplusplus
extern "C" {
#endif

/** @brief       creates and starts a new timer object.
 *
 *  @param[in]   microseconds  in [usec]
 *
 *  @returns     a timer object on success, or zero otherwise
 */
timer_obj_t timer_new(timer_val_t microseconds);


/** @brief       restarts an expired or running timer object.
 *
 *  @param[in]   self  pointer to a timer object, or NULL (GPT0)
 *  @param[in]   microseconds  in [usec]
 *
 *  @returns     none-zero value on success, or zero otherwise
 */
int timer_restart(timer_obj_t *self, timer_val_t microseconds);


/** @brief       returns True when the given timer object has expired.
 *
 *  @param[in]   self  pointer to a timer object, or NULL (GPT0)
 *
 *  @returns     none-zero value when timed out, or zero otherwise
 */
int timer_timeout(timer_obj_t *self);


/** @brief       suspends the calling thread for the given time period.
 *
 *  @param[in]   microseconds  in [usec]
 *
 *  @returns     none-zero value on success, or zero otherwise
 */
int timer_delay(timer_val_t microseconds);

/** @brief       returns the current time as 'struct timespec'.
 *
 *  @returns     the current time in 'struct timespec'
 */
struct timespec timer_get_time(void);

/** @brief       returns the time difference between two 'struct timespec'.
 *
 *  @param[in]   start  pointer to a 'struct timespec'
 *  @param[in]   stop   pointer to a 'struct timespec'
 *
 *  @returns     the time difference in [sec]
 */
double timer_diff_time(struct timespec *start, struct timespec *stop);


#ifdef __cplusplus
}
#endif
#endif /* TIMER_H_INCLUDED */
/** @}
 */
 /* ----------------------------------------------------------------------
 *  Uwe Vogt,  UV Software,  Chausseestrasse 33 A,  10115 Berlin,  Germany
 *  Tel.: +49-30-46799872,  Fax: +49-30-46799873,  Mobile: +49-170-3801903
 *  E-Mail: uwe.vogt@uv-software.de,  Homepage: http://www.uv-software.de/
 */
