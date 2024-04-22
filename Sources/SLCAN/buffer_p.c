/*  SPDX-License-Identifier: BSD-2-Clause OR GPL-3.0-or-later */
/*
 *  Software for Industrial Communication, Motion Control and Automation
 *
 *  Copyright (c) 2002-2024 Uwe Vogt, UV Software, Berlin (info@uv-software.com)
 *  All rights reserved.
 *
 *  Module 'buffer'
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
/** @file        buffer.c
 *
 *  @brief       Buffer for intertask communication.
 *
 *  @remarks     POSIX compatible variant (e.g. Linux, macOS)
 *
 *  @author      $Author: quaoar $
 *
 *  @version     $Rev: 811 $
 *
 *  @addtogroup  buffer
 *  @{
 */
#include "buffer.h"

#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <assert.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>


/*  -----------  options  ------------------------------------------------
 */


/*  -----------  defines  ------------------------------------------------
 */

#define MIN(x,y)  ((x) < (y) ? (x) : (y))

#define ENTER_CRITICAL_SECTION(buf)  assert(0 == pthread_mutex_lock(&buf->wait.mutex))
#define LEAVE_CRITICAL_SECTION(buf)  assert(0 == pthread_mutex_unlock(&buf->wait.mutex))

#define GET_TIME(ts)  do{ clock_gettime(CLOCK_REALTIME, &ts); } while(0)
#define ADD_TIME(ts,to)  do{ ts.tv_sec += (time_t)(to / 1000U); \
                             ts.tv_nsec += (long)(to % 1000U) * (long)1000000; \
                             if (ts.tv_nsec >= (long)1000000000) { \
                                 ts.tv_nsec %= (long)1000000000; \
                                 ts.tv_sec += (time_t)1; \
                             } } while(0)

#define SIGNAL_WAIT_CONDITION(buf,flg)  do{ buf->wait.flag = flg; \
                                            assert(0 == pthread_cond_signal(&buf->wait.cond)); } while(0)
#define WAIT_CONDITION_INFINITE(buf,res)  do{ buf->wait.flag = false; \
                                              res = pthread_cond_wait(&buf->wait.cond, &buf->wait.mutex); } while(0)
#define WAIT_CONDITION_TIMEOUT(buf,abs,res)  do{ buf->wait.flag = false; \
                                                 res = pthread_cond_timedwait(&buf->wait.cond, &buf->wait.mutex, &abs); } while(0)

/*  -----------  types  --------------------------------------------------
 */

typedef struct object_t_ {
    size_t maxbytes;
    size_t nbytes;
    void *data;
    struct cond_wait_t {
        pthread_mutex_t mutex;
        pthread_cond_t cond;
        bool flag;
    } wait;
} object_t;


/*  -----------  prototypes  ---------------------------------------------
 */


/*  -----------  variables  ----------------------------------------------
 */


/*  -----------  functions  ----------------------------------------------
 */

buffer_t buffer_create(size_t size) {
    object_t *object = (object_t*)NULL;

    /* reset errno variable */
    errno = 0;
    /* sanity check */
    if (!size) {
        errno = EINVAL;
        return NULL;
    }
    /* C language constructor */
    if ((object = (object_t*)malloc(sizeof(object_t))) != NULL) {
        bzero(object, sizeof(object_t));
        /* create a data buffer for data exchange */
        if ((object->data = malloc(size)) == NULL) {
            /* errno set */
            free(object);
            return NULL;
        }
        object->maxbytes = size;
        object->nbytes = 0;
        /* create a mutex and a waitable condition */
        if ((pthread_mutex_init(&object->wait.mutex, NULL) < 0) ||
            (pthread_cond_init(&object->wait.cond, NULL)) < 0) {
            /* errno set */
            free(object->data);
            free(object);
            return NULL;
        }
        object->wait.flag = false;
    }
    return (buffer_t)object;
}

int buffer_destroy(buffer_t buffer) {
    object_t *object = (object_t*)buffer;

    /* sanity check */
    errno = 0;
    if (!object) {
        errno = EFAULT;
        return -1;
    }
    /* destroy mutex and condition */
    (void) pthread_mutex_destroy(&object->wait.mutex);
    (void) pthread_cond_destroy(&object->wait.cond);
    /* destroy the data buffer */
    if (object->data)
        free(object->data);
    /* C language destructor */
    free(object);
    return 0;
}

int buffer_signal(buffer_t buffer) {
    object_t *object = (object_t*)buffer;
    int res = 0;

    /* sanity check */
    errno = 0;
    if (!object) {
        errno = EFAULT;
        return -1;
    }
    /* signal the wait condition, if waiting */
    ENTER_CRITICAL_SECTION(object);
    SIGNAL_WAIT_CONDITION(object, false);
    LEAVE_CRITICAL_SECTION(object);
    /* return success */
    return res;
}

int buffer_clear(buffer_t buffer) {
    object_t *object = (object_t*)buffer;
    int res = 0;

    /* sanity check */
    errno = 0;
    if (!object) {
        errno = EFAULT;
        return -1;
    }
    /* remove data from buffer, if any */
    ENTER_CRITICAL_SECTION(object);
    res = (int)object->nbytes;
    object->nbytes = 0;
    LEAVE_CRITICAL_SECTION(object);
    /* return number of bytes removed */
    return res;
}

int buffer_put(buffer_t buffer, const void *data, size_t nbytes) {
    object_t *object = (object_t*)buffer;
    int res = 0;

    /* sanity check */
    errno = 0;
    if (!object || !object->data) {
        errno = EFAULT;
        return -1;
    }
    if (!data || !nbytes) {
        errno = EINVAL;
        return -1;
    }
    /* copy data into buffer (with truncation), if empty */
    ENTER_CRITICAL_SECTION(object);
    if (object->nbytes == 0) {
        memcpy(object->data, data, MIN(nbytes, object->maxbytes));
        object->nbytes = MIN(nbytes, object->maxbytes);
        SIGNAL_WAIT_CONDITION(object, true);
        res = (int)object->nbytes;
    } else {  /* not empty */
        errno = EBUSY;
    }
    LEAVE_CRITICAL_SECTION(object);
    /* return number of bytes copied */
    return res;
}

int buffer_get(buffer_t buffer, void *data, size_t maxbytes, uint16_t timeout) {
    object_t *object = (object_t*)buffer;
    int res = 0;
    int waitCond = 0;
    struct timespec absTime;

    GET_TIME(absTime);
    ADD_TIME(absTime, timeout);

    /* sanity check */
    errno = 0;
    if (!object || !object->data) {
        errno = EFAULT;
        return -1;
    }
    if (!data || !maxbytes) {
        errno = EINVAL;
        return -1;
    }
    /* copy buffer into data (with truncation), if any */
    ENTER_CRITICAL_SECTION(object);
again:
    if (object->nbytes != 0) {
        memcpy(data, object->data, MIN(object->nbytes, maxbytes));
        res = (int)MIN(object->nbytes, maxbytes);
        object->nbytes = 0;
    } else {
        if (timeout == 65535U) {  /* infinite blocking read */
            WAIT_CONDITION_INFINITE(object, waitCond);
            if ((waitCond == 0) && object->wait.flag)
                goto again;
            else
                errno = ENOMSG;
        } else if (timeout != 0U) {  /* timed blocking read */
            WAIT_CONDITION_TIMEOUT(object, absTime, waitCond);
            if ((waitCond == 0) && object->wait.flag)
                goto again;
            else
                errno = ETIMEDOUT;
        } else {  /* polling (timeout == 0) */
            errno = ENOMSG;
        }
    }
    LEAVE_CRITICAL_SECTION(object);
    /* return number of bytes copied */
    return res;
}

/*  ----------------------------------------------------------------------
 *  Uwe Vogt,  UV Software,  Chausseestrasse 33 A,  10115 Berlin,  Germany
 *  Tel.: +49-30-46799872,  Fax: +49-30-46799873,  Mobile: +49-170-3801903
 *  E-Mail: uwe.vogt@uv-software.de,  Homepage: http://www.uv-software.de/
 */
