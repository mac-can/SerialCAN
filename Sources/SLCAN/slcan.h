/*  SPDX-License-Identifier: BSD-2-Clause OR GPL-3.0-or-later */
/*
 *  Software for Industrial Communication, Motion Control and Automation
 *
 *  Copyright (c) 2002-2024 Uwe Vogt, UV Software, Berlin (info@uv-software.com)
 *  All rights reserved.
 *
 *  Module 'SLCAN'
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
/** @file        slcan.h
 *
 *  @brief       Lawicel SLCAN protocol.
 *
 *  @author      $Author: makemake $
 *
 *  @version     $Rev: 823 $
 *
 *  @defgroup    slcan Lawicel SLCAN Protocol
 *  @{
 */
#ifndef SLCAN_H_INCLUDED
#define SLCAN_H_INCLUDED

#include "serial_attr.h"

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>


/*  -----------  options  ------------------------------------------------
 */

/** @name  Compiler Switches
 *  @brief Options for conditional compilation.
 *  @{ */
/** @note  Set define OPTION_SLCAN_DEBUG_LEVEL to a non-zero value to compile
 *         with logging of the SLCAN protocol (e.g. in the build environment).
 */
#if (OPTION_SLCAN_DLLEXPORT != 0)
#define SLCANAPI  __declspec(dllexport)
#elif (OPTION_SLCAN_DLLIMPORT != 0)
#define SLCANAPI  __declspec(dllimport)
#else
#define SLCANAPI  extern
#endif
/** @} */

/*  -----------  defines  ------------------------------------------------
 */

/** @name  CAN Message flags
 *  @brief CAN frame types (encoded in the CAN identifier)
 *  @{
 */
#define CAN_STD_FRAME   0x00000000U     /**< standard frame format (11-bit) */
#define CAN_XTD_FRAME   0x80000000U     /**< extended frame format (29-bit) */
#define CAN_ERR_FRAME   0x40000000U     /**< error frame (not supported) */
#define CAN_RTR_FRAME   0x20000000U     /**< remote frame */
/** @} */

/** @name  CAN Identifier
 *  @brief CAN identifier masks
 *  @{ */
#define CAN_STD_MASK    0x000007FFU     /**< highest 11-bit identifier */
#define CAN_XTD_MASK    0x1FFFFFFFU     /**< highest 29-bit identifier */
/** @} */

/** @name  CAN Data Length
 *  @brief CAN payload length and DLC definition
 *  @{ */
#define CAN_DLC_MAX     8U              /**< max. data lenth code (CAN 2.0) */
#define CAN_LEN_MAX     8U              /**< max. payload length (CAN 2.0) */
/** @} */

/** @name  CAN Baud Rate Indexes
 *  @brief CAN baud rate indexes defined by SLCAN protocol
 *  @{ */
#define CAN_10K         0U              /**< bit-rate:   10 kbit/s */
#define CAN_20K         1U              /**< bit-rate:   20 kbit/s */
#define CAN_50K         2U              /**< bit-rate:   50 kbit/s */
#define CAN_100K        3U              /**< bit-rate:  100 kbit/s */
#define CAN_125K        4U              /**< bit-rate:  120 kbit/s */
#define CAN_250K        5U              /**< bit-rate:  250 kbit/s */
#define CAN_500K        6U              /**< bit-rate:  500 kbit/s */
#define CAN_800K        7U              /**< bit-rate:  800 kbit/s */
#define CAN_1000K       8U              /**< bit-rate: 1000 kbit/s */
#define CAN_1M          CAN_1000K
/** @} */

#define CAN_INFINITE    65535U          /**< infinite time-out (blocking read) */


/*  -----------  types  --------------------------------------------------
 */

typedef void *slcan_port_t;             /**< SLCAN port (opaque data type) */

typedef sio_attr_t slcan_attr_t;        /**< serial port attributes */

/** @brief  CAN message (SocketCAN compatible)
 */
typedef struct slcan_message_t_ {       /* SLCAN message: */
    uint32_t can_id;                    /**< message identifier */
    uint8_t can_dlc;                    /**< data length code (0..8) */
    uint8_t __pad;                      /**< (padding) */
    uint8_t __res1;                     /**< (resvered for CAN FD) */
    uint8_t __res2;                     /**< (resvered for CAN FD) */
    uint8_t data[CAN_LEN_MAX];          /**< payload (max. 8 data bytes) */
} slcan_message_t;

/** @brief  SLCAN status flags
 */
typedef union slcan_flags_t_ {          /* SLACAN status flags */
    uint8_t byte;                       /**< byte access */
    struct {                            /* bit access: */
        uint8_t BEI : 1;                /**< Bus Error (BEI), see SJA1000 datasheet */
        uint8_t ALI : 1;                /**< Arbitration Lost (ALI), see SJA1000 datasheet */
        uint8_t EPI : 1;                /**< Error Passive (EPI), see SJA1000 datasheet */
        uint8_t : 1;                    /**< (not used) */
        uint8_t DOI : 1;                /**< Data Overrun (DOI), see SJA1000 datasheet */
        uint8_t EI : 1;                 /**< Error warning (EI), see SJA1000 datasheet */
        uint8_t TxFIFO : 1;             /**< CAN transmit FIFO queue full */
        uint8_t RxFIFO : 1;             /**< CAN receive FIFO queue full */
    };
} slcan_flags_t;


/*  -----------  variables  ----------------------------------------------
 */


/*  -----------  prototypes  ---------------------------------------------
 */
#ifdef __cplusplus
extern "C" {
#endif

/** @brief       creates a port instance for communication with a SLCAN compatible
 *               serial device (constructor).
 *
 *  @param[in]   queueSize  - size of the reception queue (number of messages)
 *
 *  @returns     a pointer to a SLCAN instance if successful, or NULL on error.
 *
 *  @note        System variable 'errno' will be set in case of an error.
 *
 *  @retval      ENOMEM  - out of memory (insufficient storage space)
 */
SLCANAPI slcan_port_t slcan_create(size_t queueSize);


/** @brief       destroys the port instance (destructor).
 *
 *  @remarks     An established connection will be terminated by this.
 *
 *  @remarks     Prior to this the CAN controller is set into INIT mode and
 *               the CAN channel is closed (via command 'Close Channel').
 *
 *  @param[in]   port  - pointer to a SLCAN instance
 *
 *  @returns     0 if successful, or a negative value on error.
 *
 *  @note        System variable 'errno' will be set in case of an error.
 *
 *  @retval      ENODEV  - no such device (invalid port instance)
 *  @retval      EFAULT  - bad address (release of resources)
 */
SLCANAPI int slcan_destroy(slcan_port_t port);


/** @brief       establishes a connection with the serial communication device.
 *
 *  @remarks     The SLCAN protocol is check by retrieving version information
 *               of the CAN channel (via command 'HW/SW Version').
 *
 *  @remarks     Precautionary the CAN controller is set into INIT mode and
 *               the CAN channel is closed (via command 'Close Channel').
 *
 *  @param[in]   port    - pointer to a SLCAN instance
 *  @param[in]   device  - name of the serial device
 *  @param[in]   attr    - serial port attributes (optional)
 *
 *  @returns     a file descriptor if successful, or a negative value on error.
 *
 *  @remarks     On Windows, the communication port number (zero based) is returned.
 *
 *  @note        System variable 'errno' will be set in case of an error.
 *
 *  @retval      ENODEV   - no such device (invalid port instance)
 *  @retval      EINVAL   - invalid argument (device name is NULL)
 *  @retval      EALREADY - already connected with the serial device
 *  @retval      'errno'  - error code from called system functions:
 *                          'open', 'tcsetattr', 'pthread_create'
 */
SLCANAPI int slcan_connect(slcan_port_t port, const char *device, const sio_attr_t *attr);


/** @brief       terminates the connection with the serial communication device.
 *
 *  @remarks     Prior to this the CAN controller is set into INIT mode and
 *               the CAN channel is closed (via command 'Close Channel').
 *
 *  @param[in]   port  - pointer to a SLCAN instance
 *
 *  @returns     0 if successful, or a negative value on error.
 *
 *  @note        System variable 'errno' will be set in case of an error.
 *
 *  @retval      ENODEV   - no such device (invalid port instance)
 *  @retval      EBADF    - bad file descriptor (device not connected)
 *  @retval      'errno'  - error code from called system functions:
 *                          'pthread_cancel', 'tcflush', 'close'
 */
SLCANAPI int slcan_disconnect(slcan_port_t port);


/** @brief       returns the serial communication attributes (baudrate, etc.).
 *
 *  @param[in]   port  - pointer to a SLCAN instance
 *  @param[out]  attr  - serial communication attributes
 *
 *  @returns     0 if successful, or a negative value on error.
 *
 *  @note        System variable 'errno' will be set in case of an error.
 *
 *  @retval      ENODEV   - no such device (invalid port instance)
 *  @retval      EINVAL   - invalid argument (attr is NULL)
 */
SLCANAPI int slcan_get_attr(slcan_port_t port, slcan_attr_t *attr);


/** @brief       enables or disables ACK/NACK feedback for serial commands.
 *               Defaults to ACK/NACK feedback enabled (Lawicel protocol).
 *
 *  @param[in]   port  - pointer to a SLCAN instance
 *  @param[in]   on    -
 *
 *  @returns     0 if successful, or a negative value on error.
 *
 *  @note        System variable 'errno' will be set in case of an error.
 *
 *  @retval      ENODEV   - no such device (invalid port instance)
 */
SLCANAPI int slcan_set_ack(slcan_port_t port, bool on);


/** @brief       setup with standard CAN bit-rates.
 *
 *  @remarks     This command is only active if the CAN channel is closed.
 *
 *  @param[in]   port   - pointer to a SLCAN instance
 *  @param[in]   index  -
 *
 *  @returns     0 if successful, or a negative value on error.
 *
 *  @note        System variable 'errno' will be set in case of an error.
 *
 *  @retval      ENODEV    - no such device (invalid port instance)
 *  @retval      EINVAL    - invalid argument (index)
 *  @retval      EBADF     - bad file descriptor (device not connected)
 *  @retval      EBUSY     - device / resource busy (disturbance)
 *  @retval      EBADMSG   - bad message (format or disturbance)
 *  @retval      ETIMEDOUT - timed out (command not acknowledged)
 *  @retval      'errno'   - error code from called system functions:
 *                           'write', 'read', etc.
 */
SLCANAPI int slcan_setup_bitrate(slcan_port_t port, uint8_t index);


/** @brief       setup with BTR0/BTR1 CAN bit-rates; see SJA1000 datasheet.
 *
 *  @remarks     This command is only active if the CAN channel is closed.
 *
 *  @param[in]   port  - pointer to a SLCAN instance
 *  @param[in]   btr   - SJA1000 bit-timing register
 *
 *  @returns     0 if successful, or a negative value on error.
 *
 *  @note        System variable 'errno' will be set in case of an error.
 *
 *  @retval      ENODEV    - no such device (invalid port instance)
 *  @retval      EINVAL    - invalid argument (...)
 *  @retval      EBADF     - bad file descriptor (device not connected)
 *  @retval      EBUSY     - device / resource busy (disturbance)
 *  @retval      EBADMSG   - bad message (format or disturbance)
 *  @retval      ETIMEDOUT - timed out (command not acknowledged)
 *  @retval      'errno'   - error code from called system functions:
 *                           'write', 'read', etc.
 */
SLCANAPI int slcan_setup_btr(slcan_port_t port, uint16_t btr);


/** @brief       opens the CAN channel.
 *
 *  @remarks     This command is only active if the CAN channel is closed and
 *               has been set up prior with either the 'Setup Bitrate' or
 *               'Setup BTR' command (i.e. initiated).
 *
 *  @param[in]   port  - pointer to a SLCAN instance
 *
 *  @returns     0 if successful, or a negative value on error.
 *
 *  @note        System variable 'errno' will be set in case of an error.
 *
 *  @retval      ENODEV    - no such device (invalid port instance)
 *  @retval      EINVAL    - invalid argument (...)
 *  @retval      EBADF     - bad file descriptor (device not connected)
 *  @retval      EBUSY     - device / resource busy (disturbance)
 *  @retval      EBADMSG   - bad message (format or disturbance)
 *  @retval      ETIMEDOUT - timed out (command not acknowledged)
 *  @retval      'errno'   - error code from called system functions:
 *                           'write', 'read', etc.
 */
SLCANAPI int slcan_open_channel(slcan_port_t port);


/** @brief       closes the CAN channel.
 *
 *  @remarks     This command is only active if the CAN channel is open.
 *
 *  @param[in]   port  - pointer to a SLCAN instance
 *
 *  @returns     0 if successful, or a negative value on error.
 *
 *  @note        System variable 'errno' will be set in case of an error.
 *
 *  @retval      ENODEV    - no such device (invalid port instance)
 *  @retval      EINVAL    - invalid argument (...)
 *  @retval      EBADF     - bad file descriptor (device not connected)
 *  @retval      EBUSY     - device / resource busy (disturbance)
 *  @retval      EBADMSG   - bad message (format or disturbance)
 *  @retval      ETIMEDOUT - timed out (command not acknowledged)
 *  @retval      'errno'   - error code from called system functions:
 *                           'write', 'read', etc.
 */
SLCANAPI int slcan_close_channel(slcan_port_t port);


/** @brief       transmits a CAN message.
 *
 *  @remarks     This command is only active if the CAN channel is open.
 *
 *  @param[in]   port     - pointer to a SLCAN instance
 *  @param[in]   message  - pointer to the message to be sent
 *  @param[in]   timeout  - (not implemented yet)
 *
 *  @returns     0 if successful, or a negative value on error.
 *
 *  @note        System variable 'errno' will be set in case of an error.
 *
 *  @retval      ENODEV    - no such device (invalid port instance)
 *  @retval      EINVAL    - invalid argument (message)
 *  @retval      EBADF     - bad file descriptor (device not connected)
 *  @retval      EBUSY     - device / resource busy (disturbance)
 *  @retval      EBADMSG   - bad message (format or disturbance)
 *  @retval      ETIMEDOUT - timed out (command not acknowledged)
 *  @retval      'errno'   - error code from called system functions:
 *                           'write', 'read', etc.
 */
SLCANAPI int slcan_write_message(slcan_port_t port, const slcan_message_t *message, uint16_t timeout);


/** @brief       read one message from the message queue, if any.
 *
 *  @param[in]   port     - pointer to a SLCAN instance
 *  @param[out]  message  - pointer to a message buffer
 *  @param[in]   timeout  - time to wait for the reception of a message:
 *                               0 means the function returns immediately,
 *                               65535 means blocking read, and any other
 *                               value means the time to wait im milliseconds
 *
 *  @returns     0 if successful, or a negative value on error.
 *
 *  @retval      -30  - when the message queue is empty (CAN API compatible)
 *
 *  @note        System variable 'errno' will be set in case of an error.
 *
 *  @retval      ENODEV  - no such device (invalid port instance)
 *  @retval      EINVAL  - invalid argument (message)
 *  @retval      ENOMSG  - no data available (message queue empty)
 *  @retval      ENOSPC  - no space left (message queue overflow)
 *
 *  @remarks     If a message has been successfully read from the message queue,
 *               the value ENOSPC in the system variable 'errno' indicates that
 *               a message queue overflow has occurred and that at least one
 *               CAN message has been lost.
 */
SLCANAPI int slcan_read_message(slcan_port_t port, slcan_message_t *message, uint16_t timeout);


/** @brief       read status flags.
 *
 *  @remarks     This command is only active if the CAN channel is open.
 *
 *  @param[in]   port   - pointer to a SLCAN instance
 *  @param[out]  flags  - channel status flags
 *
 *  @returns     0 if successful, or a negative value on error.
 *
 *  @note        System variable 'errno' will be set in case of an error.
 *
 *  @retval      ENODEV    - no such device (invalid port instance)
 *  @retval      EBADF     - bad file descriptor (device not connected)
 *  @retval      EBUSY     - device / resource busy (disturbance)
 *  @retval      EBADMSG   - bad message (format or disturbance)
 *  @retval      ETIMEDOUT - timed out (command not acknowledged)
 *  @retval      'errno'   - error code from called system functions:
 *                           'write', 'read', etc.
 */
SLCANAPI int slcan_status_flags(slcan_port_t port, slcan_flags_t *flags);


/** @brief       sets Acceptance Code Register (ACn Register of SJA1000).
 *
 *  @remarks     This command is only active if the CAN channel is initiated
 *               and not opened.
 *
 *  @param[in]   port  - pointer to a SLCAN instance
 *  @param[in]   code  - acceptance code register
 *
 *  @returns     0 if successful, or a negative value on error.
 *
 *  @note        System variable 'errno' will be set in case of an error.
 *
 *  @retval      ENODEV    - no such device (invalid port instance)
 *  @retval      EBADF     - bad file descriptor (device not connected)
 *  @retval      EBUSY     - device / resource busy (disturbance)
 *  @retval      EBADMSG   - bad message (format or disturbance)
 *  @retval      ETIMEDOUT - timed out (command not acknowledged)
 *  @retval      'errno'   - error code from called system functions:
 *                           'write', 'read', etc.
 */
SLCANAPI int slcan_acceptance_code(slcan_port_t port, uint32_t code);


/** @brief       sets Acceptance Mask Register (AMn Register of SJA1000).
 *
 *  @remarks     This command is only active if the CAN channel is initiated
 *               and not opened.
 *
 *  @param[in]   port  - pointer to a SLCAN instance
 *  @param[in]   mask  - acceptance mask register
 *
 *  @returns     0 if successful, or a negative value on error.
 *
 *  @note        System variable 'errno' will be set in case of an error.
 *
 *  @retval      ENODEV    - no such device (invalid port instance)
 *  @retval      EBADF     - bad file descriptor (device not connected)
 *  @retval      EBUSY     - device / resource busy (disturbance)
 *  @retval      EBADMSG   - bad message (format or disturbance)
 *  @retval      ETIMEDOUT - timed out (command not acknowledged)
 *  @retval      'errno'   - error code from called system functions:
 *                           'write', 'read', etc.
 */
SLCANAPI int slcan_acceptance_mask(slcan_port_t port, uint32_t mask);


/** @brief       get version number of both SLCAN hardware and software.
 *
 *  @remarks     This command is active always.
 *
 *  @param[in]   port      - pointer to a SLCAN instance
 *  @param[out]  hardware  - hardware version (8-bit: <major>.<minor>)
 *  @param[out]  software  - software version (8-bit: <major>.<minor>)
 *
 *  @returns     0 if successful, or a negative value on error.
 *
 *  @note        System variable 'errno' will be set in case of an error.
 *
 *  @retval      ENODEV    - no such device (invalid port instance)
 *  @retval      EBADF     - bad file descriptor (device not connected)
 *  @retval      EBUSY     - device / resource busy (disturbance)
 *  @retval      EBADMSG   - bad message (format or disturbance)
 *  @retval      ETIMEDOUT - timed out (command not acknowledged)
 *  @retval      'errno'   - error code from called system functions:
 *                           'write', 'read', etc.
 */
SLCANAPI int slcan_version_number(slcan_port_t port, uint8_t *hardware, uint8_t *software);


/** @brief       get serial number of the SLCAN device.
 *
 *  @remarks     This command is active always.
 *
 *  @param[in]   port   - pointer to a SLCAN instance
 *  @param[out]  number - serial number (32-bit: 8 bytes)
 *
 *  @returns     0 if successful, or a negative value on error.
 *
 *  @note        System variable 'errno' will be set in case of an error.
 *
 *  @retval      ENODEV    - no such device (invalid port instance)
 *  @retval      EBADF     - bad file descriptor (device not connected)
 *  @retval      EBUSY     - device / resource busy (disturbance)
 *  @retval      EBADMSG   - bad message (format or disturbance)
 *  @retval      ETIMEDOUT - timed out (command not acknowledged)
 *  @retval      'errno'   - error code from called system functions:
 *                           'write', 'read', etc.
 */
SLCANAPI int slcan_serial_number(slcan_port_t port, uint32_t *number);


/** @brief       signal all waiting objects, if any.
 *
 *  @param[in]   port  - pointer to a SLCAN instance
 *
 *  @returns     0 if successful, or a negative value on error.
 */
SLCANAPI int slcan_signal(slcan_port_t port);


/** @brief       retrieves version information of the SLCAN API as a
 *               zero-terminated string.
 *
 *  @param[out]  version_no - major and minor version number (optional)
 *  @param[out]  patch_no   - patch number (optional)
 *  @param[out]  build_no   - build number (optional)
 *
 *  @returns     pointer to a zero-terminated string, or NULL on error.
 */
SLCANAPI char *slcan_api_version(uint16_t *version_no, uint8_t *patch_no, uint32_t *build_no);


#ifdef __cplusplus
}
#endif
#endif /* SLCAN_H_INCLUDED */

/*  ----------------------------------------------------------------------
 *  Uwe Vogt,  UV Software,  Chausseestrasse 33 A,  10115 Berlin,  Germany
 *  Tel.: +49-30-46799872,  Fax: +49-30-46799873,  Mobile: +49-170-3801903
 *  E-Mail: uwe.vogt@uv-software.de,  Homepage: http://www.uv-software.de/
 */
