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
 *  @remarks     Windows compatible variant (_WIN32 and _WIN64)
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

#include <Windows.h>


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

#define BAUDRATE        57600U
#define BYTESIZE        8
#define STOPBITS        TWOSTOPBITS
#define IN_QUEUE        4096U
#define OUT_QUEUE       4096U


/*  -----------  types  --------------------------------------------------
 */

typedef struct serial_t_ {
    HANDLE hPort;
    HANDLE hThread;
    sio_attr_t attr;
    sio_recv_t callback;
    void *receiver;
    int running;
} serial_t;


/*  -----------  prototypes  ---------------------------------------------
 */

static DWORD WINAPI reception_loop(LPVOID lpParam);


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
        serial->hPort = INVALID_HANDLE_VALUE;
        serial->hThread = NULL;
        serial->attr.baudrate = BAUDRATE;
        serial->attr.bytesize = BYTESIZE8;
        serial->attr.stopbits = STOPBITS1;
        serial->attr.parity = PARITYNONE;
        serial->callback = callback;
        serial->receiver = receiver;
        serial->running = 0;
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
    // TODO: purge?
    return 0;
}

int sio_connect(sio_port_t port, const char *device, const sio_attr_t *attr) {
    serial_t *serial = (serial_t*)port;
    int comm, n;
    char path[42];
    DCB dcb;
    DWORD errors;
    COMMTIMEOUTS timeouts;

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
    if (serial->hPort != INVALID_HANDLE_VALUE) {
        errno = EALREADY;
        return -1;
    }
    /* set transmission attributes (optional) */
    if (attr) {
        serial->attr.baudrate = attr->baudrate;
        serial->attr.bytesize = attr->bytesize;
        serial->attr.stopbits = attr->stopbits;
        serial->attr.parity = attr->parity;
    }
    /* get comm port number from device name */
    if (((n = sscanf_s(device, "COM%i", &comm)) < 1) &&
        ((n = sscanf_s(device, "\\\\.\\COM%i", &comm)) < 1)) {
        errno = ENOENT;
        return -1;
    }
    /* create a file for the comm port */
    sprintf_s(path, 42, "\\\\.\\COM%i", comm); /* See Q115831 */
#if defined(_UNICODE)
    if ((serial->hPort = CreateFileA(path, (GENERIC_READ | GENERIC_WRITE),
#else
    if ((serial->hPort = CreateFile(path, (GENERIC_READ | GENERIC_WRITE),
#endif
        0,                              // exclusive access
        NULL,                           // no security attrs
        OPEN_EXISTING,                  // default for devices other than files
        0,                              // flags: no overlapped I/O
        NULL)) == INVALID_HANDLE_VALUE) {
        errno = ENODEV;
        return -1;
    }
    /* setup device buffers */
    if (!SetupComm(serial->hPort, IN_QUEUE, OUT_QUEUE)) {
        (void)ClearCommError(serial->hPort, &errors, NULL);
        (void)CloseHandle(serial->hPort);
        errno = ENODEV;
        return -1;
    }
    /* purge any information in the buffer */
    if (!PurgeComm(serial->hPort,
        (PURGE_TXABORT | PURGE_RXABORT | PURGE_TXCLEAR | PURGE_RXCLEAR))) {
        CloseHandle(serial->hPort);
        errno = ENODEV;
        return -1;
    }
    /* set up time-outs for overlapped I/O */
    timeouts.ReadIntervalTimeout = MAXDWORD;
    timeouts.ReadTotalTimeoutMultiplier = 0U;
    timeouts.ReadTotalTimeoutConstant = 0U;
    timeouts.WriteTotalTimeoutMultiplier = 0U;
    timeouts.WriteTotalTimeoutConstant = 0U;
    if (!SetCommTimeouts(serial->hPort, &timeouts)) {
        (void)CloseHandle(serial->hPort);
        errno = ENODEV;
        return -1;
    }
    /* set up the device control block */
    dcb.DCBlength = (DWORD)sizeof(DCB);
    if (!GetCommState(serial->hPort, &dcb)) {
        (void)CloseHandle(serial->hPort);
        errno = ENODEV;
        return -1;
    }
    int parity = (serial->attr.parity != PARITYNONE) ? 1 : 0;
    dcb.BaudRate = serial->attr.baudrate;   // current baud rate
    dcb.fBinary = TRUE;                     // binary mode, no EOF check
    dcb.fParity = parity;                   // enable parity checking
    dcb.fOutxCtsFlow = FALSE;               // CTS output flow control
    dcb.fOutxDsrFlow = FALSE;               // DSR output flow control
    dcb.fDtrControl = DTR_CONTROL_DISABLE;  // DTR flow control type
    dcb.fDsrSensitivity = FALSE;            // DSR sensitivity
    dcb.fTXContinueOnXoff = FALSE;          // XOFF continues Tx
    dcb.fOutX = FALSE;                      // XON/XOFF out flow control
    dcb.fInX = FALSE;                       // XON/XOFF in flow control
    dcb.fErrorChar = FALSE;                 // enable error replacement
    dcb.fNull = FALSE;                      // enable null stripping
    dcb.fRtsControl = RTS_CONTROL_DISABLE;  // RTS flow control
    dcb.fAbortOnError = FALSE;              // abort reads/writes on error
    //dcb.XonLim = 0;                       // transmit XON threshold
    //dcb.XoffLim = 30108;                  // transmit XOFF threshold
    dcb.ByteSize = serial->attr.bytesize;   // number of bits/byte, 4-8
    dcb.Parity = serial->attr.parity;       // 0-4=no,odd,even,mark,space
    dcb.StopBits = serial->attr.stopbits;   // 0,1,2 = 1, 1.5, 2
    dcb.XonChar = 0x11;                     // Tx and Rx XON character
    dcb.XoffChar = 0x13;                    // Tx and Rx XOFF character
    dcb.ErrorChar = 0x00;                   // error replacement character
    dcb.EofChar = 0x1A;                     // end of input character
    //dcb.EvtChar = -3;                     // received event character
    if (!SetCommState(serial->hPort, &dcb)) {
        (void)CloseHandle(serial->hPort);
        errno = ENODEV;
        return -1;
    }
    /* create the reception thread */
    if ((serial->hThread = CreateThread(
        NULL,                           // default security attributes
        0,                              // use default stack size
        reception_loop,                 // thread function name
        port,                           // argument to thread function
        0,                              // use default creation flags
        NULL)) == NULL) {
        (void)CloseHandle(serial->hPort);
        errno = ENODEV;
        return -1;
    }
    /* return the comm port number (zero based) */
    (void)ClearCommError(serial->hPort, &errors, NULL);
    return (comm - 1);
}

int sio_disconnect(sio_port_t port) {
    serial_t *serial = (serial_t*)port;
    DWORD errors;

    /* sanity check */
    errno = 0;
    if (!serial) {
        errno = ENODEV;
        return -1;
    }
    if (serial->hPort == INVALID_HANDLE_VALUE) {
        errno = EBADF;
        return -1;
    }
    /* kill the reception thread */
    serial->running = 0;
    (void)SetEvent(serial->hThread);
    (void)WaitForSingleObject(serial->hThread, 3000);
    (void)CloseHandle(serial->hThread);
    serial->hThread = NULL;
    /* purge all pending transfers */
    (void)PurgeComm(serial->hPort,
        (PURGE_TXABORT | PURGE_RXABORT | PURGE_TXCLEAR | PURGE_RXCLEAR));
    (void)ClearCommError(serial->hPort, &errors, NULL);
    /* disconnect from serial port */
    if (!CloseHandle(serial->hPort)) {
        errno = ENODEV;
        return -1;
    }
    serial->hPort = INVALID_HANDLE_VALUE;
    return 0;
}

int sio_transmit(sio_port_t port, const uint8_t *buffer, size_t nbytes) {
    serial_t *serial = (serial_t*)port;
    DWORD sent = 0U;
    DWORD errors;

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
    if (serial->hPort == INVALID_HANDLE_VALUE) {
        errno = EBADF;
        return -1;
    }
    /* send n bytes (set errno on error) */
    if (!WriteFile(serial->hPort, buffer, (DWORD)nbytes, &sent, NULL)) {
        (void)ClearCommError(serial->hPort, &errors, NULL);
        errno = EBUSY;
        return -1;
    }
    SERIAL_DEBUG_SYNC(buffer, (size_t)sent);
    // TODO: 'blocking write' required?
    return (int)sent;
}

static DWORD WINAPI reception_loop(LPVOID lpParam) {
    serial_t *serial = (serial_t*)lpParam;
    DWORD errors;

    /* sanity check */
    errno = 0;
    if (!serial) {
        errno = ENODEV;
        perror("serial");
        abort();
    }
    if (serial->hPort == INVALID_HANDLE_VALUE) {
        errno = EBADF;
        perror("serial");
        abort();
    }
    if (serial->running) {
        errno = EEXIST;
        perror("serial");
        abort();
    }
    /* Run, Forest, run! */
    serial->running = 1;
    while (serial->running) {
        DWORD nbytes = 0U;
        uint8_t buffer[1];

        if (ReadFile(serial->hPort, buffer, 1, &nbytes, NULL)) {
            SERIAL_DEBUG_ASYNC(buffer, nbytes);
            if ((nbytes > 0) && serial->callback)
                serial->callback(serial->receiver, &buffer[0], (size_t)nbytes);
        }
        else {
            (void)ClearCommError(serial->hPort, &errors, NULL);
        }
    }
    return 0;
}

/*  ----------------------------------------------------------------------
 *  Uwe Vogt,  UV Software,  Chausseestrasse 33 A,  10115 Berlin,  Germany
 *  Tel.: +49-30-46799872,  Fax: +49-30-46799873,  Mobile: +49-170-3801903
 *  E-Mail: uwe.vogt@uv-software.de,  Homepage: http://www.uv-software.de/
 */
