//
//  SerialCAN - CAN API V3 Driver for CAN-over-Serial-Line Interfaces
//
//  Copyright (C) 2016,2020-2021  Uwe Vogt, UV Software, Berlin (info@uv-software.com)
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
#include "SerialCAN.h"
#include "slcan.h"
#include "debug.h"
#include "can_btr.h"

#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <assert.h>

#include "build_no.h"
#define VERSION_MAJOR    0
#define VERSION_MINOR    1
#define VERSION_PATCH    0
#define VERSION_BUILD    BUILD_NO
#define VERSION_STRING   TOSTRING(VERSION_MAJOR) "." TOSTRING(VERSION_MINOR) "." TOSTRING(VERSION_PATCH) " (" TOSTRING(BUILD_NO) ")"
#if defined(_WIN64)
#define PLATFORM        "x64"
#elif defined(_WIN32)
#define PLATFORM        "x86"
#elif defined(__linux__)
#define PLATFORM        "Linux"
#elif defined(__APPLE__)
#define PLATFORM        "macOS"
#elif defined(__CYGWIN__)
#define PLATFORM        "Cygwin"
#else
#error Unsupported architecture
#endif
static const char version[] = PLATFORM " Driver for CAN-over-Serial-Line Interfaces, Version " VERSION_STRING;

#if (OPTION_SERIALCAN_DYLIB != 0)
__attribute__((constructor))
static void _initializer() {
    LOGGER_INIT("slcan.log");
}
__attribute__((destructor))
static void _finalizer() {
    LOGGER_EXIT();
}
#define EXPORT  __attribute__((visibility("default")))
#else
#define EXPORT
#endif
#define QUEUE_SIZE  65536U
#define DEVICE_TYPE  0U
#define VENDOR_NAME  "(unknown)"
#define VENDOR_DLLNAME  "(none)"
#if !defined(_WIN32) && !defined(_WIN64)
#define SERIAL_PORTNAME  "/dev/ttyS%i"
#else
#define SERIAL_PORTNAME  "\\\\.\\COM%i"
#endif
#define SERIAL_BAUDRATE  115200U
#define SERIAL_BYTESIZE  CANSIO_8DATABITS
#define SERIAL_PARITY  CANSIO_NOPARITY
#define SERIAL_STOPBITS  CANSIO_1STOPBIT
#define SERIAL_OPTIONS  (CANSIO_SLCAN)
#define CLOCK_DOMAIN  8000000
#define SUPPORTED_OP_MODE  (CANMODE_DEFAULT)

#ifndef _MSC_VER
#define STRCPY_S(dest,size,src)         strcpy(dest,src)
#define STRNCPY_S(dest,size,src,len)    strncpy(dest,src,len)
#define SSCANF_S(buf,format,...)        sscanf(buf,format,__VA_ARGS__)
#define SPRINTF_S(buf,size,format,...)  sprintf(buf,format,__VA_ARGS__)
#else
#define STRCPY_S(dest,size,src)         strcpy_s(dest,size,src)
#define STRNCPY_S(dest,size,src,len)    strncpy_s(dest,size,src,len)
#define SSCANF_S(buf,format,...)        sscanf_s(buf,format,__VA_ARGS__)
#define SPRINTF_S(buf,size,format,...)  sprintf_s(buf,size,format,__VA_ARGS__)
#endif

struct CSerialCAN::SSLCAN {
    // attributes
    bool m_bInitialized;
    slcan_port_t m_Port;
    SSerialAttributes m_Attr;
    // constructor/destructor
    SSLCAN() {
        m_bInitialized = false;
        m_Attr.baudrate = SERIAL_BAUDRATE;
        m_Attr.bytesize = SERIAL_BYTESIZE;
        m_Attr.parity = SERIAL_PARITY;
        m_Attr.stopbits = SERIAL_STOPBITS;
        m_Attr.options = SERIAL_OPTIONS;
        m_Port = slcan_create(QUEUE_SIZE);
    }
    ~SSLCAN() {
        (void)slcan_destroy(m_Port);
    }
};

CANAPI_Return_t CSerialCAN::MapErrorCode(int code) {
    CANAPI_Return_t retVal = CANERR_NOERROR;

    // note: in case that a slcan function returns -1
    //       the system error variable 'errno' is set
    if (code < 0) {
        switch (errno) {
            case EINVAL:   retVal = CANERR_ILLPARA; break;
            case ENODEV:   retVal = CANERR_HANDLE; break;
            case EBADF:    retVal = CANERR_NOTINIT; break;
            case EALREADY: retVal = CANERR_YETINIT; break;
            default:       retVal = CANERR_VENDOR - errno; break;
        }
    }
    return retVal;
}

EXPORT
CSerialCAN::CSerialCAN() {
    m_szTtyName[0] = '\0';
    m_OpMode.byte = CANMODE_DEFAULT;
    m_Status.byte = CANSTAT_RESET;
    m_Bitrate.index = CANBTR_INDEX_250K;
    m_Bitrate.btr.nominal.brp = 0;
    m_Bitrate.btr.nominal.tseg1 = 0;
    m_Bitrate.btr.nominal.tseg2 = 0;
    m_Bitrate.btr.nominal.sjw = 0;
    m_Bitrate.btr.nominal.sam = 0;
#ifndef OPTION_CAN_2_0_ONLY
    m_Bitrate.btr.data.brp = m_Bitrate.btr.nominal.brp;
    m_Bitrate.btr.data.tseg1 = m_Bitrate.btr.nominal.tseg1;
    m_Bitrate.btr.data.tseg2 = m_Bitrate.btr.nominal.tseg2;
    m_Bitrate.btr.data.sjw = m_Bitrate.btr.nominal.sjw;
#endif
    m_Counter.u64TxMessages = 0U;
    m_Counter.u64RxMessages = 0U;
    m_Counter.u64ErrorFrames = 0U;
    // serial line interface
    m_pSLCAN = new SSLCAN();
}

EXPORT
CSerialCAN::~CSerialCAN() {
    delete m_pSLCAN;
}

EXPORT
CANAPI_Return_t CSerialCAN::ProbeChannel(int32_t channel, CANAPI_OpMode_t opMode, EChannelState &state) {
    // delegate with value NULL for parameter 'param'
    return ProbeChannel(channel, opMode, NULL, state);
}

EXPORT
CANAPI_Return_t CSerialCAN::ProbeChannel(int32_t channel, CANAPI_OpMode_t opMode, const void *param, EChannelState &state) {
    char device[CANPROP_MAX_BUFFER_SIZE];
    SPRINTF_S(device, CANPROP_MAX_BUFFER_SIZE, SERIAL_PORTNAME, channel + 1);
    (void)param;  // TODO: map comm attributes

    // delegate with device name instead of channel number
    return ProbeChannel(device, opMode, state);
}

EXPORT
CANAPI_Return_t CSerialCAN::ProbeChannel(const char *device, CANAPI_OpMode_t opMode, EChannelState &state) {
    SSerialAttributes sioAttr = {};
    sioAttr.baudrate = SERIAL_BAUDRATE;
    sioAttr.bytesize = SERIAL_BYTESIZE;
    sioAttr.parity = SERIAL_PARITY;
    sioAttr.stopbits = SERIAL_STOPBITS;
    sioAttr.options = SERIAL_OPTIONS;
    // delegate with default values for parameter 'sioAttr'
    return ProbeChannel(device, opMode, sioAttr, state);
}

EXPORT
CANAPI_Return_t CSerialCAN::ProbeChannel(const char *device, CANAPI_OpMode_t opMode, SSerialAttributes sioAttr, EChannelState &state) {
    (void)device;
    (void)opMode;
    (void)sioAttr;
    // note: serial devices are not testable (for now)
    state = CSerialCAN::ChannelNotTestable;
    return CSerialCAN::NoError;
}

EXPORT
CANAPI_Return_t CSerialCAN::InitializeChannel(int32_t channel, can_mode_t opMode, const void *param) {
    char device[CANPROP_MAX_BUFFER_SIZE];
    SPRINTF_S(device, CANPROP_MAX_BUFFER_SIZE, SERIAL_PORTNAME, channel + 1);
    (void)param;  // TODO: map comm attributes

    // delegate with device name instead of channel number
    return InitializeChannel(device, opMode);
}

EXPORT
CANAPI_Return_t CSerialCAN::InitializeChannel(const char *device, can_mode_t opMode) {
    SSerialAttributes sioAttr = {};
    sioAttr.baudrate = SERIAL_BAUDRATE;
    sioAttr.bytesize = SERIAL_BYTESIZE;
    sioAttr.parity = SERIAL_PARITY;
    sioAttr.stopbits = SERIAL_STOPBITS;
    sioAttr.options = SERIAL_OPTIONS;
    // delegate with default values for parameter 'sioAttr'
    return InitializeChannel(device, opMode, sioAttr);
}

EXPORT
CANAPI_Return_t CSerialCAN::InitializeChannel(const char *device, can_mode_t opMode, SSerialAttributes sioAttr) {
    CANAPI_Return_t retVal = CSerialCAN::AlreadyInitialized;
    int result = -1;

    (void)sioAttr;  // TODO: comm parameter

    // (§) CAN interface must not be initialized
    if (!m_pSLCAN->m_bInitialized) {
        uint8_t hw_version = 0x00U;
        uint8_t sw_version = 0x00U;
        // (1) check if TTY name is given
        if (device == NULL) {
            retVal = CSerialCAN::IllegalParameter;
            goto error_initialize;
        }
        // (2) check if requested protocol option (SLCAN)
        if ((sioAttr.options & CANSIO_SLCAN) != CANSIO_SLCAN) {
            retVal = CSerialCAN::NotSupported;
            goto error_initialize;
        }
        // (3) check if requested operation mode is supported
        if ((opMode.byte & (uint8_t)(~SUPPORTED_OP_MODE)) != 0) {
            retVal = CSerialCAN::NotSupported;
            goto error_initialize;
        }
        // (4) connect serial interface
        result = slcan_connect(m_pSLCAN->m_Port, device);
        retVal = MapErrorCode(result);
        if (retVal != CSerialCAN::NoError) {
            goto error_initialize;
        }
        // (5) check for SLCAN protocol
        result = slcan_version_number(m_pSLCAN->m_Port, &hw_version, &sw_version);
        retVal = MapErrorCode(result);
        if (retVal != CSerialCAN::NoError) {
            (void)slcan_disconnect(m_pSLCAN->m_Port);
            goto error_initialize;
        }
        // (6) reset CAN controller (it's possibly running)
        (void)slcan_close_channel(m_pSLCAN->m_Port);
        // :-) CAN controller is in INIT state
        m_pSLCAN->m_bInitialized = true;
        m_pSLCAN->m_Attr.baudrate = sioAttr.baudrate;
        m_pSLCAN->m_Attr.bytesize = sioAttr.bytesize;
        m_pSLCAN->m_Attr.parity = sioAttr.parity;
        m_pSLCAN->m_Attr.stopbits = sioAttr.stopbits;
        m_pSLCAN->m_Attr.options = sioAttr.options;
        STRNCPY_S(m_szTtyName, CANPROP_MAX_BUFFER_SIZE, device, CANPROP_MAX_BUFFER_SIZE);
        m_Status.can_stopped = 1;
        m_OpMode = opMode;
    }
error_initialize:
    return retVal;
}

EXPORT
CANAPI_Return_t CSerialCAN::TeardownChannel() {
    CANAPI_Return_t retVal = CSerialCAN::NotInitialized;
    int result = -1;

    // (§) CAN interface must be initialized
    if (m_pSLCAN->m_bInitialized) {
        // (1) set CAN controller into INIT state
        (void)ResetController(); // m_Status.can_stopped := 1
        // (2) disconnect serial interface
        result = slcan_disconnect(m_pSLCAN->m_Port);
        retVal = MapErrorCode(result);
        m_pSLCAN->m_bInitialized = false;  // FIXME: how to handle errors?
    }
    return retVal;
}

EXPORT
CANAPI_Return_t CSerialCAN::SignalChannel() {
    CANAPI_Return_t retVal = CSerialCAN::NotInitialized;
    int result = -1;

    // (§) CAN interface must be initialized
    if (m_pSLCAN->m_bInitialized) {
        // (1) signal all waiting objects
        result = slcan_signal(m_pSLCAN->m_Port);
        retVal = MapErrorCode(result);
    }
    return retVal;
}

EXPORT
CANAPI_Return_t CSerialCAN::StartController(CANAPI_Bitrate_t bitrate) {
    CANAPI_Return_t retVal = CSerialCAN::NotSupported;
    int result = -1;

    // (§) CAN interface must be initialized
    if (m_pSLCAN->m_bInitialized) {
        uint16_t btr0btr1 = 0x011CU;
        CANAPI_Bitrate_t temporary = bitrate;
        // (a) check bit-rate settings (possibly after conversion from index)
        if (m_Bitrate.index <= 0) {
            // convert index to bit-rate
            if (CSerialCAN::MapIndex2Bitrate(m_Bitrate.index, temporary) != CANERR_NOERROR)
                return CANERR_BAUDRATE;  // FIXME: single point of exit
        }
        // (b) convert bit-rate to SJA1000 BTR0/BTR1 register
        if (CSerialCAN::MapBitrate2Sja1000(temporary, btr0btr1) != CANERR_NOERROR)
            return CANERR_BAUDRATE;  // FIXME: single point of exit
        // (§) CAN controller must be in INIT state!
        if (m_Status.can_stopped) {
            // (1) set the bit-timing register
            result = slcan_setup_btr(m_pSLCAN->m_Port, btr0btr1);
            retVal = MapErrorCode(result);
            if (retVal == CSerialCAN::NoError) {
                m_Status.byte = CANSTAT_RESET;
                m_Bitrate = temporary;
                // (2) clear queues, counters, time reference
                m_Counter.u64TxMessages = 0U;
                m_Counter.u64RxMessages = 0U;
                m_Counter.u64ErrorFrames = 0U;
                // (3) start the CAN controller
                result = slcan_open_channel(m_pSLCAN->m_Port);
                retVal = MapErrorCode(result);
                m_Status.can_stopped = (result == CANERR_NOERROR) ? 0 : 1;
            }
        } else
            retVal = CSerialCAN::ControllerOnline;
    }
    return retVal;
}

EXPORT
CANAPI_Return_t CSerialCAN::ResetController() {
    CANAPI_Return_t retVal = CSerialCAN::NotSupported;
    int result = -1;

    // (§) CAN interface must be initialized
    if (m_pSLCAN->m_bInitialized) {
        // (§) CAN controller must not be in INIT state!
        if (!m_Status.can_stopped) {
            result = slcan_close_channel(m_pSLCAN->m_Port);
            retVal = MapErrorCode(result);
            m_Status.can_stopped = (result == CANERR_NOERROR) ? 1 : 0;
        } else
            retVal = CSerialCAN::ControllerOffline;
    }
    return retVal;
}

EXPORT
CANAPI_Return_t CSerialCAN::WriteMessage(CANAPI_Message_t message, uint16_t timeout) {
    CANAPI_Return_t retVal = CSerialCAN::NotSupported;
    int result = -1;

    // (§) CAN interface must be initialized
    if (m_pSLCAN->m_bInitialized) {
        // (a) check identifier range
        if (!message.xtd && (message.id > CAN_MAX_STD_ID))
            return CSerialCAN::IllegalParameter;
        if (message.xtd && (message.id > CAN_MAX_XTD_ID))
            return CSerialCAN::IllegalParameter;
        // (b) check data length code
        if (message.dlc > CAN_MAX_DLC)
            return CSerialCAN::IllegalParameter;
        // (§) CAN controller must not be in INIT state!
        if (!m_Status.can_stopped) {
            slcan_message_t canMsg;
            memset(&canMsg, 0x00, sizeof(slcan_message_t));
            // (1) map message layout
            canMsg.can_id = message.id & (message.xtd ? CAN_XTD_MASK : CAN_STD_MASK);
            canMsg.can_id |= (message.xtd ? CAN_XTD_FRAME : 0x00000000U);
            canMsg.can_id |= (message.rtr ? CAN_RTR_FRAME : 0x00000000U);
            canMsg.can_dlc = message.dlc;
            memcpy(canMsg.data, message.data, canMsg.can_dlc);
            // (2) transmit the CAN message
            result = slcan_write_message(m_pSLCAN->m_Port, &canMsg, timeout);
            (void)MapErrorCode(result);
            // (3) update status and tx counter
            m_Status.transmitter_busy = (result != CANERR_NOERROR) ? 1 : 0;
            m_Counter.u64TxMessages += (result == CANERR_NOERROR) ? 1U : 0U;
            retVal = CSerialCAN::NoError;
        } else
            retVal = CSerialCAN::ControllerOffline;
    }
    return retVal;
}

EXPORT
CANAPI_Return_t CSerialCAN::ReadMessage(CANAPI_Message_t &message, uint16_t timeout) {
    CANAPI_Return_t retVal = CSerialCAN::NotSupported;
    int result = -1;

    // (§) CAN interface must be initialized
    if (m_pSLCAN->m_bInitialized) {
        // (§) CAN controller must not be in INIT state!
        if (!m_Status.can_stopped) {
            slcan_message_t canMsg;
            memset(&message, 0x00, sizeof(CANAPI_Message_t));
            // (1) read one CAN message from message queue, if any
            result = slcan_read_message(m_pSLCAN->m_Port, &canMsg, timeout);
            if (result == CANERR_NOERROR) {
                // (2) map message layout
                message.xtd = (canMsg.can_id & CAN_XTD_FRAME) ? 1 : 0;
                message.sts = (canMsg.can_id & CAN_ERR_FRAME) ? 1 : 0;
                message.rtr = (canMsg.can_id & CAN_RTR_FRAME) ? 1 : 0;
                message.id = canMsg.can_id & (message.xtd ? CAN_XTD_MASK : CAN_STD_MASK);
                message.dlc = (canMsg.can_dlc < CAN_DLC_MAX) ? canMsg.can_dlc : CAN_LEN_MAX;
                memcpy(message.data, canMsg.data, message.dlc);
                // (3) update status and rx counter
                m_Counter.u64RxMessages += !message.sts ? 1U : 0U;
                m_Counter.u64ErrorFrames += message.sts ? 1U : 0U;
                retVal = CSerialCAN::NoError;
            } else if (result != CANERR_RX_EMPTY) {
                retVal = MapErrorCode(result);
            } else
                retVal = CSerialCAN::ReceiverEmpty;
            m_Status.receiver_empty = (result != CANERR_NOERROR) ? 1 : 0;
            m_Status.queue_overrun |= (errno == ENOSPC) ? 1 : 0;
        } else
            retVal = CSerialCAN::ControllerOffline;
    }
    return retVal;
}

EXPORT
CANAPI_Return_t CSerialCAN::GetStatus(CANAPI_Status_t &status) {
    CANAPI_Return_t retVal = CSerialCAN::NotSupported;
    int result = -1;

    // (§) CAN interface must be initialized
    if (m_pSLCAN->m_bInitialized) {
        // (§) CAN controller must not be in INIT state!
        if (!m_Status.can_stopped) {
            slcan_flags_t flags;
            memset(&flags, 0x00, sizeof(slcan_flags_t));
            // (1) read status flags
            result = slcan_status_flags(m_pSLCAN->m_Port, &flags);
            (void)MapErrorCode(result);
            // (2) on success update status register
            if (result == CANERR_NOERROR) {
                // TODO: SJA1000 datasheet, rtfm!
                status.message_lost = (flags.DOI | flags.RxFIFO | flags.TxFIFO) ? 1 : 0;
                status.bus_error = flags.BEI ? 1 : 0;
                status.warning_level = (flags.EI | flags.EPI);
                status.bus_off = flags.ALI;
            }
        }
        status = m_Status;
        retVal = CSerialCAN::NoError;
    }
    return retVal;
}

EXPORT
CANAPI_Return_t CSerialCAN::GetBusLoad(uint8_t &load) {
    CANAPI_Return_t retVal = CSerialCAN::NotSupported;

    // (§) CAN interface must be initialized
    if (m_pSLCAN->m_bInitialized) {
        // TODO: get bus load
        load = 0U;
        retVal = CSerialCAN::NotSupported;
    }
    return retVal;
}

EXPORT
CANAPI_Return_t CSerialCAN::GetBitrate(CANAPI_Bitrate_t &bitrate) {
    CANAPI_Return_t retVal = CSerialCAN::NotSupported;

    // (§) CAN interface must be initialized
    if (m_pSLCAN->m_bInitialized) {
        CANAPI_Bitrate_t temporary = m_Bitrate;
        if (m_Bitrate.index <= 0) {
            // convert index to bit-rate
            if (CSerialCAN::MapIndex2Bitrate(m_Bitrate.index, temporary) != CANERR_NOERROR)
                return CANERR_BAUDRATE;  // FIXME: single point of exit
        }
        bitrate = temporary;
#ifndef OPTION_CANAPI_RETVALS
        retVal = CSerialCAN::NoError;
#else
        // note: CAN API `can_bitrate' returns CANERR_OFFLINE
        //       when the CAN controller has not been started
        if (m_Status.can_stopped)
            retVal = CSerialCAN::ControllerOffline;
        else
            retVal = CSerialCAN::NoError;
#endif
    }
    return retVal;
}

EXPORT
CANAPI_Return_t CSerialCAN::GetBusSpeed(CANAPI_BusSpeed_t &speed) {
    CANAPI_Return_t retVal = CSerialCAN::NotSupported;

    // (§) CAN interface must be initialized
    if (m_pSLCAN->m_bInitialized) {
        CANAPI_Bitrate_t temporary;
        retVal = GetBitrate(temporary);
#ifndef OPTION_CANAPI_RETVALS
        if (retVal == CSerialCAN::NoError) {
#else
        // note: CAN API `can_bitrate' returns CANERR_OFFLINE
        //       when the CAN controller has not been started
        if ((retVal == CSerialCAN::NoError) ||
            (retVal == CSerialCAN::ControllerOffline)) {
#endif
            // note: we got bit-rate settings, not an index, so
            //       we can use the method from the base class
            if (CSerialCAN::MapBitrate2Speed(temporary, speed) != CANERR_NOERROR)
                return CANERR_BAUDRATE;  // FIXME: single point of exit
        }
    }
    return retVal;
}

EXPORT
CANAPI_Return_t CSerialCAN::GetProperty(uint16_t param, void *value, uint32_t nbyte) {
    CANAPI_Return_t retVal = CSerialCAN::IllegalParameter;
    can_sio_param_t sioParam = {};

    if (!value)
        return CSerialCAN::NullPointer;

    switch (param) {
        case SERIALCAN_PROPERTY_CANAPI:
            if ((size_t)nbyte >= sizeof(uint16_t)) {
                *(uint16_t*)value = (uint16_t)CAN_API_SPEC;
                retVal = CSerialCAN::NoError;
            }
            break;
        case SERIALCAN_PROPERTY_VERSION:
            if ((size_t)nbyte >= sizeof(uint16_t)) {
                *(uint16_t*)value = ((uint16_t)VERSION_MAJOR << 8) | (uint16_t)VERSION_MINOR;
                retVal = CSerialCAN::NoError;
            }
            break;
        case SERIALCAN_PROPERTY_PATCH_NO:
            if ((size_t)nbyte >= sizeof(uint8_t)) {
                *(uint8_t*)value = (uint8_t)VERSION_PATCH;
                retVal = CSerialCAN::NoError;
            }
            break;
        case SERIALCAN_PROPERTY_BUILD_NO:
            if ((size_t)nbyte >= sizeof(uint32_t)) {
                *(uint32_t*)value = (uint32_t)VERSION_BUILD;
                retVal = CSerialCAN::NoError;
            }
            break;
        case SERIALCAN_PROPERTY_LIBRARY_ID:
            if ((size_t)nbyte >= sizeof(int32_t)) {
                *(int32_t*)value = (int32_t)SERIALCAN_LIBRARY_ID;
                retVal = CSerialCAN::NoError;
            }
            break;
        case SERIALCAN_PROPERTY_LIBRARY_VENDOR:
            if ((nbyte > strlen(SERIALCAN_LIBRARY_VENDOR)) && (nbyte <= CANPROP_MAX_BUFFER_SIZE)) {
                STRCPY_S((char*)value, nbyte, SERIALCAN_LIBRARY_VENDOR);
                retVal = CSerialCAN::NoError;
            }
            break;
        case SERIALCAN_PROPERTY_LIBRARY_NAME:
            if ((nbyte > strlen(SERIALCAN_LIBRARY_NAME)) && (nbyte <= CANPROP_MAX_BUFFER_SIZE)) {
                STRCPY_S((char*)value, nbyte, SERIALCAN_LIBRARY_NAME);
                retVal = CSerialCAN::NoError;
            }
            break;
        default:
            // (§) CAN interface must be initialized to get these properties
            if (m_pSLCAN->m_bInitialized) {
                switch (param) {
                    case SERIALCAN_PROPERTY_DEVICE_TYPE:
                        if ((size_t)nbyte >= sizeof(int32_t)) {
                            *(int32_t*)value = (int32_t)DEVICE_TYPE;
                            retVal = CSerialCAN::NoError;
                        }
                        break;
                    case SERIALCAN_PROPERTY_DEVICE_NAME:
                        if ((nbyte > strlen(m_szTtyName)) && (nbyte <= CANPROP_MAX_BUFFER_SIZE)) {
                            STRCPY_S((char*)value, nbyte, m_szTtyName);
                            retVal = CSerialCAN::NoError;
                        }
                        break;
                    case SERIALCAN_PROPERTY_DEVICE_VENDOR:
                        if ((nbyte > strlen(VENDOR_NAME)) && (nbyte <= CANPROP_MAX_BUFFER_SIZE)) {
                            STRCPY_S((char*)value, nbyte, VENDOR_NAME);
                            retVal = CSerialCAN::NoError;
                        }
                        break;
                    case SERIALCAN_PROPERTY_DEVICE_DLLNAME:
                        if ((nbyte > strlen(VENDOR_DLLNAME)) && (nbyte <= CANPROP_MAX_BUFFER_SIZE)) {
                            STRCPY_S((char*)value, nbyte, VENDOR_DLLNAME);
                            retVal = CSerialCAN::NoError;
                        }
                        break;
                    case SERIALCAN_PROPERTY_DEVICE_PARAM:
                        if ((size_t)nbyte >= sizeof(can_sio_param_t)) {
                            sioParam.name = m_szTtyName;  // FIXME: this is insecure
                            sioParam.attr.baudrate = m_pSLCAN->m_Attr.baudrate;
                            sioParam.attr.bytesize = m_pSLCAN->m_Attr.bytesize;
                            sioParam.attr.parity = m_pSLCAN->m_Attr.parity;
                            sioParam.attr.stopbits = m_pSLCAN->m_Attr.stopbits;
                            sioParam.attr.options = m_pSLCAN->m_Attr.options;
                            memcpy(value, &sioParam, sizeof(can_sio_param_t));
                            retVal = CSerialCAN::NoError;
                        }
                        break;
                    case SERIALCAN_PROPERTY_OP_CAPABILITY:
                        if ((size_t)nbyte >= sizeof(uint8_t)) {
                            *(uint8_t*)value = (uint8_t)SUPPORTED_OP_MODE;
                            retVal = CSerialCAN::NoError;
                        }
                        break;
                    case SERIALCAN_PROPERTY_OP_MODE:
                        if ((size_t)nbyte >= sizeof(uint8_t)) {
                            *(uint8_t*)value = (uint8_t)m_OpMode.byte;
                            retVal = CSerialCAN::NoError;
                        }
                        break;
                    case SERIALCAN_PROPERTY_BITRATE:
                        CANAPI_Bitrate_t bitrate;
                        if ((retVal = GetBitrate(bitrate)) == CANERR_NOERROR) {
                            if (nbyte >= sizeof(CANAPI_Bitrate_t)) {
                                memcpy(value, &bitrate, sizeof(CANAPI_Bitrate_t));
                                retVal = CSerialCAN::NoError;
                            }
                        }
                        break;
                    case SERIALCAN_PROPERTY_SPEED:
                        CANAPI_BusSpeed_t speed;
                        if ((retVal = GetBusSpeed(speed)) == CANERR_NOERROR) {
                            if (nbyte >= sizeof(CANAPI_BusSpeed_t)) {
                                memcpy(value, &speed, sizeof(CANAPI_BusSpeed_t));
                                retVal = CSerialCAN::NoError;
                            }
                        }
                        break;
                    case SERIALCAN_PROPERTY_STATUS:
                        CANAPI_Status_t status;
                        if ((retVal = GetStatus(status)) == CANERR_NOERROR) {
                            if ((size_t)nbyte >= sizeof(uint8_t)) {
                                *(uint8_t*)value = (uint8_t)status.byte;
                                retVal = CSerialCAN::NoError;
                            }
                        }
                        break;
                    case SERIALCAN_PROPERTY_BUSLOAD:
                        uint8_t load;
                        if ((retVal = GetBusLoad(load)) == CANERR_NOERROR) {
                            if ((size_t)nbyte >= sizeof(uint8_t)) {
                                *(uint8_t*)value = (uint8_t)load;
                                retVal = CSerialCAN::NoError;
                            }
                        }
                        break;
                    case SERIALCAN_PROPERTY_TX_COUNTER:
                        if ((size_t)nbyte >= sizeof(uint64_t)) {
                            *(uint64_t*)value = (uint64_t)m_Counter.u64TxMessages;
                            retVal = CSerialCAN::NoError;
                        }
                        break;
                    case SERIALCAN_PROPERTY_RX_COUNTER:
                        if ((size_t)nbyte >= sizeof(uint64_t)) {
                            *(uint64_t*)value = (uint64_t)m_Counter.u64RxMessages;
                            retVal = CSerialCAN::NoError;
                        }
                        break;
                    case SERIALCAN_PROPERTY_ERR_COUNTER:
                        if ((size_t)nbyte >= sizeof(uint64_t)) {
                            *(uint64_t*)value = (uint64_t)m_Counter.u64ErrorFrames;
                            retVal = CSerialCAN::NoError;
                        }
                        break;
                    case SERIALCAN_PROPERTY_CLOCK_DOMAIN:
                        if ((size_t)nbyte >= sizeof(int32_t)) {
                            *(int32_t*)value = (int32_t)CLOCK_DOMAIN;
                            retVal = CSerialCAN::NoError;
                        }
                        break;
                    case SERIALCAN_PROPERTY_HARDWARE_VERSION:
                        uint8_t hw_version;
                        if ((retVal = slcan_version_number(m_pSLCAN->m_Port, &hw_version, NULL)) == CANERR_NOERROR) {
                            if ((size_t)nbyte >= sizeof(uint8_t)) {
                                *(uint8_t*)value = (uint8_t)hw_version;
                                retVal = CSerialCAN::NoError;
                            }
                        }
                        break;
                    case SERIALCAN_PROPERTY_SOFTWARE_VERSION:
                        uint8_t sw_version;
                        if ((retVal = slcan_version_number(m_pSLCAN->m_Port, NULL, &sw_version)) == CANERR_NOERROR) {
                            if ((size_t)nbyte >= sizeof(uint8_t)) {
                                *(uint8_t*)value = (uint8_t)sw_version;
                                retVal = CSerialCAN::NoError;
                            }
                        }
                        break;
                    case SERIALCAN_PROPERTY_SERIAL_NUMBER:
                        uint32_t serial_no;
                        if ((retVal = slcan_serial_number(m_pSLCAN->m_Port, &serial_no)) == CANERR_NOERROR) {
                            if ((size_t)nbyte >= sizeof(uint32_t)) {
                                *(uint32_t*)value = (uint32_t)serial_no;
                                retVal = CSerialCAN::NoError;
                            }
                        }
                        break;
                    default:
                        retVal = CSerialCAN::NotSupported;
                        break;
                }
            } else {
                retVal = CSerialCAN::NotInitialized;
            }
            break;
    }
    return retVal;
}

EXPORT
CANAPI_Return_t CSerialCAN::SetProperty(uint16_t param, const void *value, uint32_t nbyte) {
    CANAPI_Return_t retVal = CSerialCAN::IllegalParameter;

    if (!value)
        return CSerialCAN::NullPointer;

    // TODO: insert coin here
    (void)param;
    (void)nbyte;

    return retVal;
}

EXPORT
char *CSerialCAN::GetHardwareVersion() {
    static char hardware[CANPROP_MAX_BUFFER_SIZE+33] = "";
    int result = -1;

    // (§) CAN interface must be initialized
    if (m_pSLCAN->m_bInitialized) {
        uint8_t hw_version = 0x00U;
        // get version number: HW and SW
        result = slcan_version_number(m_pSLCAN->m_Port, &hw_version, NULL);
        if (result == CANERR_NOERROR) {
            // note: m_szTtyName has at worst 255 characters plus terminating zero
            snprintf(hardware, CANPROP_MAX_BUFFER_SIZE+33, "Hardware %u.%u (%s)",
                     (hw_version >> 4), (hw_version & 0xFU), m_szTtyName);
            hardware[CANPROP_MAX_BUFFER_SIZE-1] = '\0';  // to be safe
            return (char*)hardware;
        }
    }
    return NULL;
}

EXPORT
char *CSerialCAN::GetFirmwareVersion() {
    static char software[CANPROP_MAX_BUFFER_SIZE] = "";
    int result = -1;

    // (§) CAN interface must be initialized
    if (m_pSLCAN->m_bInitialized) {
        uint8_t sw_version = 0x00U;
        // get version number: HW and SW
        result = slcan_version_number(m_pSLCAN->m_Port, NULL, &sw_version);
        if (result == CANERR_NOERROR) {
            SPRINTF_S(software, CANPROP_MAX_BUFFER_SIZE, "Software %u.%u (SLCAN protocol)",
                (sw_version >> 4), (sw_version & 0xFU));
            software[CANPROP_MAX_BUFFER_SIZE-1] = '\0';  // to be safe
            return (char*)software;
        }
    }
    return NULL;
}

EXPORT
char *CSerialCAN::GetVersion() {
    // get driver version
    return (char *)&version[0];
}

//  Methods for bit-rate conversion
//
EXPORT
CANAPI_Return_t CSerialCAN::MapIndex2Bitrate(int32_t index, CANAPI_Bitrate_t &bitrate) {
    return (CANAPI_Return_t)btr_index2bitrate(index, &bitrate);
}

EXPORT
CANAPI_Return_t CSerialCAN::MapString2Bitrate(const char *string, CANAPI_Bitrate_t &bitrate) {
    bool brse = false;
    // TODO: rework function 'btr_string2bitrate'
    return (CANAPI_Return_t)btr_string2bitrate((btr_string_t)string, &bitrate, &brse);
}

EXPORT
CANAPI_Return_t CSerialCAN::MapBitrate2String(CANAPI_Bitrate_t bitrate, char *string, size_t length) {
    (void) length;
    // TODO: rework function 'btr_bitrate2string'
    return (CANAPI_Return_t)btr_bitrate2string(&bitrate, false, (btr_string_t)string);
}

EXPORT
CANAPI_Return_t CSerialCAN::MapBitrate2Speed(CANAPI_Bitrate_t bitrate, CANAPI_BusSpeed_t &speed) {
    // TODO: rework function 'btr_bitrate2speed'
    return (CANAPI_Return_t)btr_bitrate2speed(&bitrate, false, false, &speed);
}

CANAPI_Return_t CSerialCAN::MapBitrate2Sja1000(CANAPI_Bitrate_t bitrate, uint16_t &btr0btr1) {
    return (CANAPI_Return_t)btr_bitrate2sja1000(&bitrate, &btr0btr1);
}

CANAPI_Return_t CSerialCAN::MapSja10002Bitrate(uint16_t btr0btr1, CANAPI_Bitrate_t &bitrate) {
    return (CANAPI_Return_t)btr_sja10002bitrate(btr0btr1, &bitrate);
}
