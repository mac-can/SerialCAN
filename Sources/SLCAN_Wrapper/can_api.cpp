//
//  CAN Interface API, Version 3 (for CAN-over-Serial-Line Interfaces)
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
//  along with SerialCAN.  If not, see <http://www.gnu.org/licenses/>.
//
#include "can_api.h"

#include "CANAPI.h"
#include "SerialCAN.h"

#include <stdio.h>
#include <string.h>
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
#ifdef _DEBUG
    static const char version[] = "CAN API V3 for CAN-over-Serial-Line Interfaces, Version " VERSION_STRING " (" PLATFORM ") _DEBUG";
#else
    static const char version[] = "CAN API V3 for CAN-over-Serial-Line Interfaces, Version " VERSION_STRING " (" PLATFORM ")";
#endif

#if (OPTION_CANAPI_SERIALCAN_DYLIB != 0)
__attribute__((constructor))
static void _initializer() {
#if (OPTION_CANAPI_DEBUG_LEVEL != 0)
    fprintf(stdout, "[%s] [%s]\n", __FILE__, __FUNCTION__);
#endif
}
__attribute__((destructor))
static void _finalizer() {
#if (OPTION_CANAPI_DEBUG_LEVEL != 0)
    fprintf(stdout, "[%s] [%s]\n", __FILE__, __FUNCTION__);
#endif
}
#define EXPORT  __attribute__((visibility("default")))
#else
#define EXPORT
#endif

#ifndef CAN_MAX_HANDLES
#define CAN_MAX_HANDLES  16
#endif
#define INVALID_HANDLE  (-1)

#define IS_HANDLE_VALID(hnd)  ((0 <= (hnd)) && ((hnd) < CAN_MAX_HANDLES))

 EXPORT
 can_board_t can_boards[1] = {  // list of supported CAN Interfaces
     // TODO: rework this (either by an own can_defs.h or by a json file)
     {EOF, NULL}
 };
static CSerialCAN canDevices[CAN_MAX_HANDLES];  // list of SerialCAN device driver
static bool canHandles[CAN_MAX_HANDLES];
static bool init = false;  // initialization flag

EXPORT
int can_test(int32_t channel, uint8_t mode, const void *param, int *result)
{
    CCANAPI::EChannelState state = CCANAPI::ChannelNotTestable;
    CANAPI_Return_t retVal = CANERR_FATAL;
    CANAPI_OpMode_t opMode = {};
    opMode.byte = mode;

    // probe if the CAN interface (hardware and driver) is present
    retVal = CSerialCAN::ProbeChannel(channel, opMode, state);
    if (result) {
        switch (state) {
        case CCANAPI::ChannelOccupied:
            *result = CANBRD_OCCUPIED;
            break;
        case CCANAPI::ChannelAvailable:
            *result = CANBRD_PRESENT;
            break;
        case CCANAPI::ChannelNotAvailable:
            *result = CANBRD_NOT_PRESENT;
            break;
        default:
            *result = CANBRD_NOT_TESTABLE;
            break;
        }
    }
#if (OPTION_CANAPI_DEBUG_LEVEL != 0)
    fprintf(stdout, "CCANAPI::ProbeChannel(%i): returned %i\n", channel, retVal);
#endif
    (void) param;
    return (int)retVal;
}

EXPORT
int can_init(int32_t channel, uint8_t mode, const void *param)
{
    int result = CANERR_HANDLE;
    CANAPI_Handle_t handle = INVALID_HANDLE;
    CANAPI_Return_t retVal = CANERR_FATAL;
    CANAPI_OpMode_t opMode = {};
    opMode.byte = mode;

    if (!init) {
        for (int i = 0; i < CAN_MAX_HANDLES; i++)
            canHandles[i] = false;
        init = 1;
    }
    // sanity checks
    if (!init)
        return CANERR_FATAL;
    if (!param)
        return CANERR_NULLPTR;
    if (channel != CANDEV_SERIAL)
        return CANERR_ILLPARA;

    // search a free handle
    for (handle = 0; handle < CAN_MAX_HANDLES; handle++)
        if (!canHandles[handle])
            break;
    if (!IS_HANDLE_VALID(handle))
        return CANERR_HANDLE;

    // initialize the CAN interface (hardware and driver)
    can_sio_param_t *device = (can_sio_param_t*)param;
    retVal = canDevices[handle].InitializeChannel(device->name, opMode, device->attr);
    if (retVal >= CANERR_NOERROR) {
        result = (int)handle;  // on success return the handle (>= 0)
        canHandles[handle] = true;
    } else
        result = (int)retVal;  // on error return the error code (< 0)
#if (OPTION_CANAPI_DEBUG_LEVEL != 0)
    fprintf(stdout, "CCANAPI[%i].InitializeChannel(%s): returned %i\n", channel, device->name, retVal);
#endif
    return result;
}

EXPORT
int can_exit(int handle)
{
    CANAPI_Return_t retVal = CANERR_FATAL;

    // sanity checks
    if (!init)
        return CANERR_NOTINIT;
    if (handle != CANEXIT_ALL) {
        if (!IS_HANDLE_VALID(handle))
            return CANERR_HANDLE;

        // stop any operation of the CAN interface
        retVal = canDevices[handle].TeardownChannel();
        canHandles[handle] = false;
    } else {
        for (int i = 0; i < CAN_MAX_HANDLES; i++) {
            (void)canDevices[i].TeardownChannel();
            canHandles[i] = false;
        }
        // ignore induvidual error code
        retVal = CANERR_NOERROR;
    }
#if (OPTION_CANAPI_DEBUG_LEVEL != 0)
    fprintf(stdout, "CCANAPI[%i].TeardownChannel: returned %i\n", handle, retVal);
#endif
    return (int)retVal;
}

EXPORT
int can_kill(int handle)
{
    CANAPI_Return_t retVal = CANERR_FATAL;

    // sanity checks
    if (!init)
        return CANERR_NOTINIT;
    if (handle != CANEXIT_ALL) {
        if (!IS_HANDLE_VALID(handle))
            return CANERR_HANDLE;

        // signal waiting objects, if any
        retVal = canDevices[handle].SignalChannel();
    } else {
        for (int i = 0; i < CAN_MAX_HANDLES; i++)
            if (canHandles[i])
                (void)canDevices[i].SignalChannel();
        // ignore induvidual error code
        retVal = CANERR_NOERROR;
    }
#if (OPTION_CANAPI_DEBUG_LEVEL != 0)
    fprintf(stdout, "CCANAPI[%i].SignalChannel: returned %i\n", handle, retVal);
#endif
    return (int)retVal;
}

EXPORT
int can_start(int handle, const can_bitrate_t *bitrate)
{
    CANAPI_Return_t retVal = CANERR_FATAL;

    // sanity checks
    if (!init)
        return CANERR_NOTINIT;
    if (!IS_HANDLE_VALID(handle))
        return CANERR_HANDLE;
    if (!bitrate)
        return CANERR_NULLPTR;

    // start CAN communication (operation maode and bit-rate settings)
    retVal = canDevices[handle].StartController(*bitrate);
#if (OPTION_CANAPI_DEBUG_LEVEL != 0)
    fprintf(stdout, "CCANAPI[%i].StartController: returned %i\n", handle, retVal);
#endif
    return (int)retVal;
}

EXPORT
int can_reset(int handle)
{
    CANAPI_Return_t retVal = CANERR_FATAL;

    // sanity checks
    if (!init)
        return CANERR_NOTINIT;
    if (!IS_HANDLE_VALID(handle))
        return CANERR_HANDLE;

    // pause CAN communication
    retVal = canDevices[handle].ResetController();
#if (OPTION_CANAPI_DEBUG_LEVEL != 0)
    fprintf(stdout, "CCANAPI[%i].ResetController: returned %i\n", handle, retVal);
#endif
    return (int)retVal;
}

EXPORT
int can_write(int handle, const can_message_t *message, uint16_t timeout)
{
    CANAPI_Return_t retVal = CANERR_FATAL;

    // sanity checks
    if (!init)
        return CANERR_NOTINIT;
    if (!IS_HANDLE_VALID(handle))
        return CANERR_HANDLE;
    if (!message)
        return CANERR_NULLPTR;

    // send a CAN message
    retVal = canDevices[handle].WriteMessage(*message, timeout);
#if (OPTION_CANAPI_DEBUG_LEVEL != 0)
    fprintf(stdout, "CCANAPI[%i].WriteMessage: returned %i\n", handle, retVal);
#endif
    return (int)retVal;
}

EXPORT
int can_read(int handle, can_message_t *message, uint16_t timeout)
{
    CANAPI_Return_t retVal = CANERR_FATAL;

    // sanity checks
    if (!init)
        return CANERR_NOTINIT;
    if (!IS_HANDLE_VALID(handle))
        return CANERR_HANDLE;
    if (!message)
        return CANERR_NULLPTR;

    // read a CAN message from reception queue, if any
    retVal = canDevices[handle].ReadMessage(*message, timeout);
#if (OPTION_CANAPI_DEBUG_LEVEL != 0)
    if (retVal != CCANAPI::ReceiverEmpty)
        fprintf(stdout, "CCANAPI[%i].ReadMessage: returned %i\n", handle, retVal);
#endif
    return (int)retVal;
}

EXPORT
int can_status(int handle, uint8_t *status)
{
    CANAPI_Return_t retVal = CANERR_FATAL;
    CANAPI_Status_t tmpStatus;

    // sanity checks
    if (!init)
        return CANERR_NOTINIT;
    if (!IS_HANDLE_VALID(handle))
        return CANERR_HANDLE;

    // get status register
    retVal = canDevices[handle].GetStatus(tmpStatus);
    if (status)
        *status = tmpStatus.byte;
#if (OPTION_CANAPI_DEBUG_LEVEL != 0)
    fprintf(stdout, "CCANAPI[%i].GetStatus: returned %i\n", handle, retVal);
#endif
    return (int)retVal;
}

EXPORT
int can_busload(int handle, uint8_t *load, uint8_t *status)
{
    CANAPI_Return_t retVal1 = CANERR_FATAL;
    CANAPI_Return_t retVal2 = CANERR_FATAL;
    CANAPI_Status_t tmpStatus;
    uint8_t tmpLoad;

    // sanity checks
    if (!init)
        return CANERR_NOTINIT;
    if (!IS_HANDLE_VALID(handle))
        return CANERR_HANDLE;

    // get bus-load
    retVal1 = canDevices[handle].GetBusLoad(tmpLoad);
    if (load && (retVal1 == CANERR_NOERROR))
        *load = tmpLoad;
#if (OPTION_CANAPI_DEBUG_LEVEL != 0)
    fprintf(stdout, "CCANAPI[%i].GetBusLoad: returned %i\n", handle, retVal1);
#endif
    // and status register
    retVal2 = canDevices[handle].GetStatus(tmpStatus);
    if (status && (retVal2 == CANERR_NOERROR))
        *status = tmpStatus.byte;
#if (OPTION_CANAPI_DEBUG_LEVEL != 0)
    fprintf(stdout, "CCANAPI[%i].GetStatus: returned %i\n", handle, retVal2);
#endif
    return (retVal1 != CANERR_NOERROR) ? (int)retVal1 : (int)retVal2;
}

EXPORT
int can_bitrate(int handle, can_bitrate_t *bitrate, can_speed_t *speed)
{
    CANAPI_Return_t retVal1 = CANERR_FATAL;
    CANAPI_Return_t retVal2 = CANERR_FATAL;
    CANAPI_Bitrate_t tmpBitrate;
    CANAPI_BusSpeed_t tmpSpeed;

    // sanity checks
    if (!init)
        return CANERR_NOTINIT;
    if (!IS_HANDLE_VALID(handle))
        return CANERR_HANDLE;

    // get bit-rate settings
    retVal1 = canDevices[handle].GetBitrate(tmpBitrate);
    if (bitrate && (retVal1 == CANERR_NOERROR))
        *bitrate = tmpBitrate;
#if (OPTION_CANAPI_DEBUG_LEVEL != 0)
    fprintf(stdout, "CCANAPI[%i].GetBitrate: returned %i\n", handle, retVal1);
#endif
    // and transmission rate
    retVal2 = canDevices[handle].GetBusSpeed(tmpSpeed);
    if (speed && (retVal2 == CANERR_NOERROR))
        *speed = tmpSpeed;
#if (OPTION_CANAPI_DEBUG_LEVEL != 0)
    fprintf(stdout, "CCANAPI[%i].GetBusSpeed: returned %i\n", handle, retVal2);
#endif
    return (retVal1 != CANERR_NOERROR) ? (int)retVal1 : (int)retVal2;
}

EXPORT
int can_property(int handle, uint16_t param, void *value, uint32_t nbytes)
{
    CANAPI_Return_t retVal = CANERR_FATAL;

    // sanity checks
    if (!init)
        return CANERR_NOTINIT;
    if (!IS_HANDLE_VALID(handle))
        return CANERR_HANDLE;

    // read a property value
    retVal = canDevices[handle].GetProperty(param, value, nbytes);
#if (OPTION_CANAPI_DEBUG_LEVEL != 0)
    fprintf(stdout, "CCANAPI[%i].GetProperty: returned %i\n", handle, retVal);
#endif
    // TODO: Hmm, what's about SetProperty? (Lucky me, there's nothing to be set)

    return (int)retVal;
}

EXPORT
char *can_hardware(int handle)
{
    // sanity checks
    if (!init)
        return NULL;
    if (!IS_HANDLE_VALID(handle))
        return NULL;

    // get hardware version (zero-terminated string)
    return (char *)canDevices[handle].GetHardwareVersion();
}

EXPORT
char *can_software(int handle)
{
    // sanity checks
    if (!init)
        return NULL;
    if (!IS_HANDLE_VALID(handle))
        return NULL;

    // get software version (zero-terminated string)
    return (char *)canDevices[handle].GetFirmwareVersion();
}

EXPORT
char *can_version()
{
    // get module version (zero-terminated string)
    return (char *)version;
}
