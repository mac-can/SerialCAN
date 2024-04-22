//  SPDX-License-Identifier: BSD-2-Clause OR GPL-3.0-or-later
//
//  CAN Interface API, Version 3 (for CAN-over-Serial-Line Interfaces)
//
//  Copyright (c) 2016-2024 Uwe Vogt, UV Software, Berlin (info@uv-software.com)
//  All rights reserved.
//
//  This file is part of SerialCAN.
//
//  SerialCAN is dual-licensed under the BSD 2-Clause "Simplified" License
//  and under the GNU General Public License v3.0 (or any later version). You can
//  choose between one of them if you use SerialCAN in whole or in part.
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
//  SerialCAN IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
//  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
//  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
//  DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
//  FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
//  DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
//  SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
//  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
//  OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
//  OF SerialCAN, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
//  GNU General Public License v3.0 or later:
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

#include "SerialCAN_Defines.h"
#include "CANBTR_Defaults.h"
#include "CANAPI.h"

/// \name   SerialCAN
/// \brief  SerialCAN dynamic library
/// \{
#define SERIALCAN_LIBRARY_ID  CANLIB_SERIALCAN
#define SERIALCAN_LIBRARY_NAME  CANDLL_SERIALCAN
#define SERIALCAN_LIBRARY_VENDOR  "UV Software, Berlin"
#define SERIALCAN_LIBRARY_LICENSE  "BSD-2-Clause OR GPL-3.0-or-later"
#define SERIALCAN_LIBRARY_COPYRIGHT  "Copyright (c) 2016-2024 by Uwe Vogt, UV Software, Berlin"
#define SERIALCAN_LIBRARY_HAZARD_NOTE  "If you connect your CAN device to a real CAN network when using this library,\n" \
                                       "you might damage your application."
/// \}


/// \name   SerialCAN API
/// \brief  CAN API V3 driver for CAN-over-Serial-Line interfaces
/// \note   See CCanApi for a description of the overridden methods
/// \{
class CANCPP CSerialCAN : public CCanApi {
private:
    CANAPI_Handle_t m_Handle;  ///< CAN interface handle
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
    //static bool GetFirstChannel(SChannelInfo &info, SSerialAttributes &sioAttr);
    //static bool GetNextChannel(SChannelInfo &info, SSerialAttributes &sioAttr);

    static CANAPI_Return_t ProbeChannel(const char *device, const CANAPI_OpMode_t &opMode, EChannelState &state);
    static CANAPI_Return_t ProbeChannel(const char *device, const CANAPI_OpMode_t &opMode, const SSerialAttributes &sioAttr, EChannelState &state);

    CANAPI_Return_t InitializeChannel(const char *device, const CANAPI_OpMode_t &opMode);
    CANAPI_Return_t InitializeChannel(const char *device, const CANAPI_OpMode_t &opMode, const SSerialAttributes &sioAttr);

    // CCanApi overrides
    static bool GetFirstChannel(SChannelInfo &info, void *param = NULL);
    static bool GetNextChannel(SChannelInfo &info, void *param = NULL);

    static CANAPI_Return_t ProbeChannel(int32_t channel, const CANAPI_OpMode_t &opMode, const void *param, EChannelState &state);
    static CANAPI_Return_t ProbeChannel(int32_t channel, const CANAPI_OpMode_t &opMode, EChannelState &state);

    CANAPI_Return_t InitializeChannel(int32_t channel, const CANAPI_OpMode_t &opMode, const void *param = NULL);
    CANAPI_Return_t TeardownChannel();
    CANAPI_Return_t SignalChannel();

    CANAPI_Return_t StartController(CANAPI_Bitrate_t bitrate);
    CANAPI_Return_t ResetController();

    CANAPI_Return_t WriteMessage(CANAPI_Message_t message, uint16_t timeout = 0U);
    CANAPI_Return_t ReadMessage(CANAPI_Message_t &message, uint16_t timeout = CANWAIT_INFINITE);

    CANAPI_Return_t GetStatus(CANAPI_Status_t &status);
    CANAPI_Return_t GetBusLoad(uint8_t &load);

    CANAPI_Return_t GetBitrate(CANAPI_Bitrate_t &bitrate);
    CANAPI_Return_t GetBusSpeed(CANAPI_BusSpeed_t &speed);

    CANAPI_Return_t GetProperty(uint16_t param, void *value, uint32_t nbyte);
    CANAPI_Return_t SetProperty(uint16_t param, const void *value, uint32_t nbyte);

    CANAPI_Return_t SetFilter11Bit(uint32_t code, uint32_t mask);
    CANAPI_Return_t SetFilter29Bit(uint32_t code, uint32_t mask);
    CANAPI_Return_t GetFilter11Bit(uint32_t &code, uint32_t &mask);
    CANAPI_Return_t GetFilter29Bit(uint32_t &code, uint32_t &mask);
    CANAPI_Return_t ResetFilters();

    char *GetHardwareVersion();  // (for compatibility reasons)
    char *GetFirmwareVersion();  // (for compatibility reasons)
    static char *GetVersion();  // (for compatibility reasons)

    static CANAPI_Return_t MapIndex2Bitrate(int32_t index, CANAPI_Bitrate_t &bitrate);
    static CANAPI_Return_t MapString2Bitrate(const char *string, CANAPI_Bitrate_t &bitrate, bool &data, bool &sam);
    static CANAPI_Return_t MapBitrate2String(CANAPI_Bitrate_t bitrate, char *string, size_t length, bool data = false, bool sam = false);
    static CANAPI_Return_t MapBitrate2Speed(CANAPI_Bitrate_t bitrate, CANAPI_BusSpeed_t &speed);
private:
    CANAPI_Return_t MapBitrate2Sja1000(CANAPI_Bitrate_t bitrate, uint16_t &btr0btr1);
    CANAPI_Return_t MapSja10002Bitrate(uint16_t btr0btr1, CANAPI_Bitrate_t &bitrate);
public:
    static uint8_t Dlc2Len(uint8_t dlc) { return CCanApi::Dlc2Len(dlc); }
    static uint8_t Len2Dlc(uint8_t len) { return CCanApi::Len2Dlc(len); }
};
/// \}

/// \name   SerialCAN Property IDs
/// \brief  Properties that can be read (or written)
/// \{
#define SERIALCAN_PROPERTY_CANAPI               (CANPROP_GET_SPEC)
#define SERIALCAN_PROPERTY_VERSION              (CANPROP_GET_VERSION)
#define SERIALCAN_PROPERTY_PATCH_NO             (CANPROP_GET_PATCH_NO)
#define SERIALCAN_PROPERTY_BUILD_NO             (CANPROP_GET_BUILD_NO)
#define SERIALCAN_PROPERTY_LIBRARY_ID           (CANPROP_GET_LIBRARY_ID)
#define SERIALCAN_PROPERTY_LIBRARY_NAME         (CANPROP_GET_LIBRARY_DLLNAME)
#define SERIALCAN_PROPERTY_LIBRARY_VENDOR       (CANPROP_GET_LIBRARY_VENDOR)
#define SERIALCAN_PROPERTY_DEVICE_TYPE          (CANPROP_GET_DEVICE_TYPE)
#define SERIALCAN_PROPERTY_DEVICE_NAME          (CANPROP_GET_DEVICE_NAME)
#define SERIALCAN_PROPERTY_DEVICE_VENDOR        (CANPROP_GET_DEVICE_VENDOR)
#define SERIALCAN_PROPERTY_DEVICE_DRIVER        (CANPROP_GET_DEVICE_DLLNAME)
#define SERIALCAN_PROPERTY_DEVICE_PARAM         (CANPROP_GET_DEVICE_PARAM)
#define SERIALCAN_PROPERTY_OP_CAPABILITY        (CANPROP_GET_OP_CAPABILITY)
#define SERIALCAN_PROPERTY_OP_MODE              (CANPROP_GET_OP_MODE)
#define SERIALCAN_PROPERTY_BITRATE              (CANPROP_GET_BITRATE)
#define SERIALCAN_PROPERTY_SPEED                (CANPROP_GET_SPEED)
#define SERIALCAN_PROPERTY_STATUS               (CANPROP_GET_STATUS)
#define SERIALCAN_PROPERTY_BUSLOAD              (CANPROP_GET_BUSLOAD)
#define SERIALCAN_PROPERTY_NUM_CHANNELS         (CANPROP_GET_NUM_CHANNELS)
#define SERIALCAN_PROPERTY_CAN_CHANNEL          (CANPROP_GET_CAN_CHANNEL)
#define SERIALCAN_PROPERTY_CAN_CLOCK            (CANPROP_GET_CAN_CLOCK)
#define SERIALCAN_PROPERTY_TX_COUNTER           (CANPROP_GET_TX_COUNTER)
#define SERIALCAN_PROPERTY_RX_COUNTER           (CANPROP_GET_RX_COUNTER)
#define SERIALCAN_PROPERTY_ERR_COUNTER          (CANPROP_GET_ERR_COUNTER)
//#define SERIALCAN_PROPERTY_RCV_QUEUE_SIZE       (CANPROP_GET_RCV_QUEUE_SIZE)
//#define SERIALCAN_PROPERTY_RCV_QUEUE_HIGH       (CANPROP_GET_RCV_QUEUE_HIGH)
//#define SERIALCAN_PROPERTY_RCV_QUEUE_OVFL       (CANPROP_GET_RCV_QUEUE_OVFL)
#define SERIALCAN_PROPERTY_SERIAL_NUMBER        (CANPROP_GET_VENDOR_PROP + SLCAN_SERIAL_NUMBER)
#define SERIALCAN_PROPERTY_HARDWARE_VERSION     (CANPROP_GET_VENDOR_PROP + SLCAN_HARDWARE_VERSION)
#define SERIALCAN_PROPERTY_FIRMWARE_VERSION     (CANPROP_GET_VENDOR_PROP + SLCAN_FIRMWARE_VERSION)
#define SERIALCAN_PROPERTY_CLOCK_DOMAIN         (CANPROP_GET_CAN_CLOCK)
/// \}
#endif // SERIALCAN_H_INCLUDED
