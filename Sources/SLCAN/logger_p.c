/*  SPDX-License-Identifier: BSD-2-Clause OR GPL-3.0-or-later */
/*
 *  Software for Industrial Communication, Motion Control and Automation
 *
 *  Copyright (c) 2002-2024 Uwe Vogt, UV Software, Berlin (info@uv-software.com)
 *  All rights reserved.
 *
 *  Module 'logger'
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
/** @file        logger.c
 *
 *  @brief       Writing log messages into an ASCII file.
 *
 *  @remarks     POSIX compatible variant (e.g. Linux, macOS)
 *
 *  @author      $Author: quaoar $
 *
 *  @version     $Rev: 811 $
 *
 *  @addtogroup  logger
 *  @{
 */
#include "logger.h"

#include <inttypes.h>
#include <string.h>
#include <stdarg.h>
#include <errno.h>
#include <time.h>

#include <unistd.h>
#include <assert.h>
#include <pthread.h>
#include <sys/select.h>
#include <sys/time.h>


/*  -----------  options  ------------------------------------------------
 */


/*  -----------  defines  ------------------------------------------------
 */

#ifndef LOG_BUF_SIZE
#define LOG_BUF_SIZE  1024
#endif
#ifndef LOG_CONSOLE
#define LOG_CONSOLE  stderr
#endif
#define PIPO  0  /* read from pipe */
#define PIPI  1  /* write into pipe */


/*  -----------  types  --------------------------------------------------
 */


/*  -----------  prototypes  ---------------------------------------------
 */

static void *logging(void *arg);


/*  -----------  variables  ----------------------------------------------
 */

static pthread_t thread;
static pthread_mutex_t mutex;
static int fildes[2] = {-1, -1};
static FILE *logger = NULL;


/*  -----------  functions  ----------------------------------------------
 */

int log_init(const char *pathname/*, flags*/)
{
    /* sanity check */
    errno = 0;
    if (logger) {
        errno = EALREADY;
        return -1;
    }
    /* create a pipe for log messages from a critical thread */
    if (pipe(fildes) < 0) {
        /* errno set */
        return -1;
    }
    /* create a mutex for mutual write operations into the log file */
    if (pthread_mutex_init(&mutex, NULL) < 0) {
        /* errno set */
        close(fildes[PIPI]);
        close(fildes[PIPO]);
        return -1;
    }
    /* create a logging thread for log messages from other threads */
    if (pthread_create(&thread, NULL, logging, NULL) < 0) {
        /* errno set */
        pthread_mutex_destroy(&mutex);
        close(fildes[PIPI]);
        close(fildes[PIPO]);
        return -1;
    }
    /* open a log file (or use stderr when not given) */
    if (pathname) {
        if ((logger = fopen(pathname, "w+")) == NULL) {
            /* errno set */
            pthread_mutex_destroy(&mutex);
            close(fildes[PIPI]);
            close(fildes[PIPO]);
            return -1;
        }
    }
    else {
        logger = LOG_CONSOLE;
    }
    /* write a header into the log file */
    time_t now = time(NULL);
    char *str = ctime(&now);
    str[strlen(str)-1] = '\0';
    fprintf(logger, "+++ uv-software Logger (%s) +++\n", str);
    return 0;
}

int log_exit(void)
{
    /* sanity check */
    errno = 0;
    if (!logger) {
        errno = EBADF;
        return -1;
    }
    /* kill the logging thread and release all resources */
    if (pthread_cancel(thread) == 0) {
#if (1)
        assert(pthread_join(thread, NULL) == 0);
#else
        void *res;
        pthread_join(thread, &res);
        if (res != PTHREAD_CANCELED)
            fprintf(stderr, "+++ error: worker thread not cancelled!\n");
#endif
    }
    assert(pthread_mutex_destroy(&mutex) == 0);
    assert(close(fildes[PIPI]) == 0);
    assert(close(fildes[PIPO]) == 0);

    /* write a footer into the log file */
    time_t now = time(NULL);
    char *str = ctime(&now);
    str[strlen(str)-1] = '\0';
    fprintf(logger, "+++ uv-software Logger (%s) +++\n", str);

    /* that´s all folks! */
    if (logger != LOG_CONSOLE)
        fclose(logger);
    logger = NULL;
    return 0;
}

int log_sync(const uint8_t *buffer, size_t nbytes)
{
    static uint64_t n = 1;
    size_t i;

    /* sanity check */
    errno = 0;
    if (!logger) {
        errno = EBADF;
        return -1;
    }
    /* enter critical section */
    assert(pthread_mutex_lock(&mutex) == 0);

    /* write a log message into the log file */
    fprintf(logger, ">>> (%" PRIu64 ")", n++);
    for (i = 0; i < nbytes; i++)
        fprintf(logger, " %02X", buffer[i]);
    fputc('\n', logger);

    /* leave critical section */
    assert(pthread_mutex_unlock(&mutex) == 0);

    /* return the number of bytes written */
    return (int)i;
}

int log_async(const uint8_t *buffer, size_t nbytes)
{
    /* sanity check */
    errno = 0;
    if (!logger) {
        errno = EBADF;
        return -1;
    }
    /* write a log message into the pipe */
    return (int)write(fildes[PIPI], buffer, nbytes);
}

int log_printf(const char *format, ...)
{
    va_list args;
    int res;

    /* sanity check */
    errno = 0;
    if (!logger) {
        errno = EBADF;
        return -1;
    }
    /* enter critical section */
    assert(pthread_mutex_lock(&mutex) == 0);

    /* write a formatted string into the log file */
    fputs("+++ ", logger);
    va_start(args, format);
    res = vfprintf(logger, format, args);
    va_end(args);
    fflush(logger);

    /* leave critical section */
    assert(pthread_mutex_unlock(&mutex) == 0);

    /* return result from printf */
    return res;
}

static void *logging(void *arg)
{
    uint8_t buffer[LOG_BUF_SIZE];
    ssize_t nbytes, i;
    uint64_t n = 1;
    (void) arg;

    fd_set rdfs;
    FD_ZERO(&rdfs);
    FD_SET(fildes[PIPO], &rdfs);

    assert(pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL) == 0);
    assert(pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL) == 0);

    for (;;) {
        do {
            nbytes = read(fildes[PIPO], buffer, LOG_BUF_SIZE);

            if (logger && (nbytes > 0)) {
                assert(pthread_mutex_lock(&mutex) == 0);
                fprintf(logger, "<<< (%" PRIu64 ")", n++);
                for (i = 0; i < nbytes; i++)
                    fprintf(logger, " %02X", buffer[i]);
                fputc('\n', logger);
                assert(pthread_mutex_unlock(&mutex) == 0);
            }
        } while(nbytes > 0);

        if (select(fildes[PIPO]+1, &rdfs, NULL, NULL, NULL) < 0) {
            perror("+++ error");
            return NULL;
        }
    }
}

/*  ----------------------------------------------------------------------
 *  Uwe Vogt,  UV Software,  Chausseestrasse 33 A,  10115 Berlin,  Germany
 *  Tel.: +49-30-46799872,  Fax: +49-30-46799873,  Mobile: +49-170-3801903
 *  E-Mail: uwe.vogt@uv-software.de,  Homepage: http://www.uv-software.de/
 */
