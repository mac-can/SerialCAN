/*  SPDX-License-Identifier: BSD-2-Clause OR GPL-3.0-or-later */
/*
 *  Software for Industrial Communication, Motion Control and Automation
 *
 *  Copyright (c) 2002-2024 Uwe Vogt, UV Software, Berlin (info@uv-software.com)
 *  All rights reserved.
 *
 *  Module 'queue'
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
/** @file        queue.c
 *
 *  @brief       Queue for intertask communication.
 *
 *  @remarks     POSIX compatible variant (e.g. Linux, macOS)
 *
 *  @author      $Author: quaoar $
 *
 *  @version     $Rev: 811 $
 *
 *  @addtogroup  queue
 *  @{
 */
#include "queue.h"

#include <string.h>
#include <stdlib.h>
#include <limits.h>
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

#define GET_TIME(ts)  do{ clock_gettime(CLOCK_REALTIME, &ts); } while(0)
#define ADD_TIME(ts,to)  do{ ts.tv_sec += (time_t)(to / 1000U); \
                             ts.tv_nsec += (long)(to % 1000U) * (long)1000000; \
                             if (ts.tv_nsec >= (long)1000000000) { \
                                 ts.tv_nsec %= (long)1000000000; \
                                 ts.tv_sec += (time_t)1; \
                             } } while(0)

#define ENTER_CRITICAL_SECTION(que)  assert(0 == pthread_mutex_lock(&que->wait.mutex))
#define LEAVE_CRITICAL_SECTION(que)  assert(0 == pthread_mutex_unlock(&que->wait.mutex))

#define SIGNAL_WAIT_CONDITION(que,flg)  do{ que->wait.flag = flg; \
                                            assert(0 == pthread_cond_signal(&que->wait.cond)); } while(0)
#define WAIT_CONDITION_INFINITE(que,res)  do{ que->wait.flag = false; \
                                              res = pthread_cond_wait(&que->wait.cond, &que->wait.mutex); } while(0)
#define WAIT_CONDITION_TIMEOUT(que,abs,res)  do{ que->wait.flag = false; \
                                                 res = pthread_cond_timedwait(&que->wait.cond, &que->wait.mutex, &abs); } while(0)

/*  -----------  types  --------------------------------------------------
 */

typedef struct object_t_ {
    size_t size;
    size_t used;
    size_t head;
    size_t tail;
    uint8_t *queueElem;
    size_t elemSize;
    struct cond_wait_t {
        pthread_mutex_t mutex;
        pthread_cond_t cond;
        bool flag;
    } wait;
    struct overflow_t {
        bool flag;
        uint64_t counter;
    } ovfl;
} object_t;


/*  -----------  prototypes  ---------------------------------------------
 */

static bool enqueue_element(object_t *queue, const void *element, size_t nbytes);
static bool dequeue_element(object_t *queue, void *element, size_t maxbytes);


/*  -----------  variables  ----------------------------------------------
 */


/*  -----------  functions  ----------------------------------------------
 */

queue_t queue_create(size_t numElem, size_t elemSize) {
    object_t *object = (object_t*)NULL;

    /* reset errno variable */
    errno = 0;
    /* sanity check */
    if (!numElem || !elemSize) {
        errno = EINVAL;
        return NULL;
    }
    /* C language constructor */
    if ((object = (object_t*)malloc(sizeof(object_t))) != NULL) {
        bzero(object, sizeof(object_t));
        /* create a fixed size queue for data exchenage */
        if ((object->queueElem = malloc(numElem * elemSize)) == NULL) {
            /* errno set */
            free(object);
            return NULL;
        }
        object->elemSize = elemSize;
        object->size = numElem;
        object->used = 0;
        object->head = 0;
        object->tail = 0;
        object->ovfl.flag = false;
        object->ovfl.counter = 0U;
        /* create a mutex and a waitable condition */
        if ((pthread_mutex_init(&object->wait.mutex, NULL) < 0) ||
            (pthread_cond_init(&object->wait.cond, NULL)) < 0) {
            /* errno set */
            free(object->queueElem);
            free(object);
            return NULL;
        }
        object->wait.flag = false;
    }
    return (object_t*)object;
}

int queue_destroy(queue_t queue) {
    object_t *object = (object_t*)queue;

    /* sanity check */
    errno = 0;
    if (!object) {
        errno = EFAULT;
        return -1;
    }
    /* destroy mutex and condition */
    (void)pthread_mutex_destroy(&object->wait.mutex);
    (void)pthread_cond_destroy(&object->wait.cond);
    /* destroy the message queue */
    if (object->queueElem)
        free(object->queueElem);
    /* C language destructor */
    free(object);
    return 0;
}

int queue_signal(queue_t queue) {
    object_t *object = (object_t*)queue;
    int res = -1;

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

int queue_clear(queue_t queue) {
    object_t *object = (object_t*)queue;
    int res = -1;

    /* sanity check */
    errno = 0;
    if (!object) {
        errno = EFAULT;
        return -1;
    }
    /* remove elements from queue, if any */
    ENTER_CRITICAL_SECTION(object);
    res = (int)object->used;
    object->used = 0;
    object->head = 0;
    object->tail = 0;
    object->ovfl.flag = false;
    object->ovfl.counter = 0U;
    LEAVE_CRITICAL_SECTION(object);
    /* return number of elements removed */
    return res;
}

bool queue_overflow(queue_t queue, uint64_t *counter) {
    object_t *object = (object_t*)queue;
    bool res = false;

    /* sanity check */
    errno = 0;
    if (!object) {
        errno = EFAULT;
        return false;
    }
    /* get overflow flag from queue */
    ENTER_CRITICAL_SECTION(object);
    res = object->ovfl.flag;
    if (counter)
        *counter = object->ovfl.counter;
    LEAVE_CRITICAL_SECTION(object);
    /* return overflow flag */
    return res;
}

int queue_enqueue(queue_t queue, const void *element, size_t nbytes) {
    object_t *object = (object_t*)queue;
    int res = -1;

    /* sanity check */
    errno = 0;
    if (!object) {
        errno = EFAULT;
        return -1;
    }
    if (!element || !nbytes) {
        errno = EINVAL;
        return -1;
    }
    /* enqueue element (with truncation), if queue not full */
    ENTER_CRITICAL_SECTION(object);
    if (enqueue_element(object, element, nbytes)) {
        res = (int)MIN(object->elemSize, nbytes);
        SIGNAL_WAIT_CONDITION(object, true);
    } else {
        errno = ENOSPC;
        res = -20;
    }
    LEAVE_CRITICAL_SECTION(object);
    /* return number of bytes enqueued, or negative value on error */
    return res;
}

int queue_dequeue(queue_t queue, void *element, size_t maxbytes, uint16_t timeout) {
    object_t *object = (object_t*)queue;
    int res = -1;
    int waitCond = 0;
    struct timespec absTime;

    GET_TIME(absTime);
    ADD_TIME(absTime, timeout);

    /* sanity check */
    errno = 0;
    if (!object) {
        errno = EFAULT;
        return -1;
    }
    if (!element || !maxbytes) {
        errno = EINVAL;
        return -1;
    }
    /* dequeue element (with truncation), if queue not empty */
    ENTER_CRITICAL_SECTION(object);
again:
    if (dequeue_element(object, element, maxbytes)) {
        res = (int)MIN(object->elemSize, maxbytes);
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
        res = -30;
    }
    LEAVE_CRITICAL_SECTION(object);
    /* return number of bytes dequeued, or negative value on error */
    return res;
}

/*  ---  FIFO  ---
 *
 *  size :  total number of elements
 *  head :  read position of the queue
 *  tail :  write position of the queue
 *  used :  number of queued elements
 *
 *  (§1) empty :  used == 0
 *  (§2) full  :  used == size  &&  size > 0
 */
static bool enqueue_element(object_t *queue, const void *element, size_t nbytes) {
    assert(queue);
    assert(element);
    assert(queue->size);
    assert(queue->elemSize);
    assert(queue->queueElem);

    if (queue->used < queue->size) {
        if (queue->used != 0U)
            queue->tail = (queue->tail + 1U) % queue->size;
        else
            queue->head = queue->tail;  /* to make sure */
        (void)memcpy(&queue->queueElem[(queue->tail * queue->elemSize)], element, MIN(queue->elemSize, nbytes));
        queue->used += 1U;
        return true;
    } else {
        queue->ovfl.counter += 1U;
        queue->ovfl.flag = true;
        return false;
    }
}

static bool dequeue_element(object_t *queue, void *element, size_t maxbytes) {
    assert(queue);
    assert(element);
    assert(queue->size);
    assert(queue->elemSize);
    assert(queue->queueElem);

    if (queue->used > 0U) {
        (void)memcpy(element, &queue->queueElem[(queue->head * queue->elemSize)], MIN(queue->elemSize, maxbytes));
        queue->head = (queue->head + 1U) % queue->size;
        queue->used -= 1U;
        return true;
    } else
        return false;
}

/*  ----------------------------------------------------------------------
 *  Uwe Vogt,  UV Software,  Chausseestrasse 33 A,  10115 Berlin,  Germany
 *  Tel.: +49-30-46799872,  Fax: +49-30-46799873,  Mobile: +49-170-3801903
 *  E-Mail: uwe.vogt@uv-software.de,  Homepage: http://www.uv-software.de/
 */
