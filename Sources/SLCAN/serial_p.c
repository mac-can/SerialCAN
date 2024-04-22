/*  SPDX-License-Identifier: BSD-2-Clause OR GPL-3.0-or-later */
/*
 *  Software for Industrial Communication, Motion Control and Automation
 *
 *  Copyright (c) 2002-2024 Uwe Vogt, UV Software, Berlin (info@uv-software.com)
 *  All rights reserved.
 *
 *  Module 'serial'
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
/** @file        serial.c
 *
 *  @brief       Serial data transmission.
 *
 *  @remarks     POSIX compatible variant (Linux, macOS)
 *
 *  @author      $Author: quaoar $
 *
 *  @version     $Rev: 811 $
 *
 *  @addtogroup  serial
 *  @{
 */
#include "serial.h"
#include "logger.h"

#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <termios.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/select.h>
#include <sys/time.h>
#include <assert.h>


/*  -----------  options  ------------------------------------------------
 */

#if defined(CBAUD) && !defined(CBAUDEX)
#error Baudrate limited to 38400 by system (unfeasible)
#endif

#if (OPTION_SERIAL_DEBUG_LEVEL > 0)
#define SERIAL_DEBUG_ERROR(...)  log_printf(__VA_ARGS__)
#else
#define SERIAL_DEBUG_ERROR(...)  while (0)
#endif

#if (OPTION_SERIAL_DEBUG_LEVEL > 1)
#define SERIAL_DEBUG_INFO(...)  log_printf(__VA_ARGS__)
#else
#define SERIAL_DEBUG_INFO(...)  while (0)
#endif

#if (OPTION_SERIAL_DEBUG_LEVEL > 2)
#define SERIAL_DEBUG_ASYNC(...)  log_async(__VA_ARGS__)
#define SERIAL_DEBUG_SYNC(...)  log_sync(__VA_ARGS__)
#undef SERIAL_DEBUG_INFO
#define SERIAL_DEBUG_INFO(...)  while (0)
#else
#define SERIAL_DEBUG_ASYNC(...)  while (0)
#define SERIAL_DEBUG_SYNC(...)  while (0)
#endif

/*  -----------  defines  ------------------------------------------------
 */

#define BAUDRATE        57600U
#define BYTESIZE        CS8
#define STOPBITS        CSTOPB
#define BUFFER_SIZE     1024


/*  -----------  types  --------------------------------------------------
 */

typedef struct serial_t_ {
    int fildes;
    pthread_t pthread;
    sio_attr_t attr;
    sio_recv_t callback;
    void *receiver;
} serial_t;


/*  -----------  prototypes  ---------------------------------------------
 */

static void *reception_loop(void *arg);


/*  -----------  variables  ----------------------------------------------
 */


/*  -----------  functions  ----------------------------------------------
 */

#ifdef CBAUDEX
static tcflag_t cbaudex(uint32_t baud) {
    switch (baud) {
        case 50: return B50;
        case 75: return B75;
        case 110: return B110;
        case 134: return B134;
        case 150: return B150;
        case 200: return B200;
        case 300: return B300;
        case 600: return B600;
        case 1200: return B1200;
        case 1800: return B1800;
        case 2400: return B2400;
        case 4800: return B4800;
        case 9600: return B9600;
        case 19200: return B19200;
        case 38400: return B38400;
        case 57600: return B57600;
        case 115200: return B115200;
        case 230400: return B230400;
        case 460800: return B460800;
        case 500000: return B500000;
        case 576000: return B576000;
        case 921600: return B921600;
        case 1000000: return B1000000;
        case 1152000: return B1152000;
        case 1500000: return B1500000;
        case 2000000: return B2000000;
        case 2500000: return B2500000;
        case 3000000: return B3000000;
        default: return B0;
    }
}
#endif

static tcflag_t cbytesize(uint8_t bytesize) {
    switch (bytesize) {
        case BYTESIZE5: return CS5;
        case BYTESIZE6: return CS6;
        case BYTESIZE7: return CS7;
        default: return CS8;
    }
}

static tcflag_t cparity(uint8_t parity) {
    switch (parity) {
        case PARITYODD: return PARENB | PARODD;
        case PARITYEVEN: return PARENB;
        default: return 0;
    }
}

static tcflag_t cstopbits(uint8_t stopbits) {
    switch (stopbits) {
        case STOPBITS2: return CSTOPB;
        default: return 0;
    }
}

sio_port_t sio_create(sio_recv_t callback, void *receiver) {
    serial_t *serial = (serial_t*)NULL;

    /* reset errno variable */
    errno = 0;
    /* C language constructor */
    if ((serial = (serial_t*)malloc(sizeof(serial_t))) != NULL) {
        serial->fildes = -1;
        serial->attr.baudrate = BAUDRATE;
        serial->attr.bytesize = BYTESIZE8;
        serial->attr.parity = PARITYNONE;
        serial->attr.stopbits = STOPBITS1;
        serial->callback = callback;
        serial->receiver = receiver;
    }
    /* return a pointer to the instance */
    return (sio_port_t)serial;
}

int sio_destroy(sio_port_t port) {
    serial_t *serial = (serial_t*)port;

    /* sanity check */
    errno = 0;
    if (!serial) {
        errno = ENODEV;
        return -1;
    }
    /* close opened file (if any) */
    (void)sio_disconnect(port);
    /* C language destructor */
    free(serial);
    return 0;
}

int sio_get_attr(sio_port_t port, sio_attr_t* attr) {
    serial_t* serial = (serial_t*)port;

    /* sanity check */
    errno = 0;
    if (!serial) {
        errno = ENODEV;
        return -1;
    }
    if (!attr) {
        errno = EINVAL;
        return -1;
    }
    /* serial attributes */
    attr->baudrate = serial->attr.baudrate;
    attr->bytesize = serial->attr.bytesize;
    attr->stopbits = serial->attr.stopbits;
    attr->parity = serial->attr.parity;
    return 0;
}

int sio_signal(sio_port_t port) {
    serial_t *serial = (serial_t*)port;

    /* sanity check */
    errno = 0;
    if (!serial) {
        errno = ENODEV;
        return -1;
    }
    /* there is nothing to signal right now */
    return 0;
}

int sio_connect(sio_port_t port, const char *device, const sio_attr_t *param) {
    serial_t *serial = (serial_t*)port;
    struct termios attr;

    /* sanity check */
    errno = 0;
    if (!serial) {
        errno = ENODEV;
        return -1;
    }
    if (!device) {
        errno = EINVAL;
        return -1;
    }
    if (serial->fildes != -1) {
        errno = EALREADY;
        return -1;
    }
    /* set transmission attributes (optional) */
    if (param) {
        serial->attr.baudrate = param->baudrate;
        serial->attr.bytesize = param->bytesize;
        serial->attr.stopbits = param->stopbits;
        serial->attr.parity = param->parity;
        // TODO: range check required?
    }
    /* connect to serial port */
    if ((serial->fildes = open(device, O_RDWR | O_NONBLOCK)) < 0) {
        /* errno set */
        return -1;
    }
    /* set connection attributes */
    tcgetattr(serial->fildes, &attr);
    attr.c_cflag = CREAD | CLOCAL;
    attr.c_cflag |= cbytesize(serial->attr.bytesize);
    attr.c_cflag |= cstopbits(serial->attr.stopbits);
    attr.c_cflag |= cparity(serial->attr.parity);
#ifdef CBAUDEX
    attr.c_cflag |= cbaudex(serial->attr.baudrate);
#else
    cfsetispeed(&attr, serial->attr.baudrate);
    cfsetospeed(&attr, serial->attr.baudrate);
#endif
    attr.c_iflag = 0;
    attr.c_oflag = 0;
    attr.c_lflag = 0;
    tcflush(serial->fildes, TCIOFLUSH);
    if (tcsetattr(serial->fildes, TCSANOW, &attr) < 0) {
        /* errno set */
        close(serial->fildes);
        serial->fildes = -1;
        return -1;
    }
    /* create the reception thread */
    if (pthread_create(&serial->pthread, NULL, reception_loop, (void*)serial) < 0) {
        /* errno set */
        close(serial->fildes);
        serial->fildes = -1;
        return -1;
    }
    /* everything is a file */
    return serial->fildes;
}

int sio_disconnect(sio_port_t port) {
    serial_t *serial = (serial_t*)port;

    /* sanity check */
    errno = 0;
    if (!serial) {
        errno = ENODEV;
        return -1;
    }
    if (serial->fildes == -1) {
        errno = EBADF;
        return -1;
    }
    /* kill the reception thread */
    if (pthread_cancel(serial->pthread) == 0) {
#if (1)
        assert(pthread_join(serial->pthread, NULL) == 0);
#else
        void *res;
        pthread_join(serial->pthread, &res);
        if (res != PTHREAD_CANCELED)
            fprintf(stderr, "+++ error(serial): worker thread not cancelled!\n");
#endif
    }
    /* purge all pending transfers */
    if (tcflush(serial->fildes, TCIOFLUSH) < 0) {
        /* errno set */
        errno = 0;
    }
    /* disconnect from serial port */
    int res = close(serial->fildes);
    if (res >= 0)
        serial->fildes = -1;
    return res;
}

int sio_transmit(sio_port_t port, const uint8_t *buffer, size_t nbytes) {
    serial_t *serial = (serial_t*)port;

    /* sanity check */
    errno = 0;
    if (!serial) {
        errno = ENODEV;
        return -1;
    }
    if (!buffer) {
        errno = EINVAL;
        return -1;
    }
    if (serial->fildes == -1) {
        errno = EBADF;
        return -1;
    }
    /* send n bytes (errno set on error) */
    ssize_t sent = write(serial->fildes, buffer, nbytes);
    SERIAL_DEBUG_SYNC(buffer, (size_t)sent);
    // TODO: 'blocking write' required?
    return (int)sent;
}

static void *reception_loop(void *arg) {
    serial_t *serial = (serial_t*)arg;

    /* sanity check */
    errno = 0;
    if (!serial) {
        errno = ENODEV;
        perror("serial");
        abort();
    }
    if (serial->fildes == -1) {
        errno = EBADF;
        perror("serial");
        abort();
    }
    /* blocking read */
    fd_set rdfs;
    FD_ZERO(&rdfs);
    FD_SET(serial->fildes, &rdfs);

    /* thread cancellation */
    assert(pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL) == 0);
    assert(pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL) == 0);

    /* the torture never stops */
    for (;;) {
        ssize_t nbytes;
        uint8_t buffer[BUFFER_SIZE];

        do {
            nbytes = read(serial->fildes, &buffer, BUFFER_SIZE);
            SERIAL_DEBUG_ASYNC(buffer, nbytes);
            if ((nbytes > 0) && serial->callback)
                serial->callback(serial->receiver, &buffer[0], (size_t)nbytes);
        } while (nbytes > 0);

        if (select(serial->fildes+1, &rdfs, NULL, NULL, NULL) < 0) {
            perror("serial");
            return NULL;
        }
    }
    return NULL;
}

/*  ----------------------------------------------------------------------
 *  Uwe Vogt,  UV Software,  Chausseestrasse 33 A,  10115 Berlin,  Germany
 *  Tel.: +49-30-46799872,  Fax: +49-30-46799873,  Mobile: +49-170-3801903
 *  E-Mail: uwe.vogt@uv-software.de,  Homepage: http://www.uv-software.de/
 */
