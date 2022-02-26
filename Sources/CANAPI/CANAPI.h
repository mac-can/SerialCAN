//  SPDX-License-Identifier: BSD-2-Clause OR GPL-3.0-or-later
//
//  CAN Interface API, Version 3 (Interface Definition)
//
//  Copyright (c) 2004-2022 Uwe Vogt, UV Software, Berlin (info@uv-software.com)
//  All rights reserved.
//
//  This file is part of CAN API V3.
//
//  CAN API V3 is dual-licensed under the BSD 2-Clause "Simplified" License and
//  under the GNU General Public License v3.0 (or any later version).
//  You can choose between one of them if you use this file.
//
//  BSD 2-Clause "Simplified" License:
//  Redistribution and use in source and binary forms, with or without
//  modification, are permitted provided that the following conditions are met:
//  1. Redistributions of source code must retain the above copyright notice, this
//     list of conditions and the following disclaimer.
//  2. Redistributions in binary form must reproduce the above copyright notice,
//     this list of conditions and the following disclaimer in the documentation
//     and/or other materials provided with the distribution.
//
//  CAN API V3 IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
//  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
//  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
//  DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
//  FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
//  DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
//  SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
//  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
//  OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
//  OF CAN API V3, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
//  GNU General Public License v3.0 or later:
//  CAN API V3 is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  CAN API V3 is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with CAN API V3.  If not, see <http://www.gnu.org/licenses/>.
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
/// \remarks     Additionally the class CCanApi provides static methods for
///              bit-rate conversion using CiA bit-timing indexes as a base.
//
/// \note        Set define OPTION_CANAPI_LIBRARY to a non-zero value to compile
///              the master loader library (e.g. in the build environment). Or
///              optionally set define OPTION_CANAPI_DRIVER to a non-zero value
///              to compile a driver/wrapper library.
//
/// \note        Set define OPTION_CANCPP_DLLEXPORT to a non-zero value to compile
///              as a dynamic link library (e.g. in the build environment).
///              In your project set define OPTION_CANCPP_DLLIMPORT to a non-zero
///              value to load the dynamic link library at run-time. Or set it to
///              zero to compile your program with the CAN API source files or to
///              link your program with the static library at compile-time.
///
/// \author      $Author: makemake $
//
/// \version     $Rev: 1033 $
//
/// \defgroup    can_api CAN Interface API, Version 3
/// \{
//
#ifndef CANAPI_H_INCLUDED
#define CANAPI_H_INCLUDED

#include "CANAPI_Defines.h"
#include "CANAPI_Types.h"

#if (CAN_API_SPEC != 0x300)
#error Requires version 3.0 of CANAPI_Types.h
#endif
#if (OPTION_CANAPI_LIBRARY == 0)
#if  (OPTION_CANAPI_DRIVER == 0)
#define OPTION_CANAPI_DRIVER  1
#endif
#endif
#if (OPTION_CANCPP_DLLEXPORT != 0)
#define CANCPP  __declspec(dllexport)
#elif (OPTION_CANCPP_DLLIMPORT != 0)
#define CANCPP  __declspec(dllimport)
#else
#define CANCPP
#endif

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
class CANCPP CCanApi {
public:
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
        ResourceError = CANERR_RESOURCE,  ///< resource allocation
        InvalidBaudrate = CANERR_BAUDRATE,  ///<  illegal baudrate
        InvalidHandle = CANERR_HANDLE,  ///<  illegal handle
        IllegalParameter = CANERR_ILLPARA,  ///< illegal parameter
        NullPointer = CANERR_NULLPTR,  ///< null-pointer assignment
        NotInitialized = CANERR_NOTINIT,  ///< not initialized
        AlreadyInitialized = CANERR_YETINIT,  ///< already initialized
        InvalidLibrary = CANERR_LIBRARY, ///< illegal library
        NotSupported = CANERR_NOTSUPP,  ///< not supported
        FatalError = CANERR_FATAL,  ///< fatal error
        VendorSpecific = CANERR_VENDOR  ///< offset for vendor-specific error code
    };
    /// \brief  CAN channel states
    enum EChannelState {
        ChannelOccupied = CANBRD_OCCUPIED, ///< channel is available, but occupied
        ChannelAvailable = CANBRD_PRESENT, ///< channel is available and can be used
        ChannelNotAvailable = CANBRD_NOT_PRESENT,  ///< channel is not available
        ChannelNotTestable  = CANBRD_NOT_TESTABLE  ///< channel is not testable
    };
    /// \brief  CAN channel information
    struct SChannelInfo {
        int32_t m_nChannelNo;  ///< channel no. at actual index in the interface list 
        char m_szDeviceName[CANPROP_MAX_BUFFER_SIZE];  ///< device name at actual index in the interface list 
        char m_szDeviceDllName[CANPROP_MAX_BUFFER_SIZE];  ///< file name of the DLL at actual index in the interface list
        int32_t m_nLibraryId;  ///< library id at actual index in the interface list
        char m_szVendorName[CANPROP_MAX_BUFFER_SIZE];  ///< vendor name at actual index in the interface list
    };
#if (OPTION_CANAPI_LIBRARY != 0)
    /// \brief  CAN API library information
    struct SLibraryInfo {
        int32_t m_nLibraryId;  ///< library id at actual index in the vendor list
        char m_szVendorName[CANPROP_MAX_BUFFER_SIZE];  ///< vendor name at actual index in the vendor list
        char m_szVendorDllName[CANPROP_MAX_BUFFER_SIZE];  ///< file name of the DLL at actual index in the vendor list
    };
    /// \brief       query library information of the first CAN API library in the
    ///              list of vendors, if any.
    //
    /// \param[out]  info    - the library information of the first entry in the list
    //
    /// \returns     true if library information have been successfully read, or
    ///              false on error.
    //
    static bool GetFirstLibrary(SLibraryInfo &info);
    
    /// \brief       query library information of the next CAN API library in the
    ///              list of vendors, if any.
    //
    /// \param[out]  info    - the library information of the next entry in the list
    //
    /// \returns     true if library information have been successfully read, or
    ///              false on error.
    //
    static bool GetNextLibrary(SLibraryInfo &info);
#endif
    /// \brief       query channel information of the first CAN interface in the
    ///              list of CAN interfaces, if any.
    //
    /// \param[in]   library - library id of the CAN interface list, or -1 for all vendors
    /// \param[out]  info    - the channel information of the first entry in the list
    /// \param[out]  param   - pointer to channel-specific parameters
    //
    /// \returns     true if channel information have been successfully read, or
    ///              false on error.
    //
#if (OPTION_CANAPI_LIBRARY != 0)
    static bool GetFirstChannel(int32_t library, SChannelInfo &info, void *param = NULL);
#else
    static bool GetFirstChannel(SChannelInfo &info, void *param = NULL);
#endif

    /// \brief       query channel information of the first CAN interface in the
    ///              list of CAN interfaces, if any.
    //
    /// \param[in]   library - library id of the CAN interface list, or -1 for all vendors
    /// \param[out]  info    - the channel information of the next entry in the list
    /// \param[out]  param   - pointer to channel-specific parameters
    //
    /// \returns     true if channel information have been successfully read, or
    ///              false on error.
    //
#if (OPTION_CANAPI_LIBRARY != 0)
    static bool GetNextChannel(int32_t library, SChannelInfo &info, void *param = NULL);
#else
   static bool GetNextChannel(SChannelInfo &info, void *param = NULL);
#endif

    /// \brief       probes if the CAN interface (hardware and driver) given by
    ///              the argument [ 'library' and ] 'channel' is present,
    ///              and if the requested operation mode is supported by the
    ///              CAN controller.
    //
    /// \note        When a requested operation mode is not supported by the
    ///              CAN controller, error CANERR_ILLPARA will be returned.
    //
    /// \param[in]   library - library id of the CAN interface
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
#if (OPTION_CANAPI_LIBRARY != 0)
    static CANAPI_Return_t ProbeChannel(int32_t library, int32_t channel, const CANAPI_OpMode_t &opMode, const void *param, EChannelState &state);
    static CANAPI_Return_t ProbeChannel(int32_t library, int32_t channel, const CANAPI_OpMode_t &opMode, EChannelState &state);
#else
    static CANAPI_Return_t ProbeChannel(int32_t channel, const CANAPI_OpMode_t &opMode, const void *param, EChannelState &state);
    static CANAPI_Return_t ProbeChannel(int32_t channel, const CANAPI_OpMode_t &opMode, EChannelState &state);
#endif

    /// \brief       initializes the CAN interface (hardware and driver) given by
    ///              the argument [ 'library' and ] 'channel'.
    ///              The operation state of the CAN controller is set to 'stopped';
    ///              no communication is possible in this state.
    //
    /// \param[in]   library - library id of the CAN interface
    /// \param[in]   channel - channel number of the CAN interface
    /// \param[in]   opMode  - operation mode of the CAN controller
    /// \param[in]   param   - pointer to channel-specific parameters
    //
    /// \returns     handle of the CAN interface if successful,
    ///              or a negative value on error.
    //
#if (OPTION_CANAPI_LIBRARY != 0)
    virtual CANAPI_Return_t InitializeChannel(int32_t library, int32_t channel, const CANAPI_OpMode_t &opMode, const void *param = NULL) = 0;
#else
    virtual CANAPI_Return_t InitializeChannel(int32_t channel, const CANAPI_OpMode_t &opMode, const void *param = NULL) = 0;
#endif

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
    ///                             value means the time to wait in milliseconds
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
    ///                             value means the time to wait in milliseconds
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
    virtual CANAPI_Return_t GetBusLoad(uint8_t &load) = 0;  // deprecated

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
    /// \param[in]   nbyte   -  size of the given buffer in byte
    //
    /// \returns     0 if successful, or a negative value on error.
    //
    virtual CANAPI_Return_t GetProperty(uint16_t param, void *value, uint32_t nbyte) = 0;

    /// \brief       modifies a property value of the CAN interface.
    //
    /// \param[in]   param    - property id to be written
    /// \param[in]   value    - pointer to a buffer with the value to be written
    /// \param[in]   nbyte   -  size of the given buffer in byte
    //
    /// \returns     0 if successful, or a negative value on error.
    //
    virtual CANAPI_Return_t SetProperty(uint16_t param, const void *value, uint32_t nbyte) = 0;

    /// \brief       retrieves the hardware version of the CAN controller
    ///              board as a zero-terminated string.
    //
    /// \returns     pointer to a zero-terminated string, or NULL on error.
    //
    virtual char *GetHardwareVersion() = 0;  // deprecated

    /// \brief       retrieves the firmware version of the CAN controller
    ///              board as a zero-terminated string.
    //
    /// \returns     pointer to a zero-terminated string, or NULL on error.
    //
    virtual char *GetFirmwareVersion() = 0;  // deprecated

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

/// \name   CAN FD Data Length Code
/// \brief  Methods for DLC conversion.
/// \{
public:
    static uint8_t Dlc2Len(uint8_t dlc) {
        const static uint8_t dlc_table[16] = {
#if (OPTION_CAN_2_0_ONLY == 0)
            0U, 1U, 2U, 3U, 4U, 5U, 6U, 7U, 8U, 12U, 16U, 20U, 24U, 32U, 48U, 64U
#else
            0U, 1U, 2U, 3U, 4U, 5U, 6U, 7U, 8U, 8U, 8U, 8U, 8U, 8U, 8U, 8U
#endif
        };
        return dlc_table[dlc & 0xFU];
    }
    static uint8_t Len2Dlc(uint8_t len) {
#if (OPTION_CAN_2_0_ONLY == 0)
        if(len > 48U) return 0x0FU;
        if(len > 32U) return 0x0EU;
        if(len > 24U) return 0x0DU;
        if(len > 20U) return 0x0CU;
        if(len > 16U) return 0x0BU;
        if(len > 12U) return 0x0AU;
        if(len > 8U) return 0x09U;
#else
        if(len > 8U) return 0x08U;
#endif
    return len;
    }
/// \}
};
/// \}
#endif // CANAPI_H_INCLUDED
/// \}
// $Id: CANAPI.h 1033 2022-01-11 19:58:04Z makemake $  Copyright (c) UV Software //
