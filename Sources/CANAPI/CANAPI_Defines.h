/*
 *  CAN Interface API, Version 3 (Definitions and Options)
 *
 *  Copyright (C) 2004-2020  Uwe Vogt, UV Software, Berlin (info@uv-software.com)
 *
 *  This file is part of CAN API V3.
 *
 *  CAN API V3 is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  CAN API V3 is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with CAN API V3.  If not, see <http://www.gnu.org/licenses/>.
 */
/** @file        CANAPI_Defines.h
 *
 *  @brief       CAN API V3 for generic CAN Interfaces - Definitions and Options
 *
 *  @author      $Author: haumea $
 *
 *  @version     $Rev: 913 $
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
#define CANLIB_PEAK_PCAN        200     /**< PEAK PCAN-Light interfaces */
#define CANLIB_VECTOR_XL        300     /**< Vector XL-Driver library */
#define CANLIB_PCANBASIC        400     /**< PEAK PCAN-Basic interfaces */
#define CANLIB_RUSOKU_LT        500     /**< Rusuko TouCAN interfaces */
#define CANLIB_KVASER_32        600     /**< Kvaser CANLIB (canlib32) */
#define CANLIB_ROCKETCAN        700     /**< CAN-over-IP (RocketCAN) */
#define CANLIB_SERIALCAN        900     /**< Serial-Line (SerialCAN) */
#define CANLIB_SOCKETCAN        1000    /**< Linux CAN (SocketCAN) */
/** @} */

/** @name  Library Names
 *  @brief Filenames of the CAN API libraries (depending on platform)
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
 #define CANDLL_PCANBasic      "libUVCANPCB.dylib"
 #define CANDLL_RUSOKU_LT      "libUVCANTOU.dylib"
 #define CANDLL_SERIALCAN      "libUVCANSLC.dylib"
#else
 #ifdef _WIN64
 #define CANAPI_PLATFORM       "x64"
 #else
 #define CANAPI_PLATFORM       "x86"
 #endif
 #define CANDLL_CANAPILIB      "uvcan300.dll"
 #define CANDLL_PCANBasic      "u3canpcb.dll"
 #define CANDLL_KVASER_32      "u3cankvl.dll"
 #define CANDLL_RUSOKU_LT      "u3cantou.dll"
 #define CANDLL_SERIALCAN      "u3canslc.dll"

 #define CAN200_CANAPILIB      "uvcan200.dll"
 #define CAN200_IXXAT_VCI      "uvcanvci.dll"
 #define CAN200_IXXAT_CAC      "uvcancac.dll"
 #define CAN200_PEAK_PCAN      "uvcanpcl.dll"
 #define CAN200_VECTOR_XL      "uvcanvxl.dll"
 #define CAN200_PCANBasic      "uvcanpcb.dll"
 #define CAN200_UVS_TCPIP      "uvcantcp.dll"
 #define CAN200_UCAN_VCP       "uvcanvcp.dll"

 #define CAN100_CANAPILIB      "uvcan100.dll"
 #define CAN100_VECTOR_XL      "u1canvxl.dll"
 #define CAN100_PCANBasic      "u1canpcb.dll"
 #define CAN100_ROCKETCAN      "u1canroc.dll"
 #define CAN100_SERIALCAN      "u1canslc.dll"
#endif
/** @} */

#define CANDEV_NETWORK        (-1)      /**< channel ID for network device */
#define CANDEV_SERIAL         (-1)      /**< channel ID for serial port device */

#define SYSERR_OFFSET         (-10000)  /**< offset for system errors */


/*  -----------  types  --------------------------------------------------
 */

/** @brief Linux-CAN (a.k.a. SocketCAN) parameters
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
