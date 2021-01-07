/*
 *  CAN Interface API, Version 3 (generic)
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
/** @file        can_api.h
 *
 *  @brief       CAN API V3 for generic CAN Interfaces
 *
 *  @author      $Author: eris $
 *
 *  @version     $Rev: 918 $
 *
 *  @defgroup    can_api CAN Interface API, Version 3
 *  @{
 */
#ifndef CAN_API_H_INCLUDED
#define CAN_API_H_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

/*  -----------  includes  -----------------------------------------------
 */

#include "CANAPI_Defines.h"             /* CAN definitions and options */
#include "CANAPI_Types.h"               /* CAN API data types and defines */


/*  -----------  options  ------------------------------------------------
 */

#if (CAN_API_SPEC != 0x300)
    #error Requires version 3.0 of CANAPI_Types.h
#endif
#if (OPTION_CAN_2_0_ONLY != 0)
    #error Requires CAN FD message format
#endif
#if (OPTION_CANAPI_LIBRARY == 0) && (OPTION_CANAPI_DRIVER == 0)
    #error Option for function signatures not set
#endif
#if (OPTION_CANAPI_DLLEXPORT != 0)
    #define CANAPI  __declspec(dllexport)
#elif (OPTION_CANAPI_DLLIMPORT != 0)
    #define CANAPI  __declspec(dllimport)
#else
    #define CANAPI  extern
#endif

/*  -----------  defines  ------------------------------------------------
 */

/** @name  Aliases
 *  @brief Alternative names
 *  @{ */
typedef int                             can_handle_t;
#define CANAPI_HANDLE                   (can_handle_t)(-1)
#define CANBRD_AVAILABLE                CANBRD_PRESENT
#define CANBRD_UNAVAILABLE              CANBRD_NOT_PRESENT
#define CANBRD_INTESTABLE               CANBRD_NOT_TESTABLE
#define CANEXIT_ALL                     CANKILL_ALL
#define CAN_MAX_EXT_ID                  CAN_MAX_XTD_ID
/** @} */

/** @name  Legacy Stuff
 *  @brief For compatibility reasons with CAN API V1 and V2
 *  @{ */
#define can_transmit(hnd, msg)          can_write(hnd, msg, 0U)
#define can_receive(hnd, msg)           can_read(hnd, msg, 0U)
#define can_msg_t                       can_message_t
/** @} */

#if (OPTION_CANAPI_LIBRARY != 0)
#define CAN_BOARD(lib ,brd)             lib, brd
#elif (OPTION_CANAPI_DRIVER != 0)
#define CAN_BOARD(lib, brd)             brd
#else
#error Remove the unneeded definition(s)!
#endif

/*  -----------  types  --------------------------------------------------
 */

#if (OPTION_CANAPI_LIBRARY != 0)
/** @brief       CAN Board Vendor:
 */
typedef struct can_vendor_t_ {
    int32_t library;                   /**< library id */
    char   *name;                      /**< vendor name */
} can_vendor_t;
#endif
/** @brief       CAN Interface Board:
 */
typedef struct can_board_t_ {
#if (OPTION_CANAPI_LIBRARY != 0)
    int32_t library;                    /**< library id */
#endif
    int32_t type;                       /**< board type */
    char   *name;                       /**< board name */
} can_board_t;


/*  -----------  variables  ----------------------------------------------
 */

#if (OPTION_CANAPI_LIBRARY != 0)
CANAPI can_vendor_t can_vendors[];      /**< list of CAN board vendors */
#endif
CANAPI can_board_t can_boards[];        /**< list of CAN interface boards */


/*  -----------  prototypes  ---------------------------------------------
 */

/** @brief       tests if the CAN interface (hardware and driver) given by
 *               the arguments 'library' and 'board' is present, and if the
 *               requested operation mode is supported by the CAN controller
 *               board.
 *
 *  @note        When a requested operation mode is not supported by the
 *               CAN controller, error CANERR_ILLPARA will be returned.
 *
 *  @remarks     Any loaded DLL will be released when not referenced
 *               by another initialized CAN interface.
 *
 *  @param[in]   library - library id of the CAN interface
 *  @param[in]   board   - type of the CAN controller board
 *  @param[in]   mode    - operation mode to be checked
 *  @param[in]   param   - pointer to board-specific parameters
 *  @param[out]  result  - result of the board test:
 *                             < 0 - board is not present,
 *                             = 0 - board is present,
 *                             > 0 - board is present, but in use
 *
 *  @returns     0 if successful, or a negative value on error.
 *
 *  @retval      CANERR_LIBRARY   - library could not be found
 *  @retval      CANERR_ILLPARA   - illegal parameter value
 *  @retval      CANERR_NOTSUPP   - function not supported
 *  @retval      others           - vendor-specific
 */
#if (OPTION_CANAPI_LIBRARY != 0)
CANAPI int can_test(int32_t library, int32_t board, uint8_t mode, const void *param, int *result);
#else
CANAPI int can_test(int32_t board, uint8_t mode, const void *param, int *result);
#endif

/** @brief       initializes the CAN interface (hardware and driver) by loading
 *               and starting the appropriate DLL for the specified CAN controller
 *               board given by the arguments 'library' and 'board'.
 *               The operation state of the CAN controller is set to 'stopped';
 *               no communication is possible in this state.
 *
 *  @param[in]   library - library id of the CAN interface
 *  @param[in]   board   - type of the CAN controller board
 *  @param[in]   mode    - operation mode of the CAN controller
 *  @param[in]   param   - pointer to board-specific parameters
 *
 *  @returns     handle of the CAN interface if successful,
 *               or a negative value on error.
 *
 *  @retval      CANERR_LIBRARY   - library could not be found
 *  @retval      CANERR_YETINIT   - interface already in use
 *  @retval      CANERR_HANDLE    - no free handle found
 *  @retval      others           - vendor-specific
 */
#if (OPTION_CANAPI_LIBRARY != 0)
CANAPI int can_init(int32_t library, int32_t board, uint8_t mode, const void *param);
#else
CANAPI int can_init(int32_t board, uint8_t mode, const void *param);
#endif


/** @brief       stops any operation of the CAN interface and sets the operation
 *               state of the CAN controller to 'offline'.
 *
 *  @note        The handle is invalid after this operation and could be assigned
 *               to a different CAN controller board in a multy-board application.
 *
 *  @remarks     Afterwards the loaded DLL will be released when not referenced
 *               by another initialized CAN interface.
 *
 *  @param[in]   handle  - handle of the CAN interface, or (-1) to shutdown all
 *
 *  @returns     0 if successful, or a negative value on error.
 *
 *  @retval      CANERR_NOTINIT   - library not initialized
 *  @retval      CANERR_HANDLE    - invalid interface handle
 *  @retval      others           - vendor-specific
 */
CANAPI int can_exit(int handle);


/** @brief       signals a waiting event object of the CAN interface. This can
 *               be used to terminate a blocking read operation in progress
 *               (e.g. by means of a Ctrl-C handler or similar).
 *
 *  @remarks     Some drivers are using waitable objects to realize blocking
 *               operations by a call to WaitForSingleObject (Windows) or
 *               pthread_cond_wait (POSIX), but these waitable objects are
 *               no cancellation points. This means that they cannot be
 *               terminated by Ctrl-C (SIGINT).
 *
 *  @note        SIGINT is not supported for any Win32 application. [MSVC Docs]
 *
 *  @param[in]   handle  - handle of the CAN interface, or (-1) to signal all
 *
 *  @returns     0 if successful, or a negative value on error.
 *
 *  @retval      CANERR_NOTINIT   - library not initialized
 *  @retval      CANERR_HANDLE    - invalid interface handle
 *  @retval      CANERR_NOTSUPP   - function not supported
 *  @retval      others           - vendor-specific
 */
CANAPI int can_kill(int handle);


/** @brief       initializes the operation mode and the bit-rate settings of the
 *               CAN interface and sets the operation state of the CAN controller
 *               to 'running'.
 *
 *  @note        All statistical counters (tx/rx/err) will be reset by this.
 *
 *  @param[in]   handle  - handle of the CAN interface
 *  @param[in]   bitrate - bit-rate as btr register or baud rate index
 *
 *  @returns     0 if successful, or a negative value on error.
 *
 *  @retval      CANERR_NOTINIT   - library not initialized
 *  @retval      CANERR_HANDLE    - invalid interface handle
 *  @retval      CANERR_NULLPTR   - null-pointer assignment
 *  @retval      CANERR_BAUDRATE  - illegal bit-rate settings
 *  @retval      CANERR_ONLINE    - interface already started
 *  @retval      others           - vendor-specific
 */
CANAPI int can_start(int handle, const can_bitrate_t *bitrate);


/** @brief       stops any operation of the CAN interface and sets the operation
 *               state of the CAN controller to 'stopped'; no communication is
 *               possible in this state.
 *
 *  @param[in]   handle  - handle of the CAN interface
 *
 *  @returns     0 if successful, or a negative value on error.
 *
 *  @retval      CANERR_NOTINIT   - library not initialized
 *  @retval      CANERR_HANDLE    - invalid interface handle
 *  @retval      CANERR_OFFLINE   - interface already stopped
 *  @retval      others           - vendor-specific
 */
CANAPI int can_reset(int handle);


/** @brief       transmits a message over the CAN bus. The CAN controller must be
 *               in operation state 'running'.
 *
 *  @param[in]   handle  - handle of the CAN interface
 *  @param[in]   message - pointer to the message to send
 *  @param[in]   timeout - time to wait for the transmission of a message:
 *                              0 means the function returns immediately,
 *                              65535 means blocking read, and any other
 *                              value means the time to wait im milliseconds
 *
 *  @returns     0 if successful, or a negative value on error.
 *
 *  @retval      CANERR_NOTINIT   - library not initialized
 *  @retval      CANERR_HANDLE    - invalid interface handle
 *  @retval      CANERR_NULLPTR   - null-pointer assignment
 *  @retval      CANERR_ILLPARA   - illegal data length code
 *  @retval      CANERR_OFFLINE   - interface not started
 *  @retval      CANERR_TX_BUSY   - transmitter busy
 *  @retval      others           - vendor-specific
 */
CANAPI int can_write(int handle, const can_message_t *message, uint16_t timeout);


/** @brief       read one message from the message queue of the CAN interface, if
 *               any message was received. The CAN controller must be in operation
 *               state 'running'.
 *
 *  @param[in]   handle  - handle of the CAN interface
 *  @param[out]  message - pointer to a message buffer
 *  @param[in]   timeout - time to wait for the reception of a message:
 *                              0 means the function returns immediately,
 *                              65535 means blocking read, and any other
 *                              value means the time to wait im milliseconds
 *
 *  @returns     0 if successful, or a negative value on error.
 *
 *  @retval      CANERR_NOTINIT   - library not initialized
 *  @retval      CANERR_HANDLE    - invalid interface handle
 *  @retval      CANERR_NULLPTR   - null-pointer assignment
 *  @retval      CANERR_OFFLINE   - interface not started
 *  @retval      CANERR_RX_EMPTY  - message queue empty
 *  @retval      CANERR_ERR_FRAME - error frame received
 *  @retval      others           - vendor-specific
 */
CANAPI int can_read(int handle, can_message_t *message, uint16_t timeout);


/** @brief       retrieves the status register of the CAN interface.
 *
 *  @param[in]   handle  - handle of the CAN interface.
 *  @param[out]  status  - 8-bit status register.
 *
 *  @returns     0 if successful, or a negative value on error.
 *
 *  @retval      CANERR_NOTINIT   - library not initialized
 *  @retval      CANERR_HANDLE    - invalid interface handle
 *  @retval      others           - vendor-specific
 */
CANAPI int can_status(int handle, uint8_t *status);


/** @brief       retrieves the bus-load (in percent) of the CAN interface.
 *
 *  @param[in]   handle  - handle of the CAN interface
 *  @param[out]  load    - bus-load in [percent]
 *  @param[out]  status  - 8-bit status register
 *
 *  @returns     0 if successful, or a negative value on error.
 *
 *  @retval      CANERR_NOTINIT   - library not initialized
 *  @retval      CANERR_HANDLE    - invalid interface handle
 *  @retval      others           - vendor-specific
 */
CANAPI int can_busload(int handle, uint8_t *load, uint8_t *status);


/** @brief       retrieves the bit-rate setting of the CAN interface. The
 *               CAN controller must be in operation state 'running'.
 *
 *  @param[in]   handle  - handle of the CAN interface
 *  @param[out]  bitrate - bit-rate setting
 *  @param[out]  speed   - transmission rate
 *
 *  @returns     0 if successful, or a negative value on error.
 *
 *  @retval      CANERR_NOTINIT   - library not initialized
 *  @retval      CANERR_HANDLE    - invalid interface handle
 *  @retval      CANERR_OFFLINE   - interface not started
 *  @retval      CANERR_BAUDRATE  - invalid bit-rate settings
 *  @retval      CANERR_NOTSUPP   - function not supported
 *  @retval      others           - vendor-specific
 */
CANAPI int can_bitrate(int handle, can_bitrate_t *bitrate, can_speed_t *speed);


/** @brief       retrieves or modifies a property value of the CAN interface.
 *
 *  @note        To read or to write a property value of the CAN API V3 DLL,
 *               -1 can be given as handle.
 *
 *  @note        It is also possibel to give the library id of a CAN interface
 *               DLL as argument, to read or to write a property value of that
 *               CAN interface DLL.
 *
 *  @param[in]   handle   - handle or library id of the CAN interface, or (-1)
 *  @param[in]   param    - property id to be read or to be written
 *  @param[out]  value    - pointer to a buffer for the value to be read
 *  @param[in]   value    - pointer to a buffer with the value to be written
 *  @param[in]   nbytes   - size of the given buffer in bytes
 *
 *  @returns     0 if successful, or a negative value on error.
 *
 *  @retval      CANERR_NOTINIT   - library not initialized
 *  @retval      CANERR_HANDLE    - invalid interface handle
 *  @retval      CANERR_NULLPTR   - null-pointer assignment
 *  @retval      CANERR_ILLPARA   - illegal parameter, value or nbytes
 *  @retval      CANERR_...       - tbd.
 *  @retval      CANERR_NOTSUPP   - property or function not supported
 *  @retval      others           - vendor-specific
 */
CANAPI int can_property(int handle, uint16_t param, void *value, uint32_t nbytes);


/** @brief       retrieves the hardware version of the CAN controller
 *               board as a zero-terminated string.
 *
 *  @param[in]   handle  - handle of the CAN interface
 *
 *  @returns     pointer to a zero-terminated string, or NULL on error.
 */
CANAPI char *can_hardware(int handle);


/** @brief       retrieves the firmware version of the CAN controller
 *               board as a zero-terminated string.
 *
 *  @param[in]   handle  - handle of the CAN interface
 *
 *  @returns     pointer to a zero-terminated string, or NULL on error.
 */
CANAPI char *can_software(int handle);


#if (OPTION_CANAPI_LIBRARY != 0)
/** @brief       retrieves version information of the CAN interface
 *               (wrapper library) as a zero-terminated string.
 *
 *  @note        Instead of a valid handle, the library id of a
 *               CAN interface DLL can be given as argument.
 *
 *  @param[in]   handle   - handle or library id of the CAN interface
 *
 *  @returns     pointer to a zero-terminated string, or NULL on error.
 */
CANAPI char *can_library(int handle);
#endif


/** @brief       retrieves version information of the CAN API V3 DLL
 *               as a zero-terminated string.
 *
 *  @returns     pointer to a zero-terminated string, or NULL on error.
 */
CANAPI char* can_version();


#ifdef __cplusplus
}
#endif
#endif /* CAN_API_H_INCLUDED */
/** @}
 */
/*  ----------------------------------------------------------------------
 *  Uwe Vogt,  UV Software,  Chausseestrasse 33 A,  10115 Berlin,  Germany
 *  Tel.: +49-30-46799872,  Fax: +49-30-46799873,  Mobile: +49-170-3801903
 *  E-Mail: uwe.vogt@uv-software.de,  Homepage: http://www.uv-software.de/
 */
