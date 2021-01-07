/*
 *  Software for Industrial Communication, Motion Control, and Automation
 *
 *  Copyright (C) 2016,2020  Uwe Vogt, UV Software, Berlin (info@uv-software.com)
 *
 *  This module is part of the SourceMedley repository.
 *
 *  This module is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This module is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this module.  If not, see <http://www.gnu.org/licenses/>.
 */
/** @file        logger_w.c
 *
 *  @brief       Writing log messages into an ASCII file.
 *
 *  @remarks     Windows compatible variant (_WIN32 and _WIN64)
 *
 *  @author      $Author: haumea $
 *
 *  @version     $Rev: 669 $
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

#include <Windows.h>


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
#define LOG_PIPE_SIZE  16384


/*  -----------  types  --------------------------------------------------
 */


/*  -----------  prototypes  ---------------------------------------------
 */

static DWORD WINAPI logging(LPVOID lpParam);


/*  -----------  variables  ----------------------------------------------
 */

static HANDLE hThread = NULL;
static HANDLE hMutex = NULL;
static HANDLE hPipo, hPipi;
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
    if (!CreatePipe(&hPipo, &hPipi, NULL,   // default security attributes
                                    LOG_PIPE_SIZE)) {
        errno = ENODEV;
        return -1;
    }
    /* create a mutex for mutual write operations into the log file */
    if ((hMutex = CreateMutex(NULL,         // default security attributes
                              FALSE,        // initially not owned
                              NULL)) == NULL) {
        CloseHandle(hPipi);
        CloseHandle(hPipo);
        errno = ENODEV;
        return -1;
    }
    /* create a logging thread for log messages from other threads */
    if ((hThread = CreateThread(NULL,       // default security attributes
                                0,          // use default stack size
                                logging,    // thread function name
                                NULL,       // argument to thread function
                                0,          // use default creation flags
                                NULL)) == NULL) {
        CloseHandle(hMutex);
        CloseHandle(hPipi);
        CloseHandle(hPipo);
        errno = ENODEV;
        return -1;
    }
    /* open a log file (or use stderr when not given) */
    if (pathname) {
        if ((fopen_s(&logger, pathname, "w+")) != 0) {
            /* errno set */
            CloseHandle(hMutex);
            CloseHandle(hPipi);
            CloseHandle(hPipo);
            return -1;
        }
    }
    else {
        logger = LOG_CONSOLE;
    }
    /* write a header into the log file */
    char str[26];
    time_t now = time(NULL);
    ctime_s(str, 26, &now);
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
    (void) TerminateThread(hThread, 0);
    (void) WaitForSingleObject(hThread, 0);
    (void) CloseHandle(hMutex);
    (void) CloseHandle(hPipi);
    (void) CloseHandle(hPipo);
    hThread = NULL;
    hMutex = NULL;
    /* write a footer into the log file */
    char str[26];
    time_t now = time(NULL);
    ctime_s(str, 26, &now);
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
    if (WaitForSingleObject(hMutex, INFINITE) != WAIT_OBJECT_0) {
        errno = ENODEV;
        return -1;
    }
    /* write a log message into the log file */
    fprintf(logger, ">>> (%" PRIu64 ")", n++);
    for (i = 0; i < nbytes; i++)
        fprintf(logger, " %02X", buffer[i]);
    fputc('\n', logger);
    
    /* leave critical section */
    (void) ReleaseMutex(hMutex);

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
    DWORD res;
    if (!WriteFile(hPipi, buffer, (DWORD)nbytes, &res, NULL)) {
        errno = ENODEV;
        return -1;
    }
    /* return the number of bytes written */
    return (int)res;
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
    if (WaitForSingleObject(hMutex, INFINITE) != WAIT_OBJECT_0) {
        errno = ENODEV;
        return -1;
    }
    /* write a formatted string into the log file */
    fputs("+++ ", logger);
    va_start(args, format);
    res = vfprintf(logger, format, args);
    va_end(args);
    fflush(logger);

    /* leave critical section */
    (void) ReleaseMutex(hMutex);

    /* return result from printf */
    return res;
}

static DWORD WINAPI logging(LPVOID lpParam)
{
    uint8_t buffer[LOG_BUF_SIZE];
    DWORD nbytes, i;
    DWORD64 n = 1;
    (void)lpParam;

    for (;;) {
        if (ReadFile(hPipo, buffer, LOG_BUF_SIZE, &nbytes, NULL)) {
            if (logger && (nbytes > 0)) {
                if (WaitForSingleObject(hMutex, INFINITE) == WAIT_OBJECT_0) {
                    fprintf(logger, "<<< (%" PRIu64 ")", n++);
                    for (i = 0; i < nbytes; i++)
                        fprintf(logger, " %02X", buffer[i]);
                    fputc('\n', logger);
                    (void) ReleaseMutex(hMutex);
                }
            }
        }
    }
}

/*  ----------------------------------------------------------------------
 *  Uwe Vogt,  UV Software,  Chausseestrasse 33 A,  10115 Berlin,  Germany
 *  Tel.: +49-30-46799872,  Fax: +49-30-46799873,  Mobile: +49-170-3801903
 *  E-Mail: uwe.vogt@uv-software.de,  Homepage: http://www.uv-software.de/
 */
