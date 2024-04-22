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
 *  @remarks     Windows compatible variant (_WIN32 and _WIN64)
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

#include <Windows.h>


/*  -----------  options  ------------------------------------------------
 */


/*  -----------  defines  ------------------------------------------------
 */

#define MIN(x,y)  ((x) < (y) ? (x) : (y))

#define ENTER_CRITICAL_SECTION(buf)  do { (void)WaitForSingleObject(buf->hMutex, INFINITE); } while(0)
#define LEAVE_CRITICAL_SECTION(buf)  do { (void)ReleaseMutex(buf->hMutex); } while(0)


/*  -----------  types  --------------------------------------------------
 */

typedef struct object_t_ {
    size_t maxbytes;
    size_t nbytes;
    void *data;
    HANDLE hMutex;
    HANDLE hEvent;
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
    /* C language constructor */
    if ((object = (object_t*)malloc(sizeof(object_t))) != NULL) {
        memset(object, 0x00, sizeof(object_t));
        /* create a data buffer for data exchange */
        if ((object->data = malloc(size)) == NULL) {
            /* errno set */
            free(object);
            return NULL;
        }
        object->maxbytes = size;
        object->nbytes = 0;
        /* create a mutex and an event handle */
        if ((object->hMutex = CreateMutex(
            NULL,             // default security attributes
            FALSE,            // initially not owned
            NULL)) == NULL) {
            errno = ENODEV;
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
            free(object);
            return NULL;
        }
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
    /* destroy mutex and event handle */
    (void)CloseHandle(object->hEvent);
    (void)CloseHandle(object->hMutex);
    /* destroy the data buffer */
    if (object->data)
        free(object->data);
    /* C language destructor */
    free(object);
    return 0;
}

int buffer_signal(buffer_t buffer) {
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
    /* reset pending event, if any */
    (void)ResetEvent(object->hEvent);
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
        (void)SetEvent(object->hEvent);
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
    if (object->nbytes != 0) {
        memcpy(data, object->data, MIN(object->nbytes, maxbytes));
        res = (int)MIN(object->nbytes, maxbytes);
        object->nbytes = 0;
    }
    LEAVE_CRITICAL_SECTION(object);

    /* when no data available - blocking read or polling */
    if (res <= 0) {
        if (timeout > 0U) {  /* blocking read */
            switch (WaitForSingleObject(object->hEvent, (timeout != 65535U) ? (DWORD)timeout : INFINITE)) {
            case WAIT_OBJECT_0:     /* event signalled */
                /* copy buffer into data (with truncation) */
                ENTER_CRITICAL_SECTION(object);
                if (object->nbytes != 0) {
                    memcpy(data, object->data, MIN(object->nbytes, maxbytes));
                    res = (int)MIN(object->nbytes, maxbytes);
                    object->nbytes = 0;
                }
                LEAVE_CRITICAL_SECTION(object);
                /* - when signalled externally (e.g. by SIGINT) */
                if (res < 0)
                    errno = ENOMSG;
                break;
            case WAIT_TIMEOUT:      /* event timed out */
                errno = ETIMEDOUT;
                break;
            default:                /* error: no data! */
                errno = ENOMSG;
                break;
            }
        } else {  /* polling (timeout == 0) */
            errno = ENOMSG;
        }
    }
    /* return number of bytes copied, or negative value on error */
    return res;
}

/*  ----------------------------------------------------------------------
 *  Uwe Vogt,  UV Software,  Chausseestrasse 33 A,  10115 Berlin,  Germany
 *  Tel.: +49-30-46799872,  Fax: +49-30-46799873,  Mobile: +49-170-3801903
 *  E-Mail: uwe.vogt@uv-software.de,  Homepage: http://www.uv-software.de/
 */
