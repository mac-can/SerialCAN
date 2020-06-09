//
//  CAN Interface API, Version 3 (Interface Definition)
//
//  Copyright (C) 2004-2020  Uwe Vogt, UV Software, Berlin (info@uv-software.com)
//
//  This file is part of CAN API V3.
//
//  CAN API V3 is free software: you can redistribute it and/or modify
//  it under the terms of the GNU Lesser General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  CAN API V3 is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU Lesser General Public License for more details.
//
//  You should have received a copy of the GNU Lesser General Public License
//  along with CAN API V3.  If not, see <https://www.gnu.org/licenses/>.
//
/// \file        CANAPI.h
//
/// \brief       CAN API V3 for generic CAN Interfaces
//
/// \brief       CAN API V3 is a wrapper specification to have a uniform CAN
///              Interface API for various CAN interfaces from different
///              vendors running under multiple operating systems.
//
/// \remarks     This header file provides an abstract class with pure virtual
///              methods and all data types and defines for the implementation
///              of a CAN API V3 wrapper library for a CAN adapter from an
///              arbitrary vendor.
//
/// \remarks     Additionally the class CCANAPI provides static methods for
///              bit-rate conversion using CiA bit-timing indexes as a base.
//
/// \author      $Author: haumea $
//
/// \version     $Rev: 912 $
//
/// \defgroup    can_api CAN Interface API, Version 3
/// \{
//
#ifndef CANAPI_H_INCLUDED
#define CANAPI_H_INCLUDED

#include "CANAPI_Defines.h"
#include "CANAPI_Types.h"

/// \name   Aliases
/// \brief  CAN API V3 Data-types.
/// \{

/// \brief  CAN Status-register
//
typedef can_status_t CANAPI_Status_t;

/// \brief  CAN Operation Mode
//
typedef can_mode_t CANAPI_OpMode_t;

/// \brief  CAN Bit-rate Settings (nominal and data)
//
typedef can_bitrate_t CANAPI_Bitrate_t;

/// \brief  CAN Transmission Rate (nominal and data)
//
typedef can_speed_t CANAPI_BusSpeed_t;

/// \brief  CAN Time-stamp
//
typedef can_timestamp_t CANAPI_Timestamp_t;

/// \brief  CAN Message (with Time-stamp)
//
typedef can_message_t CANAPI_Message_t;

/// \brief  CAN Device handle (internally)
//
typedef int CANAPI_Handle_t;

/// \brief  Function results (data type)
//
typedef int CANAPI_Return_t;
/// @}

/// \name   CAN API V3
/// \brief  An abstract class for CAN API V3 campatible CAN driver implementations.
/// \{
class CCANAPI {
public:
    /// \brief  CAN channel states
    enum EChannelState {
        ChannelOccupied = CANBRD_OCCUPIED, ///< channel is available, but occupied
        ChannelAvailable = CANBRD_PRESENT, ///< channel is available and can be used
        ChannelNotAvailable = CANBRD_NOT_PRESENT,  ///< channel is not available
        ChannelNotTestable  = CANBRD_NOT_TESTABLE  ///< channel is not testable
    };
    /// \brief  Common error codes (CAN API V3 compatible)
    enum EErrorCodes {
        NoError = CANERR_NOERROR,  ///< no error!
        BusOFF = CANERR_BOFF,  ///< busoff status
        ErrorWarning = CANERR_EWRN,  ///< error warning status
        BusError = CANERR_BERR,  ///< bus error
        ControllerOffline = CANERR_OFFLINE,  ///< not started
        ControllerOnline = CANERR_ONLINE,  ///< already started
        MessageLost = CANERR_MSG_LST,  ///< message lost
        TransmitterBusy = CANERR_TX_BUSY,  ///< transmitter busy
        ReceiverEmpty = CANERR_RX_EMPTY,  ///< receiver empty
        ErrorFrame = CANERR_ERR_FRAME,  ///< error frame
        Timeout = CANERR_TIMEOUT,  ///< timed out
        InvalidBaudrate = CANERR_BAUDRATE,  ///<  illegal baudrate
        IllegalParameter = CANERR_ILLPARA,  ///< illegal parameter
        NullPointer = CANERR_NULLPTR,  ///< null-pointer assignment
        NotInitialized = CANERR_NOTINIT,  ///< not initialized
        AlreadyInitialized = CANERR_YETINIT,  ///< already initialized
        NotSupported = CANERR_NOTSUPP,  ///< not supported
        FatalError = CANERR_FATAL,  ///< fatal error
        VendorSpecific = CANERR_VENDOR  ///< offset for vendor-specific error code
    };
    /// \brief       probes if the CAN interface (hardware and driver) given by
    ///              the argument 'channel' is present, and if the requested
    ///              operation mode is supported by the CAN controller.
    //
    /// \note        When a requested operation mode is not supported by the
    ///              CAN controller, error CANERR_ILLPARA will be returned.
    //
    /// \param[in]   channel - channel number of the CAN interface
    /// \param[in]   opMode  - operation mode to be checked
    /// \param[in]   param   - pointer to channel-specific parameters
    /// \param[out]  state   - state of the CAN channel:
    ///                            < 0 - channel is not present,
    ///                            = 0 - channel is present,
    ///                            > 0 - channel is present, but in use
    //
    /// \returns     0 if successful, or a negative value on error.
    //
    static CANAPI_Return_t ProbeChannel(int32_t channel, CANAPI_OpMode_t opMode, const void *param, EChannelState &state);
    static CANAPI_Return_t ProbeChannel(int32_t channel, CANAPI_OpMode_t opMode, EChannelState &state);

    /// \brief       initializes the CAN interface (hardware and driver) given by
    ///              the argument 'channel'.
    ///              The operation state of the CAN controller is set to 'stopped';
    ///              no communication is possible in this state.
    //
    /// \param[in]   channel - channel number of the CAN interface
    /// \param[in]   opMode  - operation mode of the CAN controller
    /// \param[in]   param   - pointer to channel-specific parameters
    //
    /// \returns     handle of the CAN interface if successful,
    ///              or a negative value on error.
    //
    virtual CANAPI_Return_t InitializeChannel(int32_t channel, CANAPI_OpMode_t opMode, const void *param = NULL) = 0;

    /// \brief       stops any operation of the CAN interface and sets the operation
    ///              state of the CAN controller to 'stopped'.
    //
    /// \returns     0 if successful, or a negative value on error.
    ///
    virtual CANAPI_Return_t TeardownChannel() = 0;

    /// \brief       signals waiting event objects of the CAN interface. This can
    ///              be used to terminate blocking operations in progress
    ///              (e.g. by means of a Ctrl-C handler or similar).
    //
    /// \remarks     Some drivers are using waitable objects to realize blocking
    ///              operations by a call to WaitForSingleObject (Windows) or
    ///              pthread_cond_wait (POSIX), but these waitable objects are
    ///              no cancellation points. This means that they cannot be
    ///              terminated by Ctrl-C (SIGINT).
    //
    /// \note        SIGINT is not supported for any Win32 application. [MSVC Docs]
    //
    /// \returns     0 if successful, or a negative value on error.
    ///
    virtual CANAPI_Return_t SignalChannel() = 0;

    /// \brief       initializes the operation mode and the bit-rate settings of the
    ///              CAN interface and sets the operation state of the CAN controller
    ///              to 'running'.
    //
    /// \note        All statistical counters (tx/rx/err) will be reset by this.
    //
    /// \param[in]   bitrate - bit-rate as btr register or baud rate index
    //
    /// \returns     0 if successful, or a negative value on error.
    //
    virtual CANAPI_Return_t StartController(CANAPI_Bitrate_t bitrate) = 0;

    /// \brief       stops any operation of the CAN interface and sets the operation
    ///              state of the CAN controller to 'stopped'; no communication is
    ///              possible in this state.
    //
    /// \returns     0 if successful, or a negative value on error.
    //
    virtual CANAPI_Return_t ResetController() = 0;

    /// \brief       transmits one message over the CAN bus. The CAN controller must
    ///              be in operation state 'running'.
    //
    /// \param[in]   message - the message to send
    /// \param[in]   timeout - time to wait for the transmission of the message:
    ///                             0 means the function returns immediately,
    ///                             65535 means blocking read, and any other
    ///                             value means the time to wait im milliseconds
    //
    /// \returns     0 if successful, or a negative value on error.
    //
    virtual CANAPI_Return_t WriteMessage(CANAPI_Message_t message, uint16_t timeout = 0U) = 0;

    /// \brief       read one message from the message queue of the CAN interface, if
    ///              any message was received. The CAN controller must be in operation
    ///              state 'running'.
    //
    /// \param[out]  message - the message read from the message queue, if any
    /// \param[in]   timeout - time to wait for the reception of a message:
    ///                             0 means the function returns immediately,
    ///                             65535 means blocking read, and any other
    ///                             value means the time to wait im milliseconds
    //
    /// \returns     0 if successful, or a negative value on error.
    //
    virtual CANAPI_Return_t ReadMessage(CANAPI_Message_t &message, uint16_t timeout = CANREAD_INFINITE) = 0;

    /// \brief       retrieves the status register of the CAN interface.
    //
    /// \param[out]  status  - 8-bit status register.
    //
    /// \returns     0 if successful, or a negative value on error.
    //
    virtual CANAPI_Return_t GetStatus(CANAPI_Status_t &status) = 0;

    /// \brief       retrieves the bus-load (in percent) of the CAN interface.
    //
    /// \param[out]  load    - bus-load in [%]
    //
    /// \returns     0 if successful, or a negative value on error.
    //
    virtual CANAPI_Return_t GetBusLoad(uint8_t &load) = 0;

    /// \brief       retrieves the bit-rate setting of the CAN interface. The
    ///              CAN controller must be in operation state 'running'.
    //
    /// \param[out]  bitrate - bit-rate setting
    //
    /// \returns     0 if successful, or a negative value on error.
    //
    virtual CANAPI_Return_t GetBitrate(CANAPI_Bitrate_t &bitrate) = 0;

    /// \brief       retrieves the transmission rate of the CAN interface. The
    ///              CAN controller must be in operation state 'running'.
    //
    /// \param[out]  speed   - transmission rate
    //
    /// \returns     0 if successful, or a negative value on error.
    //
    virtual CANAPI_Return_t GetBusSpeed(CANAPI_BusSpeed_t &speed) = 0;

    /// \brief       retrieves a property value of the CAN interface.
    //
    /// \param[in]   param    - property id to be read
    /// \param[out]  value    - pointer to a buffer for the value to be read
    /// \param[in]   nbytes   - size of the given buffer in bytes
    //
    /// \returns     0 if successful, or a negative value on error.
    //
    virtual CANAPI_Return_t GetProperty(uint16_t param, void *value, uint32_t nbytes) = 0;

    /// \brief       modifies a property value of the CAN interface.
    //
    /// \param[in]   param    - property id to be written
    /// \param[in]   value    - pointer to a buffer with the value to be written
    /// \param[in]   nbytes   - size of the given buffer in bytes
    //
    /// \returns     0 if successful, or a negative value on error.
    //
    virtual CANAPI_Return_t SetProperty(uint16_t param, const void *value, uint32_t nbytes) = 0;

    /// \brief       retrieves the hardware version of the CAN controller
    ///              board as a zero-terminated string.
    //
    /// \returns     pointer to a zero-terminated string, or NULL on error.
    //
    virtual char *GetHardwareVersion() = 0;

    /// \brief       retrieves the firmware version of the CAN controller
    ///              board as a zero-terminated string.
    //
    /// \returns     pointer to a zero-terminated string, or NULL on error.
    //
    virtual char *GetFirmwareVersion() = 0;

    /// \brief       retrieves version information of the CAN API V3 driver
    ///              as a zero-terminated string.
    //
    /// \returns     pointer to a zero-terminated string, or NULL on error.
    //
    static char *GetVersion();

/// \name   CAN API V3 Bit-rate converstion
/// \brief  Methods for bit-rate conversion.
/// \note   To be overridden when required.
/// \{
public:
    static CANAPI_Return_t MapIndex2Bitrate(int32_t index, CANAPI_Bitrate_t &bitrate);
    static CANAPI_Return_t MapString2Bitrate(const char *string, CANAPI_Bitrate_t &bitrate);
    static CANAPI_Return_t MapBitrate2String(CANAPI_Bitrate_t bitrate, char *string, size_t length);
    static CANAPI_Return_t MapBitrate2Speed(CANAPI_Bitrate_t bitrate, CANAPI_BusSpeed_t &speed);
/// \}
};
/// \}
#endif // CANAPI_H_INCLUDED
/// \}
// $Id: CANAPI.h 912 2020-06-09 16:36:41Z haumea $  Copyright (C) UV Software //
