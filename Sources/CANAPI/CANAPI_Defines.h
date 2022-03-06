/*  SPDX-License-Identifier: BSD-2-Clause OR GPL-3.0-or-later */
/*
 *  CAN Interface API, Version 3 (Definitions and Options)
 *
 *  Copyright (c) 2004-2021 Uwe Vogt, UV Software, Berlin (info@uv-software.com)
 *  All rights reserved.
 *
 *  This file is part of CAN API V3.
 *
 *  CAN API V3 is dual-licensed under the BSD 2-Clause "Simplified" License and
 *  under the GNU General Public License v3.0 (or any later version).
 *  You can choose between one of them if you use this file.
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
 *  CAN API V3 IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 *  DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 *  FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 *  DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 *  SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 *  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 *  OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *  OF CAN API V3, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *  GNU General Public License v3.0 or later:
 *  CAN API V3 is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  CAN API V3 is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with CAN API V3.  If not, see <http://www.gnu.org/licenses/>.
 */
/** @file        CANAPI_Defines.h
 *
 *  @brief       CAN API V3 for generic CAN Interfaces - Definitions and Options
 *
 *  @author      $Author: eris $
 *
 *  @version     $Rev: 993 $
 *
 *  @addtogroup  can_api
 *  @{
 */
#ifndef CANAPI_DEFINES_H_INCLUDED
#define CANAPI_DEFINES_H_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

/*  -----------  includes  ------------------------------------------------
 */

#include <stdint.h>                     /* C99 header for sized integer types */
#include <stdbool.h>                    /* C99 header for boolean type */


/*  -----------  options  ------------------------------------------------
 */


/*  -----------  defines  ------------------------------------------------
 */

/** @name  Library IDs
 *  @brief Unique IDs to identify a CAN API library (CAN API V1 compatible)
 *  @{ */
#define CANLIB_IXXAT_VCI        100     /**< IXXAT Virtual CAN interfaces */
#define CANLIB_IXXAT_CAC        800     /**< IXXAT canAnalyzer/32 Client */
#define CANLIB_PEAK_PCAN        200     /**< PEAK PCAN interfaces */
#define CANLIB_VECTOR_XL        300     /**< Vector XL-Driver library */
#define CANLIB_PCANBASIC        400     /**< PEAK PCAN-Basic interfaces */
#define CANLIB_RUSOKU_LT        500     /**< Rusuko TouCAN interfaces */
#define CANLIB_KVASER_32        600     /**< Kvaser CANLIB (canlib32) */
#define CANLIB_ROCKETCAN        700     /**< CAN-over-IP (RocketCAN) */
#define CANLIB_SERIALCAN        900     /**< Serial-Line (SerialCAN) */
#define CANLIB_SOCKETCAN        1000    /**< Linux CAN (SocketCAN) */
#define CANLIB_CANAPILIB        (-1)    /**< CAN API Main Library */
/** @note  Peak's PCAN-Light DLL is outdated, so the library ID can be reused.
 *  @} */

/** @name  Library Names
 *  @brief Filenames of the CAN API libraries (depending on the platform)
 *  @{ */
#if defined(__linux__)
 #define CANAPI_PLATFORM       "Linux"
 #define CANDLL_CANAPILIB      "libuvcan300.so.2"
 #define CANDLL_SERIALCAN      "libuvcanslc.so.1"
 #define CANDLL_SOCKETCAN      "libuvcansoc.so.1"

 #define CAN200_SOCKETCAN      "libu2cansoc.so.1"
#elif defined(__APPLE__)
 #define CANAPI_PLATFORM       "macOS"
 #define CANDLL_CANAPILIB      "libUVCAN300.dylib"
 #define CANDLL_PCANBASIC      "libUVCANPCB.dylib"
 #define CANDLL_RUSOKU_LT      "libUVCANTOU.dylib"
 #define CANDLL_SERIALCAN      "libUVCANSLC.dylib"
 #define CANDLL_KVASERCAN      "libUVCANKVD.dylib"
 #define CANDLL_PEAK_PCAN      "libUVCANPCD.dylib"
#else
 #ifdef _WIN64
 #define CANAPI_PLATFORM       "x64"
 #else
 #define CANAPI_PLATFORM       "x86"
 #endif
 #define CANDLL_CANAPILIB      "uvcan300.dll"
 #define CANDLL_PCANBASIC      "u3canpcb.dll"
 #define CANDLL_KVASER_32      "u3cankvl.dll"
 #define CANDLL_RUSOKU_LT      "u3cantou.dll"
 #define CANDLL_SERIALCAN      "u3canslc.dll"

 #define CAN200_CANAPILIB      "uvcan200.dll"
 #define CAN200_IXXAT_VCI      "uvcanvci.dll"
 #define CAN200_IXXAT_CAC      "uvcancac.dll"
 #define CAN200_PEAK_PCAN      "uvcanpcl.dll"
 #define CAN200_VECTOR_XL      "uvcanvxl.dll"
 #define CAN200_PCANBASIC      "uvcanpcb.dll"
 #define CAN200_UVS_TCPIP      "uvcantcp.dll"
 #define CAN200_UCAN_VCP       "uvcanvcp.dll"

 #define CAN100_CANAPILIB      "uvcan100.dll"
 #define CAN100_VECTOR_XL      "u1canvxl.dll"
 #define CAN100_PCANBASIC      "u1canpcb.dll"
 #define CAN100_ROCKETCAN      "u1canroc.dll"
 #define CAN100_SERIALCAN      "u1canslc.dll"
#endif
/** @} */

#define CANDEV_NETWORK        (-1)      /**< channel ID for network device */
#define CANDEV_SERIAL         (-1)      /**< channel ID for serial port device */

#define SYSERR_OFFSET         (-10000)  /**< offset for system errors */

/** @name  Miscellaneous
 *  @brief More or less useful stuff
 *  @{ */
#define CAN_API_VENDOR         "UV Software, Berlin"
#define CAN_API_AUTHOR         "Uwe Vogt, UV Software"
#define CAN_API_WEBSITE        "www.uv-software.com"
#define CAN_API_CONTACT        "info@uv-software.com"
#define CAN_API_LICENSE        "BSD-2-Clause OR GPL-3.0-or-later"
#define CAN_API_COPYRIGHT      "Copyright (c) 2005-20%02u, UV Software, Berlin"
#define CAN_API_HAZARD_NOTE    "Do not connect your CAN device to a real CAN network when using this program.\n" \
                               "This can damage your application."
/** @} */


/*  -----------  types  --------------------------------------------------
 */

/** @brief Linux-CAN (aka SocketCAN) parameters
 */
typedef struct can_netdev_param_t_ {    /* device parameters: */
    char* ifname;                       /**< interface name (string) */
    int   family;                       /**< protocol family (PF_CAN) */
    int   type;                         /**< communication semantics (SOCK_RAW) */
    int   protocol;                     /**< protocol to be used (CAN_RAW) */
} can_netdev_param_t;

///** @brief  CAN-over-Internet-Protocol (RocketCAN) parameters
// */
//typedef struct can_net_param_t_ {       /* device parameters: */
//    uint32_t addr;                      /**< IPv4 address */
//    int      packet;                    /**< packet type (UDP, TCP) */
//    int      protocol;                  /**< protocol to be used */
//} can_net_param_t;


#ifdef __cplusplus
}
#endif
#endif /* CANAPI_DEFINES_H_INCLUDED */
/** @}
 */
/*  ----------------------------------------------------------------------
 *  Uwe Vogt,  UV Software,  Chausseestrasse 33 A,  10115 Berlin,  Germany
 *  Tel.: +49-30-46799872,  Fax: +49-30-46799873,  Mobile: +49-170-3801903
 *  E-Mail: uwe.vogt@uv-software.de,  Homepage: http://www.uv-software.de/
 */
