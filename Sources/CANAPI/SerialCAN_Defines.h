/*  SPDX-License-Identifier: BSD-2-Clause OR GPL-3.0-or-later */
/*
 *  CAN Interface API, Version 3 (for CAN-over-Serial-Line Interfaces)
 *
 *  Copyright (c) 2004-2024 Uwe Vogt, UV Software, Berlin (info@uv-software.com)
 *  All rights reserved.
 *
 *  This file is part of SerialCAN.
 *
 *  SerialCAN is dual-licensed under the BSD 2-Clause "Simplified" License
 *  and under the GNU General Public License v3.0 (or any later version). You can
 *  choose between one of them if you use SerialCAN in whole or in part.
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
 *  SerialCAN IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 *  DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 *  FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 *  DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 *  SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 *  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 *  OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *  OF SerialCAN, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *  GNU General Public License v3.0 or later:
 *  SerialCAN is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  SerialCAN is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with SerialCAN.  If not, see <https://www.gnu.org/licenses/>.
 */
/** @file        SerialCAN_Defines.h
 *
 *  @brief       CAN API V3 for generic CAN Interfaces - Definitions and Options
 *
 *  @author      $Author: eris $
 *
 *  @version     $Rev: 1398 $
 *
 *  @addtogroup  can_api
 *  @{
 */
#ifndef CANAPI_SERIALCAN_H_INCLUDED
#define CANAPI_SERIALCAN_H_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

/*  -----------  includes  ------------------------------------------------
 */

#include <stdint.h>                     /* C99 header for sized integer types */
#include <stdbool.h>                    /* C99 header for boolean type */


/*  -----------  options  ------------------------------------------------
 */

/** @name  Compiler Switches
 *  @brief Options for conditional compilation.
 *  @{ */
/** @note  Set define OPTION_CAN_2_0_ONLY to a non-zero value to compile
 *         with CAN 2.0 frame format only (e.g. in the build environment).
 */
#ifndef OPTION_DISABLED
#define OPTION_DISABLED  0  /**< if a define is not defined, it is automatically set to 0 */
#endif
/** @} */

/*  -----------  defines  ------------------------------------------------
 */

/** @name  CAN API Interfaces
 *  @brief Serial line interfaces
 *  @{ */
#define CANSIO_BOARDS               0  /**< number of serial line interfaces */
/** @} */

/** @name  Protocol option
 *  @brief SLCAN protocol option
 *  @{ */
#define CANSIO_LAWICEL           0x00U  /**< Lawicel SLCAN protocol */
#define CANSIO_CANABLE           0x01U  /**< CANable SLCAN protocol */
#define CANSIO_AUTO              0xFFU  /**< auto detect (not realized yet) */
#define CANSIO_SLCAN    CANSIO_LAWICEL  /**< Lawicel SLCAN protocol (default) */
 /** @} */

 /** @name  Baud rate option
  *  @brief Baud rate in bits per second
  *  @note  CBAUDEX compatible (e.g. Linux)
  *  @{ */
#define CANSIO_BD57600          57600U  /**< 57.6 kBd */
#define CANSIO_BD115200        115200U  /**< 115.2 kBd */
#define CANSIO_BD230400        230400U  /**< 230.4 kBd */
#define CANSIO_BD460800        460800U  /**< 460.8 kBd */
#define CANSIO_BD500000        500000U  /**< 500.0 kBd */
#define CANSIO_BD576000        576000U  /**< 576.0 kBd */
#define CANSIO_BD921600        921600U  /**< 921.6 kBd */
#define CANSIO_BD1000000      1000000U  /**< 1.000 MBd */
#define CANSIO_BD1152000      1152000U  /**< 1.152 MBd */
#define CANSIO_BD1500000      1500000U  /**< 1.500 MBd */
#define CANSIO_BD2000000      2000000U  /**< 2.000 MBd */
#define CANSIO_BD2500000      2500000U  /**< 2.500 MBd */
#define CANSIO_BD3000000      3000000U  /**< 3.000 MBd */
/** @} */                   

/** @name  Data size option
 *  @brief Number of data bits (5, 6, 7, 8)
 *  @{ */
#define CANSIO_5DATABITS            5U  /**< 5 bits per data byte */
#define CANSIO_6DATABITS            6U  /**< 6 bits per data byte */
#define CANSIO_7DATABITS            7U  /**< 7 bits per data byte */
#define CANSIO_8DATABITS            8U  /**< 8 bits per data byte */
/** @} */

/** @name  Parity option
 *  @brief Parity bit (None, Even, Odd)
 *  @{ */
#define CANSIO_NOPARITY             0U  /**< no parity */
#define CANSIO_ODDPARITY            1U  /**< odd parity */
#define CANSIO_EVENPARITY           2U  /**< even parity */
/** @} */

/** @name  Stop bit option
 *  @brief Number of stop bits (1 or 2)
 *  @{ */
#define CANSIO_1STOPBIT             1U  /**< 1 stop bit */
#define CANSIO_2STOPBITS            2U  /**< 2 stop bits */
/** @} */

/** @name  CAN API Property Value
 *  @brief SLCAN parameter to be read or written
 *  @{ */
#define SLCAN_DEVICE_ID          0x00U  /**< device id number */
#define SLCAN_SERIAL_NUMBER      0x01U  /**< device serial number */
#define SLCAN_HARDWARE_VERSION   0x02U  /**< device hardware version */
#define SLCAN_FIRMWARE_VERSION   0x03U  /**< device firmware version */
#define SLCAN_CLOCK_FREQUENCY    0x05U  /**< CAN clock frequency (in [Hz]) */
// TODO: define more or all parameters
// ...
/** @} */

/** @name  CAN API Library ID
 *  @brief Library ID and dynamic library names
 *  @{ */
#define SLCAN_LIB_ID             900    /**< library ID (CAN/COP API V1 compatible) */
#if defined(_WIN32) || defined (_WIN64)
#define SLCAN_LIB_DRIVER        "(driverless)"
#define SLCAN_LIB_WRAPPER       "u3canslc.dll"
#elif defined(__linux__)
#define SLCAN_LIB_DRIVER        "(driverless)"
#define SLCAN_LIB_WRAPPER       "libuvcanslc.so"
#elif defined(__APPLE__)
#define SLCAN_LIB_DRIVER        "(driverless)"
#define SLCAN_LIB_WRAPPER       "libUVCANSLC.dylib"
#elif defined(__CYGWIN__)
#define SLCAN_LIB_DRIVER        "(driverless)"
#define SLCAN_LIB_WRAPPER       "libuvcanslc.so"
#else
#error Platform not supported
#endif
 /** @} */

 /** @name  Miscellaneous
  *  @brief More or less useful stuff
  *  @{ */
#define SLCAN_LIB_VENDOR        "(unknown)"
#define SLCAN_LIB_WEBSITE       "http://info.cern.ch/hypertext/WWW/TheProject.html"
#define SLCAN_LIB_HAZARD_NOTE   "If you connect your CAN device to a real CAN network when using this library,\n" \
                                "you might damage your application."
  /** @} */

  /*  -----------  types  --------------------------------------------------
 */

/** @brief SerialCAN port attributes
 */
typedef struct can_sio_attr_t_ {        /* serial port attributes: */
    uint32_t baudrate;                  /**<  baud rate (in [bps]) */
    uint8_t  bytesize;                  /**<  number fo data bits (5, 6, 7, 8) */
    uint8_t  parity;                    /**<  parity bit (None, Even, Odd) */
    uint8_t  stopbits;                  /**<  number of stop bits (1 or 2) */
    uint8_t  protocol;                  /**<  protocol (defaul: Lawicel) */
} can_sio_attr_t;

/** @brief SerialCAN port parameters
 */
typedef struct can_sio_param_t_ {       /* device parameters: */
    char* name;                         /**< name of the serial port device */
    can_sio_attr_t attr;                /**< serial communication attributes*/
} can_sio_param_t;


#ifdef __cplusplus
}
#endif
#endif /* CANAPI_SERIALCAN_H_INCLUDED */
/** @}
 */
/*  ----------------------------------------------------------------------
 *  Uwe Vogt,  UV Software,  Chausseestrasse 33 A,  10115 Berlin,  Germany
 *  Tel.: +49-30-46799872,  Fax: +49-30-46799873,  Mobile: +49-170-3801903
 *  E-Mail: uwe.vogt@uv-software.de, Homepage: https://www.uv-software.de/
 */
