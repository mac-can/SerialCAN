/*
 *  CAN Interface API, Version 3 (Data Types and Defines)
 *
 *  Copyright (C) 2004-2021  Uwe Vogt, UV Software, Berlin (info@uv-software.com)
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
/** @file        CANAPI_Types.h
 *
 *  @brief       CAN API V3 for generic CAN Interfaces - Data Types and Defines
 *
 *  @author      $Author: eris $
 *
 *  @version     $Rev: 918 $
 *
 *  @addtogroup  can_api
 *  @{
 */
#ifndef CANAPI_TYPES_H_INCLUDED
#define CANAPI_TYPES_H_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

/*  -----------  includes  -----------------------------------------------
 */

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <time.h>


/*  -----------  options  ------------------------------------------------
 */

/** @note  Set define OPTION_CANAPI_LIBRARY to a non-zero value to compile
 *         the master loader library (e.g. in the build environment). Or optionally
 *         set define OPTION_CANAPI_DRIVER to a non-zero value to compile
 *         a driver library.
 */
/** @note  Set define OPTION_CAN_2_0_ONLY to a non-zero value to compile
 *         with CAN 2.0 frame format only (e.g. in the build environment).
 */

/*  -----------  defines  ------------------------------------------------
 */

/** @name  CAN API Revision Number
 *  @brief Revision number of the CAN API wrapper specification
 *  @{ */
#define CAN_API_V3_SPEC          0x300  /**< CAN API specification V3 */
#define CAN_API_V2_SPEC          0x200  /**< CAN API specification V2 */
#define CAN_API_V1_SPEC          0x100  /**< CAN API specification V1 */
#define CAN_API_SPEC  (CAN_API_V3_SPEC) /**< CAN API revision number! */
/** @} */

/** @name  CAN Identifier
 *  @brief CAN Identifier range
 *  @{ */
#define CAN_MAX_STD_ID           0x7FF  /**< highest 11-bit identifier */
#define CAN_MAX_XTD_ID      0x1FFFFFFF  /**< highest 29-bit identifier */
/** @} */

/** @name  CAN Data Length
 *  @brief CAN payload length and DLC definition
 *  @{ */
#define CAN_MAX_DLC                  8  /**< max. data length code (CAN 2.0) */
#define CAN_MAX_LEN                  8  /**< max. payload length (CAN 2.0) */
/** @} */

/** @name  CAN FD Data Length
 *  @brief CAN FD payload length and DLC definition
 *  @{ */
#define CANFD_MAX_DLC               15  /**< max. data length code (CAN FD) */
#define CANFD_MAX_LEN               64  /**< max. payload length (CAN FD) */
/** @} */

/** @name  CAN Baud Rate Indexes (for compatibility)
 *  @brief CAN baud rate indexes defined by CiA (for CANopen)
 *  @note  They must be passed with a minus sign to can_start()
 *  @{ */
#define CANBDR_1000                  0  /**< baud rate: 1000 kbit/s */
#define CANBDR_800                   1  /**< baud rate:  800 kbit/s */
#define CANBDR_500                   2  /**< baud rate:  500 kbit/s */
#define CANBDR_250                   3  /**< baud rate:  250 kbit/s */
#define CANBDR_125                   4  /**< baud rate:  125 kbit/s */
#define CANBDR_100                   5  /**< baud rate:  100 kbit/s */
#define CANBDR_50                    6  /**< baud rate:   50 kbit/s */
#define CANBDR_20                    7  /**< baud rate:   20 kbit/s */
#define CANBDR_10                    8  /**< baud rate:   10 kbit/s */
/** @} */

/** @name  CAN 2.0 Predefined Bit-rates
 *  @brief Indexes to predefined bit-rates (CAN 2.0 only!)
 *  @{ */
#define CANBTR_INDEX_1M             (0) /**< bit-rate: 1000 kbit/s */
#define CANBTR_INDEX_800K          (-1) /**< bit-rate:  800 kbit/s */
#define CANBTR_INDEX_500K          (-2) /**< bit-rate:  500 kbit/s */
#define CANBTR_INDEX_250K          (-3) /**< bit-rate:  250 kbit/s */
#define CANBTR_INDEX_125K          (-4) /**< bit-rate:  125 kbit/s */
#define CANBTR_INDEX_100K          (-5) /**< bit-rate:  100 kbit/s */
#define CANBTR_INDEX_50K           (-6) /**< bit-rate:   50 kbit/s */
#define CANBTR_INDEX_20K           (-7) /**< bit-rate:   20 kbit/s */
#define CANBTR_INDEX_10K           (-8) /**< bit-rate:   10 kbit/s */
/** @} */

/** @name  CAN Controller Frequencies
 *  @brief Frequencies for calculation of the bit-rate
 *  @note  Usable frequencies depend on the microcontroller used
 *  @{ */
#define CANBTR_FREQ_80MHz     80000000  /**< frequency: 80 MHz */
#define CANBTR_FREQ_60MHz     60000000  /**< frequency: 60 MHz */
#define CANBTR_FREQ_40MHz     40000000  /**< frequency: 40 MHz */
#define CANBTR_FREQ_30MHz     30000000  /**< frequency: 30 MHz */
#define CANBTR_FREQ_24MHz     24000000  /**< frequency: 24 MHz */
#define CANBTR_FREQ_20MHz     20000000  /**< frequency: 20 MHz */
#define CANBTR_FREQ_SJA1000    8000000  /**< frequency:  8 MHz */
/** @} */

/** @name  CAN 2.0 and CAN FD Nominal Bit-rate Settings
 *  @brief Limits for nominal bit-rate settings
 *  @{ */
#define CANBTR_NOMINAL_BRP_MIN      1U  /**< min. bit-timing prescaler */
#define CANBTR_NOMINAL_BRP_MAX   1024U  /**< max. bit-timing prescaler */
#define CANBTR_NOMINAL_TSEG1_MIN    1U  /**< min. time segment 1 (before SP) */
#define CANBTR_NOMINAL_TSEG1_MAX  256U  /**< max. time segment 1 (before SP) */
#define CANBTR_NOMINAL_TSEG2_MIN    1U  /**< min. time segment 2 (after SP) */
#define CANBTR_NOMINAL_TSEG2_MAX  128U  /**< max. time segment 2 (after SP) */
#define CANBTR_NOMINAL_SJW_MIN      1U  /**< min. syncronization jump width */
#define CANBTR_NOMINAL_SJW_MAX    128U  /**< max. syncronization jump width */
/** @} */

/** @name  CAN FD Data Bit-rate Settings
 *  @brief Limits for data bit-rate settings
 *  @{ */
#define CANBTR_DATA_BRP_MIN         1U  /**< min. baud rate prescaler */
#define CANBTR_DATA_BRP_MAX      1024U  /**< max. baud rate prescaler */
#define CANBTR_DATA_TSEG1_MIN       1U  /**< min. time segment 1 (before SP) */
#define CANBTR_DATA_TSEG1_MAX      32U  /**< max. time segment 1 (before SP) */
#define CANBTR_DATA_TSEG2_MIN       1U  /**< min. time segment 2 (after SP) */
#define CANBTR_DATA_TSEG2_MAX      16U  /**< max. time segment 2 (after SP) */
#define CANBTR_DATA_SJW_MIN         1U  /**< min. syncronization jump width */
#define CANBTR_DATA_SJW_MAX        16U  /**< max. syncronization jump width */
/** @} */

/** @name  SJA1000 Bit-rate Settings (CAN 2.0 only)
 *  @brief Limits for bit-rate settings of the SJA1000 CAN controller
 *  @{ */
#define CANBTR_SJA1000_BRP_MIN      1U  /**< min. baud rate prescaler */
#define CANBTR_SJA1000_BRP_MAX     64U  /**< max. baud rate prescaler */
#define CANBTR_SJA1000_TSEG1_MIN    1U  /**< min. time segment 1 (before SP) */
#define CANBTR_SJA1000_TSEG1_MAX   16U  /**< max. time segment 1 (before SP) */
#define CANBTR_SJA1000_TSEG2_MIN    1U  /**< min. time segment 2 (after SP) */
#define CANBTR_SJA1000_TSEG2_MAX    8U  /**< max. time segment 2 (after SP) */
#define CANBTR_SJA1000_SJW_MIN      1U  /**< min. syncronization jump width */
#define CANBTR_SJA1000_SJW_MAX      4U  /**< max. syncronization jump width */
#define CANBTR_SJA1000_SAM_MIN      0U  /**< single: the bus is sampled once */
#define CANBTR_SJA1000_SAM_MAX      1U  /**< triple: the bus is sampled three times */
/** @} */

/** @name  CAN Mode Flags
 *  @brief Flags to control the operation mode
 *  @{ */
#define CANMODE_FDOE              0x80U /**< CAN FD operation enable/disable */
#define CANMODE_BRSE              0x40U /**< bit-rate switch enable/disable */
#define CANMODE_NISO              0x20U /**< Non-ISO CAN FD enable/disable */
#define CANMODE_SHRD              0x10U /**< shared access enable/disable */
#define CANMODE_NXTD              0x08U /**< extended format disable/enable */
#define CANMODE_NRTR              0x04U /**< remote frames disable/enable */
#define CANMODE_ERR               0x02U /**< error frames enable/disable */
#define CANMODE_MON               0x01U /**< monitor mode enable/disable */
#define CANMODE_DEFAULT           0x00U /**< CAN 2.0 operation mode */
/** @} */

/** @name  CAN Error Codes
 *  @brief General CAN error codes (negative)
 *  @note  Codes less or equal than -100 are for vendor-specific error codes
 *         and codes less or equal than -10000 are for OS-specific error codes
 *         (add 10000 to get the reported OS error code, e.g. errno).
 *  @{ */
#define CANERR_NOERROR              (0) /**< no error! */
#define CANERR_BOFF                (-1) /**< CAN - busoff status */
#define CANERR_EWRN                (-2) /**< CAN - error warning status */
#define CANERR_BERR                (-3) /**< CAN - bus error */
#define CANERR_OFFLINE             (-9) /**< CAN - not started */
#define CANERR_ONLINE              (-8) /**< CAN - already started */
#define CANERR_MSG_LST            (-10) /**< CAN - message lost */
#define CANERR_LEC_STUFF          (-11) /**< LEC - stuff error */
#define CANERR_LEC_FORM           (-12) /**< LEC - form error */
#define CANERR_LEC_ACK            (-13) /**< LEC - acknowledge error */
#define CANERR_LEC_BIT1           (-14) /**< LEC - recessive bit error */
#define CANERR_LEC_BIT0           (-15) /**< LEC - dominant bit error */
#define CANERR_LEC_CRC            (-16) /**< LEC - checksum error */
#define CANERR_TX_BUSY            (-20) /**< USR - transmitter busy */
#define CANERR_RX_EMPTY           (-30) /**< USR - receiver empty */
#define CANERR_ERR_FRAME          (-40) /**< USR - error frame */
#define CANERR_TIMEOUT            (-50) /**< USR - time-out */
#define CANERR_RESOURCE           (-90) /**< USR - resource allocation */
#define CANERR_BAUDRATE           (-91) /**< USR - illegal baudrate */
#define CANERR_HANDLE             (-92) /**< USR - illegal handle */
#define CANERR_ILLPARA            (-93) /**< USR - illegal parameter */
#define CANERR_NULLPTR            (-94) /**< USR - null-pointer assignment */
#define CANERR_NOTINIT            (-95) /**< USR - not initialized */
#define CANERR_YETINIT            (-96) /**< USR - already initialized */
#define CANERR_LIBRARY            (-97) /**< USR - illegal library */
#define CANERR_NOTSUPP            (-98) /**< USR - not supported */
#define CANERR_FATAL              (-99) /**< USR - other errors */
#define CANERR_VENDOR            (-100) /**< USR - vendor specific */
/** @} */

/** @name  CAN Status Codes
 *  @brief CAN status from CAN controller
 *  @{ */
#define CANSTAT_RESET             0x80U /**< CAN status: controller stopped */
#define CANSTAT_BOFF              0x40U /**< CAN status: busoff status */
#define CANSTAT_EWRN              0x20U /**< CAN status: error warning level */
#define CANSTAT_BERR              0x10U /**< CAN status: bus error (LEC) */
#define CANSTAT_TX_BUSY           0x08U /**< CAN status: transmitter busy */
#define CANSTAT_RX_EMPTY          0x04U /**< CAN status: receiver empty */
#define CANSTAT_MSG_LST           0x02U /**< CAN status: message lost */
#define CANSTAT_QUE_OVR           0x01U /**< CAN status: event-queue overrun */
/** @} */

/** @name  Board Test Codes
 *  @brief Results of the board test
 *  @{ */
#define CANBRD_NOT_PRESENT         (-1) /**< CAN board not present */
#define CANBRD_PRESENT              (0) /**< CAN board present */
#define CANBRD_OCCUPIED            (+1) /**< CAN board present, but occupied */
#define CANBRD_NOT_TESTABLE        (-2) /**< CAN board not testable (e.g. legacy API) */
/** @} */

/** @name  Blocking Read
 *  @brief Control of blocking read
 *  @{ */
#define CANREAD_INFINITE         65535U /**< infinite time-out (blocking read) */
#define CANKILL_ALL                (-1) /**< to signal all waiting event objects */
/** @} */

/** @name  Property IDs
 *  @brief Properties that can be read or written
 *  @{ */
#define CANPROP_GET_SPEC             0U /**< version of the wrapper specification (uint16_t) */
#define CANPROP_GET_VERSION          1U /**< version number of the library (uint16_t) */
#define CANPROP_GET_PATCH_NO         2U /**< patch number of the library (uint8_t) */
#define CANPROP_GET_BUILD_NO         3U /**< build number of the library (uint32_t) */
#define CANPROP_GET_LIBRARY_ID       4U /**< library id of the library (int32_t) */
#define CANPROP_GET_LIBRARY_VENDOR   5U /**< vendor name of the library (char[256]) */
#define CANPROP_GET_LIBRARY_DLLNAME  6U /**< file name of the library DLL (char[256]) */
#define CANPROP_GET_DEVICE_TYPE     10U /**< device type of the CAN interface (int32_t) */
#define CANPROP_GET_DEVICE_NAME     11U /**< device name of the CAN interface (char[256]) */
#define CANPROP_GET_DEVICE_VENDOR   12U /**< vendor name of the CAN interface (char[256]) */
#define CANPROP_GET_DEVICE_DLLNAME  13U /**< file name of the CAN interface DLL(char[256]) */
#define CANPROP_GET_DEVICE_PARAM    14U /**< device parameter of the CAN interface (char[256]) */
#define CANPROP_GET_OP_CAPABILITY   15U /**< supported operation modes of the CAN controller (uint8_t) */
#define CANPROP_GET_OP_MODE         16U /**< active operation mode of the CAN controller (uint8_t) */
#define CANPROP_GET_BITRATE         17U /**< active bit-rate of the CAN controller (can_bitrate_t) */
#define CANPROP_GET_SPEED           18U /**< active bus speed of the CAN controller (can_speed_t) */
#define CANPROP_GET_STATUS          19U /**< current status register of the CAN controller (uint8_t) */
#define CANPROP_GET_BUSLOAD         20U /**< current bus load of the CAN controller (uint8_t) */
#define CANPROP_GET_TX_COUNTER      24U /**< total number of sent messages (uint64_t) */
#define CANPROP_GET_RX_COUNTER      25U /**< total number of reveiced messages (uint64_t) */
#define CANPROP_GET_ERR_COUNTER     26U /**< total number of reveiced error frames (uint64_t) */
#define CANPROP_GET_FLT_11BIT_CODE  32U /**< accecptance filter code of 11-bit identifier (int32_t) */
#define CANPROP_GET_FLT_11BIT_MASK  33U /**< accecptance filter mask of 11-bit identifier (int32_t) */
#define CANPROP_GET_FLT_29BIT_CODE  34U /**< accecptance filter code of 29-bit identifier (int32_t) */
#define CANPROP_GET_FLT_29BIT_MASK  35U /**< accecptance filter mask of 29-bit identifier (int32_t) */
#define CANPROP_SET_FLT_11BIT_CODE  36U /**< set value for accecptance filter code of 11-bit identifier (int32_t) */
#define CANPROP_SET_FLT_11BIT_MASK  37U /**< set value for accecptance filter mask of 11-bit identifier (int32_t) */
#define CANPROP_SET_FLT_29BIT_CODE  38U /**< set value for accecptance filter code of 29-bit identifier (int32_t) */
#define CANPROP_SET_FLT_29BIT_MASK  39U /**< set value for accecptance filter mask of 29-bit identifier (int32_t) */
#if (OPTION_CANAPI_LIBRARY != 0)
/* - -  build-in bit-rate conversion  - - - - - - - - - - - - - - - - - */
#define CANPROP_GET_BTR_INDEX       64U /**< bit-rate as CiA index (int32_t) */
#define CANPROP_GET_BTR_VALUE       65U /**< bit-rate as struct (can_bitrate_t) */
#define CANPROP_GET_BTR_SPEED       66U /**< bit-rate as bus speed (can_speed_t) */
#define CANPROP_GET_BTR_STRING      67U /**< bit-rate as string (char[256]) */
#define CANPROP_GET_BTR_SJA1000     68U /**< bit-rate as SJA1000 register (uint16_t) */
#define CANPROP_SET_BTR_INDEX       72U /**< set value for conversion form CiA index (int32_t) */
#define CANPROP_SET_BTR_VALUE       73U /**< set value for conversion form struct (can_bitrate_t) */
#define CANPROP_SET_BTR_SPEED       74U /**< set value for conversion form bus speed (can_speed_t) */
#define CANPROP_SET_BTR_STRING      75U /**< set value for conversion form string (char[256]) */
#define CANPROP_SET_BTR_SJA1000     76U /**< set value for conversion form SJA1000 register (uint16_t) */
/* - -  build-in message formatter  - - - - - - - - - - - - - - - - - - */
#define CANPROP_GET_MSG_STRING     128U /**< last received or sent message as formatted string (char[1024]) */
#define CANPROP_GET_MSG_TIME       129U /**< time-stamp of last received or sent message (char[256]) */
#define CANPROP_GET_MSG_ID         130U /**< identifier of last received or sent message (char[256]) */
#define CANPROP_GET_MSG_DLC        131U /**< DLC/length of last received or sent message (char[256]) */
#define CANPROP_GET_MSG_FLAGS      132U /**< flags of last received or sent message (char[256]) */
#define CANPROP_GET_MSG_DATA       133U /**< data of last received or sent message (char[256]) */
#define CANPROP_GET_MSG_ASCII      134U /**< data as ASCII of last received or sent message (char[256]) */
#define CANPROP_SET_MSG_FORMAT     144U /**< set message output format {DEFAULT, ...} (int) */
#define CANPROP_SET_FMT_TIMESTAMP  145U /**< set formatter option: time-stamp {ZERO, ABS, REL} (int) */
#define CANPROP_SET_FMT_TIEMUSEC   146U /**< set formatter option: time-stamp in usec {OFF, ON} (int) */
#define CANPROP_SET_FMT_TIME       147U /**< set formatter option: time format {TIME, SEC, DJD} (int) */
#define CANPROP_SET_FMT_ID         148U /**< set formatter option: identifier {HEX, DEC, OCT} (int) */
#define CANPROP_SET_FMT_XTD        149U /**< set formatter option: extended identifier {OFF, ON} (int) */
#define CANPROP_SET_FMT_DLC        150U /**< set formatter option: DLC/length {DEC, OCT, HEX} (int) */
#define CANPROP_SET_FMT_LENGTH     151U /**< set formatter option: CAN FD format {DLC, LENGTH} (int) */
#define CANPROP_SET_FMT_BRACKETS   152U /**< set formatter option: DLC in brackets {'\0', '(', '['} (int) */
#define CANPROP_SET_FMT_FLAGS      153U /**< set formatter option: message flags {ON, OFF} (int) */
#define CANPROP_SET_FMT_DATA       154U /**< set formatter option: message data {HEX, DEC, OCT} (int) */
#define CANPROP_SET_FMT_ASCII      155U /**< set formatter option: data as ASCII {ON, OFF} (int) */
#define CANPROP_SET_FMT_SUBSTITUTE 156U /**< set formatter option: substitute for non-printables (int) */
#define CANPROP_SET_FMT_CHANNEL    157U /**< set formatter option: message source {OFF, ON} (int) */
#define CANPROP_SET_FMT_COUNTER    158U /**< set formatter option: message counter {ON, OFF} (int) */
#define CANPROP_SET_FMT_SEPARATOR  159U /**< set formatter option: separator {SPACES, TABS} (int) */
#define CANPROP_SET_FMT_WRAPAROUND 160U /**< set formatter option: wraparound {NO, 8, 10, 16, 32, 64} (int) */
#define CANPROP_SET_FMT_EOL_CHAR   161U /**< set formatter option: end-of-line character {OFF, ON} (int) */
#define CANPROP_SET_FMT_RX_PROMPT  162U /**< set formatter option: prompt for received messages (char[6+1]) */
#define CANPROP_SET_FMT_TX_PROMPT  163U /**< set formatter option: prompt for sent messages (char[6+1]) */
#endif
/* - -  access to vendor-specific propUrties  - - - - - - - - - - - - - */
#define CANPROP_GET_VENDOR_PROP    256U /**< get a vendor-specific property value (void*) */
#define CANPROP_SET_VENDOR_PROP    512U /**< set a vendor-specific property value (void*) */
#define CANPROP_VENDOR_PROP_RANGE  256U /**< range for vendor-specific property values */
#define CANPROP_MAX_BUFFER_SIZE    256U /**< max. buffer size for property values */
#define CANPROP_MAX_STRING_LENGTH 1024U /**< max. length of a formatted message */
/** @} */

/** @name  Property Values
 *  @brief Values which can be used as property value (argument)
 *  @{ */
#define CANPARA_FORMAT_DEFAULT       0  /**< message formatter output (default) */
/* - -  formatter option: ON or OFF - - - - - - - - - - - - - - - - - - */
#define CANPARA_OPTION_OFF           0  /**< formatter option: OFF (false, no, 0) */
#define CANPARA_OPTION_ON            1  /**< formatter option: ON (true, yes, !0) */
/* - -  number/data format: HEX, DEC, OCT, BIN  - - - - - - - - - - - - */
#define CANPARA_NUMBER_HEX          16  /**< numerical format: HEXadecimal (base 16) */
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

/*  -----------  types  --------------------------------------------------
 */

/** @brief       CAN Status-register:
 */
typedef union can_status_t_ {
    uint8_t byte;                       /**< byte access */
    struct {                            /*   bit access: */
        uint8_t queue_overrun : 1;      /**<   event-queue overrun */
        uint8_t message_lost : 1;       /**<   message lost */
        uint8_t receiver_empty : 1;     /**<   receiver empty */
        uint8_t transmitter_busy : 1;   /**<   transmitter busy */
        uint8_t bus_error : 1;          /**<   bus error (LEC) */
        uint8_t warning_level : 1;      /**<   error warning status */
        uint8_t bus_off : 1;            /**<   bus off status */
        uint8_t can_stopped : 1;        /**<   CAN controller stopped */
    };
} can_status_t;

/** @brief       CAN Operation Mode:
 */
typedef union can_mode_t_ {
    uint8_t byte;                       /**< byte access */
    struct {                            /*   bit access: */
        uint8_t mon : 1;                /**<   monitor mode enabled */
        uint8_t err : 1;                /**<   error frames enabled */
        uint8_t nrtr : 1;               /**<   remote frames disabled */
        uint8_t nxtd : 1;               /**<   extended format disabled */
        uint8_t shrd : 1;               /**<   shared access enabled */
#if (OPTION_CAN_2_0_ONLY == 0)
        uint8_t niso : 1;               /**<   Non-ISO CAN FD enabled */
        uint8_t brse : 1;               /**<   bit-rate switch enabled */
        uint8_t fdoe : 1;               /**<   CAN FD operation enabled */
#else
        uint8_t : 3;
#endif
    };
} can_mode_t;

/** @brief       CAN Bit-rate Settings (nominal and data):
 */
typedef union can_bitrate_t_ {
    int32_t index;                      /**< index to predefined bit-rate (<= 0) */
    struct {                            /*   bit-timing register: */
        int32_t frequency;              /**<   clock domain (frequency in [Hz]) */
        struct {                        /*     nominal bus speed: */
            uint16_t brp;               /**<     bit-rate prescaler */
            uint16_t tseg1;             /**<     TSEG1 segment */
            uint16_t tseg2;             /**<     TSEG2 segment */
            uint16_t sjw;               /**<     synchronization jump width */
            uint8_t  sam;               /**<     number of samples (SJA1000) */
        } nominal;                      /**<   nominal bus speed */
#if (OPTION_CAN_2_0_ONLY == 0)
        struct {                        /*     data bus speed: */
            uint16_t brp;               /**<     bit-rate prescaler */
            uint16_t tseg1;             /**<     TSEG1 segment */
            uint16_t tseg2;             /**<     TSEG2 segment */
            uint16_t sjw;               /**<     synchronization jump width */
        } data;                         /**<   data bus speed */
#endif
    } btr;                              /**< bit-timing register */
} can_bitrate_t;

/** @brief       CAN Transmission Rate (nominal and data):
 */
typedef struct can_speed_t_ {
    struct {                            /*   nominal bus speed: */
#if (OPTION_CAN_2_0_ONLY == 0)
        bool  fdoe;                     /**<   CAN FD operation enabled */
#endif
        float speed;                    /**<   bus speed in [Bit/s] */
        float samplepoint;              /**<   sample point in [percent] */
    } nominal;                          /**< nominal bus speed */
#if (OPTION_CAN_2_0_ONLY == 0)
    struct {                            /*   data bus speed: */
        bool  brse;                     /**<   bit-rate switch enabled */
        float speed;                    /**<   bus speed in [Bit/s] */
        float samplepoint;              /**<   sample point in [percent] */
    } data;                             /**< data bus speed */
#endif
} can_speed_t;

/** @brief       CAN Time-stamp:
 *               We use 'struct timespec' with nanoseconds resolution
 */
typedef struct timespec can_timestamp_t;

/** @brief       CAN Message (with Time-stamp):
 */
typedef struct can_message_t_ {
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
    can_timestamp_t timestamp;          /**< time-stamp { sec, nsec } */
} can_message_t;


#ifdef __cplusplus
}
#endif
#endif /* CANAPI_TYPES_H_INCLUDED */
/** @}
 */
/*  ----------------------------------------------------------------------
 *  Uwe Vogt,  UV Software,  Chausseestrasse 33 A,  10115 Berlin,  Germany
 *  Tel.: +49-30-46799872,  Fax: +49-30-46799873,  Mobile: +49-170-3801903
 *  E-Mail: uwe.vogt@uv-software.de,  Homepage: http://www.uv-software.de/
 */
