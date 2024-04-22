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
 *  @remarks     Windows compatible variant (_WIN32 and _WIN64)
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
#include <errno.h>
#include <assert.h>

#include <Windows.h>


/*  -----------  options  ------------------------------------------------
 */


/*  -----------  defines  ------------------------------------------------
 */

#define MIN(x,y)  ((x) < (y) ? (x) : (y))

#define ENTER_CRITICAL_SECTION(que)  do { (void)WaitForSingleObject(que->hMutex, INFINITE); } while(0)
#define LEAVE_CRITICAL_SECTION(que)  do { (void)ReleaseMutex(que->hMutex); } while(0)


/*  -----------  types  --------------------------------------------------
 */

typedef struct object_t_ {
    size_t size;
    size_t used;
    size_t head;
    size_t tail;
    uint8_t *queueElem;
    size_t elemSize;
    HANDLE hMutex;
    HANDLE hEvent;
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
        (void)memset(object, 0x00, sizeof(object_t));
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
        /* create a mutex and an event handle */
        if ((object->hMutex = CreateMutex(
            NULL,             // default security attributes
            FALSE,            // initially not owned
            NULL)) == NULL) {
            errno = ENODEV;
            free(object->queueElem);
            free(object);
            return NULL;
        }
        if ((object->hEvent = CreateEvent(
            NULL,             // default security attributes
            FALSE,            // auto-reset event
            FALSE,            // initial state is nonsignaled
            NULL)) == NULL) {
            errno = ENODEV;
            (void)CloseHandle(object->hMutex);
            free(object->queueElem);
            free(object);
            return NULL;
        }
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
    /* destroy mutex and event handle */
    (void)CloseHandle(object->hEvent);
    (void)CloseHandle(object->hMutex);
    /* destroy the message queue */
    if (object->queueElem)
        free(object->queueElem);
    /* C language destructor */
    free(object);
    return 0;
}

int queue_signal(queue_t buffer) {
    object_t *object = (object_t*)buffer;

    /* sanity check */
    errno = 0;
    if (!object) {
        errno = EFAULT;
        return -1;
    }
    /* signal event object */
    (void)SetEvent(object->hEvent);
    return 0;
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
        (void)SetEvent(object->hEvent);
    }
    else {
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
    if (dequeue_element(object, element, maxbytes)) {
        res = (int)MIN(object->elemSize, maxbytes);
    }
    LEAVE_CRITICAL_SECTION(object);

    /* when no data available - blocking read or polling */
    if (res < 0) {
        if (timeout > 0U) {  /* blocking read */
            switch (WaitForSingleObject(object->hEvent, (timeout != 65535U) ? (DWORD)timeout : INFINITE)) {
            case WAIT_OBJECT_0:     /* event signalled */
                /* - dequeue element (with truncation) */
                ENTER_CRITICAL_SECTION(object);
                if (dequeue_element(object, element, maxbytes)) {
                    res = (int)MIN(object->elemSize, maxbytes);
                }
                LEAVE_CRITICAL_SECTION(object);
                /* - when signalled externally (e.g. by SIGINT) */
                if (res < 0) {
                    errno = ENOMSG;
                    res = -30;
                }
                break;
            case WAIT_TIMEOUT:      /* event timed out */
                errno = ETIMEDOUT;
                res = -30;
                break;
            default:                /* error: no data! */
                errno = ENOMSG;
                res = -30;
                break;
            }
        } else {  /* polling (timeout == 0) */
            errno = ENOMSG;
            res = -30;
        }
    }
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
 *  (�1) empty :  used == 0
 *  (�2) full  :  used == size  &&  size > 0
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
