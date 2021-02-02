/*
 *  CAN Interface API, Version 3 (Message Formatter)
 *
 *  Copyright (C) 2019-2020  Uwe Vogt, UV Software, Berlin (info@uv-software.com)
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
/** @file        can_msg.h
 *
 *  @brief       CAN Message Formatter
 *
 *  @author      $Author: haumea $
 *
 *  @version     $Rev: 980 $
 *
 *  @defgroup    can_msg CAN Message Formatter
 *  @{
 */
#ifndef CAN_MSG_H_INCLUDED
#define CAN_MSG_H_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

/*  -----------  includes  -----------------------------------------------
 */

#if (OPTION_CANAPI_COMPANIONS != 0)     // set it in the build environment!
#include "CANAPI_Types.h"               //   use CAN API V3 types and defines
#else                                   // otherwise:
#define CANBTR_STANDALONE_VARIANT       //   don't include CAN API V3 headers
#include <stdint.h>                     //   C99 header for sized integer types
#include <stdbool.h>                    //   C99 header for boolean type
#include <time.h>                       //   time types for time-stamp
#endif

/*  -----------  options  ------------------------------------------------
 */

/** @note  Set define OPTION_CANAPI_COMPANIONS to a non-zero value to compile
 *         this module in conjunction with the CAN API V3 sources (e.g. in
 *         the build environment).
 */
/** @note  Set define OPTION_CAN_2_0_ONLY to a non-zero value to compile
 *         with CAN 2.0 frame format only (e.g. in the build environment).
 */
#if (OPTION_CAN_2_0_ONLY != 0)
#warning Compilation with with legacy CAN 2.0 frame format!
#endif

/*  -----------  defines  ------------------------------------------------
 */

#ifdef CANBTR_STANDALONE_VARIANT
/** @name  CAN Identifier
 *  @brief CAN Identifier range
 *  @{ */
#define CAN_MAX_STD_ID           0x7FF  /**< highest 11-bit identifier */
#define CAN_MAX_XTD_ID      0x1FFFFFFF  /**< highest 29-bit identifier */
/** @} */

/** @name  CAN Data Length
 *  @brief CAN payload length and DLC definition
 *  @{ */
#define CAN_MAX_DLC                  8  /**< max. data lenth code (CAN 2.0) */
#define CAN_MAX_LEN                  8  /**< max. payload length (CAN 2.0) */
/** @} */

/** @name  CAN FD Data Length
 *  @brief CAN FD payload length and DLC definition
 *  @{ */
#define CANFD_MAX_DLC               15  /**< max. data lenth code (CAN FD) */
#define CANFD_MAX_LEN               64  /**< max. payload length (CAN FD) */
/** @} */

/** @name  Property Values
 *  @brief Values which can be used as property value (argument)
 *  @{ */
#define CANPARA_FORMAT_DEFAULT       0  /**< message formatter output (default) */
 /* - -  formatter option: ON or OFF - - - - - - - - - - - - - - - - - - */
#define CANPARA_OPTION_OFF           0  /**< formatter option: OFF (false, no, 0) */
#define CANPARA_OPTION_ON            1  /**< formatter option: ON (true, yes, !0) */
/* - -  number/data format: HEX, DEC, OCT, BIN  - - - - - - - - - - - - */
#define CANPARA_NUMBER_HEX          16  /**< mumerical format: HEXadecimal (base 16) */
#define CANPARA_NUMBER_DEC          10  /**< numerical format: DECimal (base 10) */
#define CANPARA_NUMBER_OCT           8  /**< numerical format: OCTal (base 8) */
/* - -  time-stamp reference: ZERO, ABS, REL  - - - - - - - - - - - - - */
#define CANPARA_TIMESTAMP_ZERO       0  /**< time-stamp reference: ZERO-based (first message) */
#define CANPARA_TIMESTAMP_ABS        1  /**< time-stamp reference: ABSolute time (local time) */
#define CANPARA_TIMESTAMP_REL        2  /**< time-stamp reference: RELative time */
/* - -  time format: TIME, SEC, DJD - - - - - - - - - - - - - - - - - - */
#define CANPARA_TIME_HHMMSS          0  /**< time-stamp format: <hours>:<min>:<sec>.<fraction> */
#define CANPARA_TIME_SEC             1  /**< time-stamp format: <sec>.<fraction> */
#define CANPARA_TIME_DJD             2  /**< time-stamp format: <days>.<fraction> */
/* - -  CAN FD length format: DLC or length - - - - - - - - - - - - - - */
#define CANPARA_CANFD_DLC            0  /**< message length as Data Length Code */
#define CANPARA_CANFD_LENGTH         1  /**< message length as LENGTH in bytes */
/* - -  field separator: spaces or tabs - - - - - - - - - - - - - - - - */
#define CANPARA_SEPARATOR_SPACES    32  /**< field separator: SPACES (ASCII = 32) */
#define CANPARA_SEPARATOR_TABS       9  /**< field separator: TABulatorS (ASCII = 9) */
/* - -  data field wraparound: no, 8, 10, 16, 32, 64  - - - - - - - - - */
#define CANPARA_WRAPAROUND_NO        0  /**< data field wraparound: NO wraparound */
#define CANPARA_WRAPAROUND_8         8  /**< data field wraparound after 8 bytes */
#define CANPARA_WRAPAROUND_10       10  /**< data field wraparound after 10 bytes */
#define CANPARA_WRAPAROUND_16       16  /**< data field wraparound after 16 bytes */
#define CANPARA_WRAPAROUND_32       32  /**< data field wraparound after 32 bytes */
#define CANPARA_WRAPAROUND_64       64  /**< data field wraparound after 64 bytes */
/** @} */
#define CANPROP_MAX_STRING_LENGTH 1024U /**< max. length of a formatted message */
#endif
#define MSG_STRING_LENGTH         CANPROP_MAX_STRING_LENGTH


/*  -----------  types  --------------------------------------------------
 */

/** @brief       CAN Message Format (output)
 */
typedef enum msg_format_t_ {
    MSG_FORMAT_DEFAULT = CANPARA_FORMAT_DEFAULT
} msg_format_t;

/** @brief       Formatter Option: ON or OFF
 */
typedef enum msg_fmt_option_t_ {
    MSG_FMT_OPTION_OFF = CANPARA_OPTION_OFF,
    MSG_FMT_OPTION_ON  = CANPARA_OPTION_ON
} msg_fmt_option_t;

/** @brief       Number/Data Format: HEX, DEC, OCT, BIN
 */
typedef enum msg_fmt_number_t_ {
    MSG_FMT_NUMBER_HEX = CANPARA_NUMBER_HEX,
    MSG_FMT_NUMBER_DEC = CANPARA_NUMBER_DEC,
    MSG_FMT_NUMBER_OCT = CANPARA_NUMBER_OCT
} msg_fmt_number_t;

/** @brief       Time-stamp Reference: ZERO, ABS, REL
 */
typedef enum msg_fmt_timestamp_t_ {
    MSG_FMT_TIMESTAMP_ZERO     = CANPARA_TIMESTAMP_ZERO,
    MSG_FMT_TIMESTAMP_ABSOLUTE = CANPARA_TIMESTAMP_ABS,
    MSG_FMT_TIMESTAMP_RELATIVE = CANPARA_TIMESTAMP_REL
} msg_fmt_timestamp_t;

/** @brief       Time Format: TIME, SEC, DJD
 */
typedef enum msg_fmt_time_t_ {
    MSG_FMT_TIME_HHMMSS = CANPARA_TIME_HHMMSS,
    MSG_FMT_TIME_SEC    = CANPARA_TIME_SEC,
    MSG_FMT_TIME_DJD    = CANPARA_TIME_DJD
} msg_fmt_time_t;

/** @brief       CAN FD Format: DLC or LENGTH
 */
typedef enum msg_fmt_canfd_t_ {
    MSG_FMT_CANFD_DLC    = CANPARA_CANFD_DLC,
    MSG_FMT_CANFD_LENGTH = CANPARA_CANFD_LENGTH
} msg_fmt_canfd_t;

/** @brief       Field Separator: Spaces or Tabs
 */
typedef enum msg_fmt_separator_t_ {
    MSG_FMT_SEPARATOR_SPACES = CANPARA_SEPARATOR_SPACES,
    MSG_FMT_SEPARATOR_TABS   = CANPARA_SEPARATOR_TABS
} msg_fmt_separator_t;

/** @brief       Data Field Wraparound: No, 8, 10, 16, 32, 64
 */
typedef enum msg_fmt_wraparound_t_ {
    MSG_FMT_WRAPAROUND_NO = CANPARA_WRAPAROUND_NO,
    MSG_FMT_WRAPAROUND_8  = CANPARA_WRAPAROUND_8,
    MSG_FMT_WRAPAROUND_10 = CANPARA_WRAPAROUND_10,
    MSG_FMT_WRAPAROUND_16 = CANPARA_WRAPAROUND_16,
    MSG_FMT_WRAPAROUND_32 = CANPARA_WRAPAROUND_32,
    MSG_FMT_WRAPAROUND_64 = CANPARA_WRAPAROUND_64
} msg_fmt_wraparound_t;

/** @brief       CAN Time-stamp:
 */
#ifdef CANMSG_STANDALONE
typedef struct timespec msg_timestamp_t;  /* w/ nanoseconds resolution */
#else
typedef can_timestamp_t msg_timestamp_t;  /* CAN API V3 time-stamp */
#endif
/** @brief       CAN Message (with Time-stamp):
 */
#ifdef CANMSG_STANDALONE
typedef struct msg_message_t_ {
    uint32_t id;                        /**< CAN identifier */
    struct {
        uint8_t xtd : 1;                /**< flag: extended format */
        uint8_t rtr : 1;                /**< flag: remote frame */
#if (OPTION_CAN_2_0_ONLY == 0)
        uint8_t fdf : 1;                /**< flag: CAN FD format */
        uint8_t brs : 1;                /**< flag: bit-rate switching */
        uint8_t esi : 1;                /**< flag: error state indicator */
        uint8_t : 2;
#else
        uint8_t : 5;
#endif
        uint8_t sts : 1;                /**< flag: status message */
    };
#if (OPTION_CAN_2_0_ONLY == 0)
    uint8_t dlc;                        /**< data length code (0 .. 15) */
    uint8_t data[CANFD_MAX_LEN];        /**< payload (CAN FD:  0 .. 64) */
#else
    uint8_t dlc;                        /**< data length code (0 .. 8) */
    uint8_t data[CAN_MAX_LEN];          /**< payload (CAN 2.0: 0 .. 8) */
#endif
    msg_timestamp_t timestamp;          /**< time-stamp { sec, usec } */
} msg_message_t;
#else
typedef can_message_t msg_message_t;    /* CAN API V3 message */
#endif
/** @brief       CAN Message Counter (64-bit):
 */
typedef uint64_t msg_counter_t;         /**< 64-bit message counter */

/** @brief       CAN Message Channel (optional):
 */
typedef int32_t msg_channel_t;          /**< message source (channel) */

/** @brief       CAN Message Direction (RX or TX)
 */
typedef enum msg_direction_t_ {
    MSG_RX_MESSAGE = 0,
    MSG_TX_MESSAGE = 1
} msg_direction_t;


/*  -----------  variables  ----------------------------------------------
 */


/*  -----------  prototypes  ---------------------------------------------
 */

/** @brief       ...
 *
 *  @param[in]   message - ...
 *
 *  @returns     pointer to a zero-terminated string.
 */
char *msg_format_message(const msg_message_t *message, msg_direction_t direction,
                               msg_counter_t counter, msg_channel_t channel);

/** @brief       ...
 *
 *  @param[in]   message - ...
 *
 *  @returns     pointer to a zero-terminated string.
 */
char *msg_format_time(const msg_message_t *message);

/** @brief       ...
 *
 *  @param[in]   message - ...
 *
 *  @returns     pointer to a zero-terminated string.
 */
char *msg_format_id(const msg_message_t *message);

/** @brief       ...
 *
 *  @param[in]   message - ...
 *
 *  @returns     pointer to a zero-terminated string.
 */
char *msg_format_flags(const msg_message_t *message);

/** @brief       ...
 *
 *  @param[in]   message - ...
 *
 *  @returns     pointer to a zero-terminated string.
 */
char *msg_format_dlc(const msg_message_t *message);

/** @brief       ...
 *
 *  @param[in]   message - ...
 *
 *  @returns     pointer to a zero-terminated string.
 */
char *msg_format_data(const msg_message_t *message);

/** @brief       ...
 *
 *  @param[in]   message - ...
 *
 *  @returns     pointer to a zero-terminated string.
 */
char *msg_format_ascii(const msg_message_t *message);

/** @brief       set message output format {DEFAULT, ...}.
 *
 *  @param[in]   format - ...
 *
 *  @returns     non-zero value on success, otherwise 0.
 */
int msg_set_format(msg_format_t format);

/** @brief       set formatter option: time-stamp {ZERO, ABS, REL}.
 *
 *  @param[in]   option - ...
 *
 *  @returns     non-zero value on success, otherwise 0.
 */
int msg_set_fmt_time_stamp(msg_fmt_timestamp_t option);

/** @brief       set formatter option: time-stamp in usec {OFF, ON}.
 *
 *  @param[in]   option - ...
 *
 *  @returns     non-zero value on success, otherwise 0.
 */
int msg_set_fmt_time_usec(msg_fmt_option_t option);

/** @brief       set formatter option: time format {TIME, SEC, DJD}.
 *
 *  @param[in]   option - ...
 *
 *  @returns     non-zero value on success, otherwise 0.
 */
int msg_set_fmt_time_format(msg_fmt_time_t option);

/** @brief       set formatter option: identifier {HEX, DEC, OCT, BIN}.
 *
 *  @param[in]   option - ...
 *
 *  @returns     non-zero value on success, otherwise 0.
 */
int msg_set_fmt_id(msg_fmt_number_t option);

/** @brief       set formatter option: extended identifier {OFF, ON}.
 *
 *  @param[in]   option - ...
 *
 *  @returns     non-zero value on success, otherwise 0.
 */
int msg_set_fmt_id_xtd(msg_fmt_option_t option);

/** @brief       set formatter option: DLC/length {HEX, DEC, OCT, BIN}.
 *
 *  @param[in]   option - ...
 *
 *  @returns     non-zero value on success, otherwise 0.
 */
int msg_set_fmt_dlc(msg_fmt_number_t option);

/** @brief       set formatter option: CAN FD format {DLC, LENGTH}.
 *
 *  @param[in]   option - ...
 *
 *  @returns     non-zero value on success, otherwise 0.
 */
int msg_set_fmt_dlc_format(msg_fmt_canfd_t option);

/** @brief       set formatter option: DLC in brackets {'\0', '(', '['}.
 *
 *  @param[in]   option - ...
 *
 *  @returns     non-zero value on success, otherwise 0.
 */
int msg_set_fmt_dlc_brackets(int option);

/** @brief       set formatter option: message flags {ON, OFF}.
 *
 *  @param[in]   option - ...
 *
 *  @returns     non-zero value on success, otherwise 0.
 */
int msg_set_fmt_flags(msg_fmt_option_t option);

/** @brief       set formatter option: message data {HEX, DEC, OCT, BIN}.
 *
 *  @param[in]   option - ...
 *
 *  @returns     non-zero value on success, otherwise 0.
 */
int msg_set_fmt_data(msg_fmt_number_t option);

/** @brief       set formatter option: data as ASCII {ON, OFF}.
 *
 *  @param[in]   option - ...
 *
 *  @returns     non-zero value on success, otherwise 0.
 */
int msg_set_fmt_ascii(msg_fmt_option_t option);

/** @brief       set formatter option: substitute for non-printable characters.
 *
 *  @param[in]   option - ...
 *
 *  @returns     non-zero value on success, otherwise 0.
 */
int msg_set_fmt_ascii_subst(int option);

/** @brief       set formatter option: message source {OFF, ON}.
 *
 *  @param[in]   option - ...
 *
 *  @returns     non-zero value on success, otherwise 0.
 */
int msg_set_fmt_channel(msg_fmt_option_t option);

/** @brief       set formatter option: message counter {ON, OFF}.
 *
 *  @param[in]   option - ...
 *
 *  @returns     non-zero value on success, otherwise 0.
 */
int msg_set_fmt_counter(msg_fmt_option_t option);

/** @brief       set formatter option: separator {SPACES, TABS}.
 *
 *  @param[in]   option - ...
 *
 *  @returns     non-zero value on success, otherwise 0.
 */
int msg_set_fmt_separator(msg_fmt_separator_t option);

/** @brief       set formatter option: wraparound {NO, 8, 16, 32, 64}.
 *
 *  @param[in]   option - ...
 *
 *  @returns     non-zero value on success, otherwise 0.
 */
int msg_set_fmt_wraparound(msg_fmt_wraparound_t option);

/** @brief       set formatter option: end-of-line character {OFF, ON}.
 *
 *  @param[in]   option - ...
 *
 *  @returns     non-zero value on success, otherwise 0.
 */
int msg_set_fmt_eol(msg_fmt_option_t option);

/** @brief       set formatter option: prompt for received messages (char[6+1]).
 *
 *  @param[in]   option - ...
 *
 *  @returns     non-zero value on success, otherwise 0.
 */
int msg_set_fmt_rx_prompt(const char *option);

/** @brief       set formatter option: prompt for sent messages (char[6+1]).
 *
 *  @param[in]   option - ...
 *
 *  @returns     non-zero value on success, otherwise 0.
 */
int msg_set_fmt_tx_prompt(const char *option);


#ifdef __cplusplus
}
#endif
#endif /* CAN_MSG_H_INCLUDED */
/** @}
 */
/*  ----------------------------------------------------------------------
 *  Uwe Vogt,  UV Software,  Chausseestrasse 33 A,  10115 Berlin,  Germany
 *  Tel.: +49-30-46799872,  Fax: +49-30-46799873,  Mobile: +49-170-3801903
 *  E-Mail: uwe.vogt@uv-software.de,  Homepage: http://www.uv-software.de/
 */
