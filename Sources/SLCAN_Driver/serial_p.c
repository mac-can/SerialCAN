/*
 *  Software for Industrial Communication, Motion Control, and Automation
 *
 *  Copyright (C) 2002-2020  Uwe Vogt, UV Software, Berlin (info@uv-software.com)
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
/** @file        serial.c
 *
 *  @brief       Serial data transmission.
 *
 *  @remarks     POSIX compatible variant (Linux, macOS)
 *
 *  @author      $Author: eris $
 *
 *  @version     $Rev: 690 $
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

#ifndef CBAUDEX
#define BAUDRATE        115200U
#else
#define BAUDRATE        B115200
#endif
#define BYTESIZE        CS8
#define STOPBITS        CSTOPB
#define BUFFER_SIZE     1024


/*  -----------  types  --------------------------------------------------
 */

typedef struct serial_t_ {
    int fildes;
    pthread_t pthread;
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

sio_port_t sio_create(sio_recv_t callback, void *receiver) {
    serial_t *serial = (serial_t*)NULL;

    /* reset errno variable */
    errno = 0;
    /* C language constructor */
    if ((serial = (serial_t*)malloc(sizeof(serial_t))) != NULL) {
        serial->fildes = -1;
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
    (void) sio_disconnect(port);
    /* C language destructor */
    free(serial);
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
    /* there is nothong to signal right now */
    return 0;
}

int sio_connect(sio_port_t port, const char *device, const sio_attr_t *param) {
    serial_t *serial = (serial_t*)port;
    struct termios attr;
    // TODO: set transmission attributes
    (void) param;

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
    /* connect to serial port */
    if ((serial->fildes = open(device, O_RDWR | O_NONBLOCK)) < 0) {
        /* errno set */
        return -1;
    }
    /* set connection attributes */
    tcgetattr(serial->fildes, &attr);
    attr.c_cflag = CREAD | CLOCAL;
    attr.c_cflag |= BYTESIZE;
    attr.c_cflag |= STOPBITS;
#ifdef CBAUDEX
    attr.c_cflag |= BAUDRATE;
#else
    cfsetispeed(&attr, BAUDRATE);
    cfsetospeed(&attr, BAUDRATE);
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

#if (0)
int sio_receive(sio_port_t port, uint8_t *buffer, size_t nbytes, uint16_t timeout) {
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
    // TODO: insert coin here
    return -1;
}
#endif

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
}

/*  ----------------------------------------------------------------------
 *  Uwe Vogt,  UV Software,  Chausseestrasse 33 A,  10115 Berlin,  Germany
 *  Tel.: +49-30-46799872,  Fax: +49-30-46799873,  Mobile: +49-170-3801903
 *  E-Mail: uwe.vogt@uv-software.de,  Homepage: http://www.uv-software.de/
 */
