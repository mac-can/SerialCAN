/*  SPDX-License-Identifier: BSD-2-Clause OR GPL-3.0-or-later */
/*
 *  Software for Industrial Communication, Motion Control and Automation
 *
 *  Copyright (c) 2002-2021 Uwe Vogt, UV Software, Berlin (info@uv-software.com)
 *  All rights reserved.
 *
 *  This module is dual-licensed under the BSD 2-Clause "Simplified" License and
 *  under the GNU General Public License v3.0 (or any later version).
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
 *  along with this module.  If not, see <http://www.gnu.org/licenses/>.
 */
/** @file        serial.h
 *
 *  @brief       Serial data transmission.
 *
 *  @author      $Author: eris $
 *
 *  @version     $Rev: 710 $
 *
 *  @defgroup    serial Serial Data Transmission
 *  @{
 */
#ifndef SERIAL_H_INCLUDED
#define SERIAL_H_INCLUDED

#include <stdio.h>
#include <stdint.h>


/*  -----------  options  ------------------------------------------------
 */


/*  -----------  defines  ------------------------------------------------
 */


/*  -----------  types  --------------------------------------------------
 */

typedef void *sio_port_t;               /**< serial port (opaque data type) */

/** @brief       SerialCAN port attributes
 */
typedef struct sio_attr_t_ {            /* serial port attributes: */
    uint32_t baudrate;                  /**<  baud rate (in [bps]) */
    uint8_t  bytesize;                  /**<  number fo data bits (5, 6, 7, 8) */
    uint8_t  parity;                    /**<  parity bit (None, Even, Odd) */
    uint8_t  stopbits;                  /**<  number of stop bits (1 or 2) */
} sio_attr_t;

/** @brief       reception callback function
 *
 *  @param[in]   receiver -  pointer to an instance to handle the received data
 *  @param[in]   buffer   -  data buffer with the received data
 *  @param[in]   nbytes   -  number of received data bytes
 */
typedef void (*sio_recv_t)(const void *receiver, const uint8_t *buffer, size_t nbytes);


/*  -----------  variables  ----------------------------------------------
 */


/*  -----------  prototypes  ---------------------------------------------
 */
#ifdef __cplusplus
extern "C" {
#endif

/** @brief       creates a port instance for communication with a serial
 *               device (constructor).
 *
 *  @param[in]   callback  - pointer to a reception callback function
 *  @param[in]   receiver  - pointer to an instance to handle received data
 *
 *  @returns     a pointer to a port instance if successful, or NULL on error.
 *
 *  @note        System variable 'errno' will be set in case of an error.
 *
 *  @retval      ENOMEM  - out of memory (insufficient storage space)
 */
extern sio_port_t sio_create(sio_recv_t callback, void *receiver);


/** @brief       destroys the port instance (destructor).
 *
 *  @remarks     An established connection will be terminated by this.
 *
 *  @param[in]   port  - pointer to a port instance
 *
 *  @returns     0 if successful, or a negative value on error.
 *
 *  @note        System variable 'errno' will be set in case of an error.
 *
 *  @retval      ENODEV  - no such device (invalid port instance)
 */
extern int sio_destroy(sio_port_t port);


/** @brief       establishes a connection with the serial communication device.
 *
 *  @param[in]   port    - pointer to a port instance
 *  @param[in]   device  - name of the serial device
 *  @param[in]   param   - serial port attributes
 *
 *  @returns     a file descriptor if successful, or a negative value on error.
 *
 *  @note        System variable 'errno' will be set in case of an error.
 *
 *  @retval      ENODEV   - no such device (invalid port instance)
 *  @retval      EINVAL   - invalid argument (device name is NULL)
 *  @retval      EALREADY - already connected with the serial device
 *  @retval      'errno'  - error code from called system functions:
 *                          'open', 'tcsetattr', 'pthread_create'
 */
extern int sio_connect(sio_port_t port, const char *device, const sio_attr_t *param);


/** @brief       terminates the connection with the serial communication device.
 *
 *  @param[in]   port  - pointer to a port instance
 *
 *  @returns     0 if successful, or a negative value on error.
 *
 *  @note        System variable 'errno' will be set in case of an error.
 *
 *  @retval      ENODEV   - no such device (invalid port instance)
 *  @retval      EBADF    - bad file descriptor (device not connected)
 *  @retval      'errno'  - error code from called system functions:
 *                          'pthread_cancle', 'tcflush', 'close'
 */
extern int sio_disconnect(sio_port_t port);


/** @brief       transmits n data bytes via a serial communication device.
 *
 *  @remarks     A connection with the serial communication device must be
 *               established.
 *
 *  @param[in]   port    - pointer to a port instance
 *  @param[in]   buffer  - data buffer with the data to be sent
 *  @param[in]   nbytes  - number of data bytes to be sent
 *
 *  @returns     0 if successful, or a negative value on error.
 *
 *  @note        System variable 'errno' will be set in case of an error.
 *
 *  @retval      ENODEV   - no such device (invalid port instance)
 *  @retval      EINVAL   - invalid argument (buffer is NULL)
 *  @retval      EBADF    - bad file descriptor (device not connected)
 *  @retval      'errno'  - error code from called system functions:
 *                          'write'
 */
extern int sio_transmit(sio_port_t port, const uint8_t *buffer, size_t nbytes);


/** @brief       signals waiting objects, if any.
 *
 *  @param[in]   port  - pointer to a port instance
 *
 *  @returns     0 if successful, or a negative value on error.
 */
extern int sio_signal(sio_port_t port);


#ifdef __cplusplus
}
#endif
#endif /* SERIAL_H_INCLUDED */

/*  ----------------------------------------------------------------------
 *  Uwe Vogt,  UV Software,  Chausseestrasse 33 A,  10115 Berlin,  Germany
 *  Tel.: +49-30-46799872,  Fax: +49-30-46799873,  Mobile: +49-170-3801903
 *  E-Mail: uwe.vogt@uv-software.de,  Homepage: http://www.uv-software.de/
 */
