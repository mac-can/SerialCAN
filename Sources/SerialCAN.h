//
//  SerialCAN - CAN API V3 Driver for CAN-over-Serial-Line Interfaces
//
//  Copyright (C) 2020  Uwe Vogt, UV Software, Berlin (info@uv-software.com)
//
//  This file is part of SerialCAN.
//
//  SerialCAN is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  SerialCAN is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with SerialCAN.  If not, see <https://www.gnu.org/licenses/>.
//
#ifndef SERIALCAN_H_INCLUDED
#define SERIALCAN_H_INCLUDED

#include "CANAPI.h"
#include "CANAPI_SerialCAN.h"

/// \name   SerialCAN
/// \brief  SerialCAN dynamic library
/// \{
#define SERIALCAN_LIBRARY_ID  CANLIB_SERIALCAN
#define SERIALCAN_LIBRARY_NAME  CANDLL_SERIALCAN
#define SERIALCAN_LIBRARY_VENDOR  "UV Software, Berlin"
#define SERIALCAN_LIBRARY_LICENSE  "GNU General Public License, Version 3"
#define SERIALCAN_LIBRARY_COPYRIGHT  "Copyright (C) 2016,2020  Uwe Vogt, UV Software, Berlin"
#define SERIALCAN_LIBRARY_HAZARD_NOTE  "If you connect your CAN device to a real CAN network when using this library,\n" \
                                       "you might damage your application."
/// \}


/// \name   SerialCAN API
/// \brief  CAN API V3 driver for CAN-over-Serial-Line interfaces
/// \note   See CCANAPI for a description of the overridden methods
/// \{
class CSerialCAN : public CCANAPI {
private:
    char m_szTtyName[CANPROP_MAX_BUFFER_SIZE];  ///< TTY device name
    CANAPI_OpMode_t m_OpMode;  ///< CAN operation mode
    CANAPI_Status_t m_Status;  ///< CAN status register
    CANAPI_Bitrate_t m_Bitrate;  ///< CAN bitrate settings
    struct {
        uint64_t u64TxMessages;  ///< number of transmitted CAN messages
        uint64_t u64RxMessages;  ///< number of received CAN messages
        uint64_t u64ErrorFrames;  ///< number of received status messages
    } m_Counter;
    // opaque data type
    struct SSLCAN;  ///< C++ forward declaration
    SSLCAN *m_pSLCAN;  ///< serial line interface
public:
    // constructor / destructor
    CSerialCAN();
    ~CSerialCAN();
    // CSerialCAN-specific error codes (CAN API V3 extension)
    enum EErrorCodes {
        // note: range 0...-99 is reserved by CAN API V3
        GeneralError = VendorSpecific
    };
    // serial line attributes
    typedef can_sio_attr_t SSerialAttributes;

    // CSerial methods
    static CANAPI_Return_t ProbeChannel(const char *device, CANAPI_OpMode_t opMode, EChannelState &state);
    static CANAPI_Return_t ProbeChannel(const char *device, CANAPI_OpMode_t opMode, SSerialAttributes sioAttr, EChannelState &state);

    CANAPI_Return_t InitializeChannel(const char *device, can_mode_t opMode);
    CANAPI_Return_t InitializeChannel(const char *device, can_mode_t opMode, SSerialAttributes sioAttr);

    // CCANAPI overrides
    static CANAPI_Return_t ProbeChannel(int32_t channel, CANAPI_OpMode_t opMode, const void *param, EChannelState &state);
    static CANAPI_Return_t ProbeChannel(int32_t channel, CANAPI_OpMode_t opMode, EChannelState &state);

    CANAPI_Return_t InitializeChannel(int32_t channel, can_mode_t opMode, const void *param = NULL);
    CANAPI_Return_t TeardownChannel();
    CANAPI_Return_t SignalChannel();

    CANAPI_Return_t StartController(CANAPI_Bitrate_t bitrate);
    CANAPI_Return_t ResetController();

    CANAPI_Return_t WriteMessage(CANAPI_Message_t message, uint16_t timeout = 0U);
    CANAPI_Return_t ReadMessage(CANAPI_Message_t &message, uint16_t timeout = CANREAD_INFINITE);

    CANAPI_Return_t GetStatus(CANAPI_Status_t &status);
    CANAPI_Return_t GetBusLoad(uint8_t &load);

    CANAPI_Return_t GetBitrate(CANAPI_Bitrate_t &bitrate);
    CANAPI_Return_t GetBusSpeed(CANAPI_BusSpeed_t &speed);

    CANAPI_Return_t GetProperty(uint16_t param, void *value, uint32_t nbytes);
    CANAPI_Return_t SetProperty(uint16_t param, const void *value, uint32_t nbytes);

    char *GetHardwareVersion();  // (for compatibility reasons)
    char *GetFirmwareVersion();  // (for compatibility reasons)
    static char *GetVersion();  // (for compatibility reasons)

    static CANAPI_Return_t MapIndex2Bitrate(int32_t index, CANAPI_Bitrate_t &bitrate);
    static CANAPI_Return_t MapString2Bitrate(const char *string, CANAPI_Bitrate_t &bitrate);
    static CANAPI_Return_t MapBitrate2String(CANAPI_Bitrate_t bitrate, char *string, size_t length);
    static CANAPI_Return_t MapBitrate2Speed(CANAPI_Bitrate_t bitrate, CANAPI_BusSpeed_t &speed);
private:
    CANAPI_Return_t MapBitrate2Sja1000(CANAPI_Bitrate_t bitrate, uint16_t &btr0btr1);
    CANAPI_Return_t MapSja10002Bitrate(uint16_t btr0btr1, CANAPI_Bitrate_t &bitrate);
    CANAPI_Return_t MapErrorCode(int code);
};
/// \}

/// \name   SerialCAN Property IDs
/// \brief  Properties that can be read (or written)
/// \{
#define SERIALCAN_PROPERTY_CANAPI            (CANPROP_GET_SPEC)
#define SERIALCAN_PROPERTY_VERSION           (CANPROP_GET_VERSION)
#define SERIALCAN_PROPERTY_PATCH_NO          (CANPROP_GET_PATCH_NO)
#define SERIALCAN_PROPERTY_BUILD_NO          (CANPROP_GET_BUILD_NO)
#define SERIALCAN_PROPERTY_LIBRARY_ID        (CANPROP_GET_LIBRARY_ID)
#define SERIALCAN_PROPERTY_LIBRARY_NAME      (CANPROP_GET_LIBRARY_DLLNAME)
#define SERIALCAN_PROPERTY_LIBRARY_VENDOR    (CANPROP_GET_LIBRARY_VENDOR)
#define SERIALCAN_PROPERTY_DEVICE_TYPE       (CANPROP_GET_DEVICE_TYPE)
#define SERIALCAN_PROPERTY_DEVICE_NAME       (CANPROP_GET_DEVICE_NAME)
#define SERIALCAN_PROPERTY_DEVICE_VENDOR     (CANPROP_GET_DEVICE_VENDOR)
#define SERIALCAN_PROPERTY_DEVICE_DLLNAME    (CANPROP_GET_DEVICE_DLLNAME)
#define SERIALCAN_PROPERTY_DEVICE_PARAM      (CANPROP_GET_DEVICE_PARAM)
#define SERIALCAN_PROPERTY_OP_CAPABILITY     (CANPROP_GET_OP_CAPABILITY)
#define SERIALCAN_PROPERTY_OP_MODE           (CANPROP_GET_OP_MODE)
#define SERIALCAN_PROPERTY_BITRATE           (CANPROP_GET_BITRATE)
#define SERIALCAN_PROPERTY_SPEED             (CANPROP_GET_SPEED)
#define SERIALCAN_PROPERTY_STATUS            (CANPROP_GET_STATUS)
#define SERIALCAN_PROPERTY_BUSLOAD           (CANPROP_GET_BUSLOAD)
#define SERIALCAN_PROPERTY_TX_COUNTER        (CANPROP_GET_TX_COUNTER)
#define SERIALCAN_PROPERTY_RX_COUNTER        (CANPROP_GET_RX_COUNTER)
#define SERIALCAN_PROPERTY_ERR_COUNTER       (CANPROP_GET_ERR_COUNTER)
#define SERIALCAN_PROPERTY_CLOCK_DOMAIN      (CANPROP_GET_VENDOR_PROP + 0U)
#define SERIALCAN_PROPERTY_HARDWARE_VERSION  (CANPROP_GET_VENDOR_PROP + 1U)
#define SERIALCAN_PROPERTY_SOFTWARE_VERSION  (CANPROP_GET_VENDOR_PROP + 2U)
#define SERIALCAN_PROPERTY_SERIAL_NUMBER     (CANPROP_GET_VENDOR_PROP + 3U)
/// \}
#endif // SERIALCAN_H_INCLUDED
