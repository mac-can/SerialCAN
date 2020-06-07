/*
 *  CAN Interface API, Version 3 (CAN-over-Serial-Line)
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
 *  @brief       CAN API V3 for generic CAN Interfaces - CAN-over-Serial-Line
 *
 *  @author      $Author: eris $
 *
 *  @version     $Rev: 907 $
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


/*  -----------  defines  ------------------------------------------------
 */

/** @name  Protocol option flags
 *  @brief SerialCAN protocol options
 *  @{ */
#define CANSIO_SLCAN            0x00U   /**< Lawicel SLCAN protocol */
/** @} */

/** @name  Data size option
 *  @brief Number fo data bits (5, 6, 7, 8)
 *  @{ */
#define CANSIO_5DATABITS        5U      /**< 5 bits per data byte */
#define CANSIO_6DATABITS        6U      /**< 6 bits per data byte */
#define CANSIO_7DATABITS        7U      /**< 7 bits per data byte */
#define CANSIO_8DATABITS        8U      /**< 8 bits per data byte */
/** @} */

/** @name  Parity option
 *  @brief Parity bit (None, Even, Odd)
 *  @{ */
#define CANSIO_NOPARITY         0U      /**< no parity */
#define CANSIO_ODDPARITY        1U      /**< odd parity */
#define CANSIO_EVENPARITY       2U      /**< even parity */
/** @} */

/** @name  Stop bit option
 *  @brief Number of stop bits (1 or 2)
 *  @{ */
#define CANSIO_1STOPBIT         1U      /**< 1 stop bit */
#define CANSIO_2STOPBITS        2U      /**< 2 stop bits */
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
    uint8_t  options;                   /**<  protocol options */
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
 *  E-Mail: uwe.vogt@uv-software.de,  Homepage: http://www.uv-software.de/
 */
