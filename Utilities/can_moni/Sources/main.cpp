//  SPDX-License-Identifier: GPL-3.0-or-later
//
//  CAN Monitor for generic Interfaces (CAN API V3)
//
//  Copyright (c) 2007,2012-2024 Uwe Vogt, UV Software, Berlin (info@uv-software.com)
//
//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program.  If not, see <https://www.gnu.org/licenses/>.
//
#include "Driver.h"
#include "Options.h"
#include "Message.h"
#include "Timer.h"
#if (SERIAL_CAN_SUPPORTED != 0)
#include "SerialCAN_Defines.h"
#endif
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <signal.h>
#include <errno.h>
#include <time.h>

#include <inttypes.h>

#if defined(_WIN64)
#define PLATFORM  "x64"
#elif defined(_WIN32)
#define PLATFORM  "x86"
#elif defined(__linux__)
#define PLATFORM  "Linux"
#elif defined(__APPLE__)
#define PLATFORM  "macOS"
#elif defined(__CYGWIN__)
#define PLATFORM  "Cygwin"
#else
#error Platform not supported
#endif
#ifdef _MSC_VER
//not #if defined(_WIN32) || defined(_WIN64) because we have strncasecmp in mingw
#define strncasecmp _strnicmp
#define strcasecmp _stricmp
#endif

#define MAX_ID  (CAN_MAX_STD_ID + 1)

static int get_exclusion(const char* arg);

class CCanDevice : public CCanDriver {
public:
    uint64_t ReceptionLoop();
public:
    int ListCanDevices(void);
    int TestCanDevices(CANAPI_OpMode_t opMode);
    int ListCanBitrates(CANAPI_OpMode_t opMode);
#if (OPTION_CANAPI_LIBRARY == 0)
    bool WriteJsonFile(const char* filename);
#else
    bool IsBlacklisted(int32_t library, int32_t blacklist[]);
#endif
};
static void sigterm(int signo);

static volatile int running = 1;
static int can_id[MAX_ID];
static int can_id_xtd = 1;

static CCanDevice canDevice = CCanDevice();  // global due to SignalChannel() in sigterm()

int main(int argc, const char* argv[]) {
    CCanDevice::SChannelInfo channel = { (-1), "", "", (-1), "" };
#if (OPTION_CANAPI_LIBRARY != 0)
    CCanDevice::SLibraryInfo library = { (-1), "", "" };
#endif
    CANAPI_Return_t retVal = CANERR_FATAL;
    char property[CANPROP_MAX_BUFFER_SIZE + 1] = "";
    char* string = NULL;

    /* device parameter */
    void* devParam = NULL;
#if (SERIAL_CAN_SUPPORTED != 0)
    /* - CAN-over-Serial-Line (SLCAN protocol) */
    can_sio_param_t sioParam;
    sioParam.name = NULL;
    sioParam.attr.options = CANSIO_SLCAN;
    sioParam.attr.baudrate = CANSIO_BD57600;
    sioParam.attr.bytesize = CANSIO_8DATABITS;
    sioParam.attr.parity = CANSIO_NOPARITY;
    sioParam.attr.stopbits = CANSIO_1STOPBIT;
#endif
    /* exclude list (11-bit IDs only) */
    for (int i = 0; i < MAX_ID; i++) {
        can_id[i] = 1;
    }
    /* signal handler */
    if ((signal(SIGINT, sigterm) == SIG_ERR) ||
#if !defined(_WIN32) && !defined(_WIN64)
       (signal(SIGHUP, sigterm) == SIG_ERR) ||
#endif
       (signal(SIGTERM, sigterm) == SIG_ERR)) {
        perror("+++ error");
        return errno;
    }
    /* scan command-line */
    SOptions opts = SOptions();
    if (opts.ScanCommanline(argc, argv) != 0) {
        /* program usage already shown */
        return 1;
    }
    /* CAN Monitor for generic CAN interfaces */
    opts.ShowGreetings(stdout);
#if (OPTION_CANAPI_LIBRARY != 0)
    /* - set search path for JSON files (if set) */
    if (opts.m_szSearchPath) {
        if (!CCanDriver::SetSearchPath(opts.m_szSearchPath)) {
            fprintf(stderr, "+++ error: search path for JSON files could not be set\n");
            return 1;
        }
        if (opts.m_fVerbose)
            fprintf(stdout, "Pathname=%s\n", opts.m_szSearchPath);
    }
#else
    /* - write library info and device list into JSON file */
    if (opts.m_szJsonFilename) {
        if (!canDevice.WriteJsonFile(opts.m_szJsonFilename)) {
            fprintf(stderr, "+++ error: JSON file could not be written\n");
            return 1;
        }
        if (opts.m_fVerbose)
            fprintf(stdout, "JsonFile=%s\n", opts.m_szJsonFilename);

    }
#endif
    /* - list all supported devices (optional) */
    if (opts.m_fListBoards) {
        int n = canDevice.ListCanDevices();
        fprintf(stdout, "Number of supported CAN interfaces: %i\n", n);
    }
    /* - list all present devices (optional) */
    if (opts.m_fTestBoards) {
        int n = canDevice.TestCanDevices(opts.m_OpMode);
        fprintf(stdout, "Number of present CAN interfaces: %i\n", n);
    }
    /* - list bit-rate settings (optional) */
    if (opts.m_fListBitrates) {
        (void)canDevice.ListCanBitrates(opts.m_OpMode);
    }
    /* - exit if no interface is given */
    if (opts.m_fExit) {
        return 0;
    }
    /* - update exclude list (if set) */
    if (opts.m_szExcludeList) {
        if (!get_exclusion(opts.m_szExcludeList)) {
            fprintf(stderr, "+++ error: %s could not be parsed\n", opts.m_szExcludeList);
            return 1;
        }
    }
    /* - show operation mode, bit-rate settings and acceptance filter (if set) */
    if (opts.m_fVerbose) {
        /* -- operation mode */
        fprintf(stdout, "Op.-mode=%s", (opts.m_OpMode.byte & CANMODE_FDOE) ? "CANFD" : "CAN2.0");
        if ((opts.m_OpMode.byte & CANMODE_BRSE)) fprintf(stdout, "+BRS");
        if ((opts.m_OpMode.byte & CANMODE_NISO)) fprintf(stdout, "+NISO");
        if ((opts.m_OpMode.byte & CANMODE_SHRD)) fprintf(stdout, "+SHRD");
        if ((opts.m_OpMode.byte & CANMODE_NXTD)) fprintf(stdout, "+NXTD");
        if ((opts.m_OpMode.byte & CANMODE_NRTR)) fprintf(stdout, "+NRTR");
        if ((opts.m_OpMode.byte & CANMODE_ERR)) fprintf(stdout, "+ERR");
        if ((opts.m_OpMode.byte & CANMODE_MON)) fprintf(stdout, "+MON");
        fprintf(stdout, " (op_mode=%02Xh)\n", opts.m_OpMode.byte);
        /* -- bit-rate settings */
        if (opts.m_Bitrate.btr.frequency > 0) {
            fprintf(stdout, "Bit-rate=%.0fkbps@%.1f%%", opts.m_BusSpeed.nominal.speed / 1000., opts.m_BusSpeed.nominal.samplepoint * 100.);
#if (CAN_FD_SUPPORTED != 0)
            if (opts.m_OpMode.byte & CANMODE_BRSE)
                fprintf(stdout, ":%.0fkbps@%.1f%%", opts.m_BusSpeed.data.speed / 1000., opts.m_BusSpeed.data.samplepoint * 100.);
#endif
            (void)CCanDevice::MapBitrate2String(opts.m_Bitrate, property, CANPROP_MAX_BUFFER_SIZE,
                                                (opts.m_OpMode.byte & CANMODE_BRSE), opts.m_bHasNoSamp);
            fprintf(stdout, " (%s)\n", property);
        }
        else {
            fprintf(stdout, "Baudrate=%.0fkbps@%.1f%% (index %i)\n",
                             opts.m_BusSpeed.nominal.speed / 1000.,
                             opts.m_BusSpeed.nominal.samplepoint * 100., -opts.m_Bitrate.index);
        }
        /* -- acceptance filter (if set) */
        if ((opts.m_StdFilter.m_u32Code != CANACC_CODE_11BIT) || (opts.m_StdFilter.m_u32Mask != CANACC_MASK_11BIT))
            fprintf(stdout, "Acc.-filter 11-bit=set (code=%03Xh, mask=%03Xh)\n", opts.m_StdFilter.m_u32Code, opts.m_StdFilter.m_u32Mask);
        if (((opts.m_XtdFilter.m_u32Code != CANACC_CODE_29BIT) || (opts.m_XtdFilter.m_u32Mask != CANACC_MASK_29BIT)) && !opts.m_OpMode.nxtd)
            fprintf(stdout, "Acc.-filter 29-bit=set (code=%08Xh, mask=%08Xh)\n", opts.m_XtdFilter.m_u32Code, opts.m_XtdFilter.m_u32Mask);
        fputc('\n', stdout);
    }
    /* - search the <interface> by its name in the device list */
    bool flagFound = false;
#if (OPTION_CANAPI_LIBRARY != 0)
    bool iterLibrary = CCanDevice::GetFirstLibrary(library);
    while (iterLibrary && !flagFound) {
        bool iterChannel = CCanDevice::GetFirstChannel(library.m_nLibraryId, channel);
        while (iterChannel) {
            if (strcasecmp(opts.m_szInterface, channel.m_szDeviceName) == 0) {
                flagFound = true;
                break;
            }
            iterChannel = CCanDevice::GetNextChannel(channel);
        }
        iterLibrary = CCanDevice::GetNextLibrary(library);
    }
#else
#if (SERIAL_CAN_SUPPORTED == 0)
    /* -- loop over build-in device list (old 'can_boards[]') */
    bool iterChannel = CCanDevice::GetFirstChannel(channel);
    while (iterChannel) {
        if (strcasecmp(opts.m_szInterface, channel.m_szDeviceName) == 0) {
            flagFound = true;
            break;
        }
        iterChannel = CCanDevice::GetNextChannel(channel);
    }
#else
    /* -- note: SerialCAN has no build-in device list (fake it) */
    channel.m_nLibraryId = CANLIB_SERIALCAN;
    channel.m_nChannelNo = CANDEV_SERIAL;
    flagFound = true;
#endif
#endif
    if (!flagFound) {
        fprintf(stderr, "+++ error: %s could not be found\n", opts.m_szInterface);
        return 1;
    }
#if (SERIAL_CAN_SUPPORTED != 0)
    /* - CAN-over-Serial-Line (SLCAN protocol) */
    if (channel.m_nLibraryId == CANLIB_SERIALCAN) {
        channel.m_nChannelNo = CANDEV_SERIAL;  // note: override channel number from JSON file
        sioParam.name = opts.m_szInterface;
        devParam = (void*)&sioParam;
    }
#endif
    /* - initialize interface */
    fprintf(stdout, "Hardware=%s...", opts.m_szInterface);
    fflush (stdout);
#if (OPTION_CANAPI_LIBRARY != 0)
    retVal = canDevice.InitializeChannel(channel.m_nLibraryId, channel.m_nChannelNo, opts.m_OpMode, devParam);
#else
    retVal = canDevice.InitializeChannel(channel.m_nChannelNo, opts.m_OpMode, devParam);
#endif
    if (retVal != CCanApi::NoError) {
        fprintf(stdout, "FAILED!\n");
        fprintf(stderr, "+++ error: CAN Controller could not be initialized (%i)", retVal);
        if (retVal == CCanApi::IllegalParameter)
            fprintf(stderr, "\n           - possibly CAN operating mode %02Xh not supported", opts.m_OpMode.byte);
        fputc('\n', stderr);
        goto farewell;
    }
    /* -- set acceptance filter for 11-bit IDs */
    if ((opts.m_StdFilter.m_u32Code != CANACC_CODE_11BIT) || (opts.m_StdFilter.m_u32Mask != CANACC_MASK_11BIT)) {
        retVal = canDevice.SetFilter11Bit(opts.m_StdFilter.m_u32Code, opts.m_StdFilter.m_u32Mask);
        if (retVal != CCanApi::NoError) {
            fprintf(stdout, "FAILED!\n");
            fprintf(stderr, "+++ error: CAN acceptance filter could not be set (%i)\n", retVal);
            goto teardown;
        }
    }
    /* -- set acceptance filter for 29-bit IDs */
    if (((opts.m_XtdFilter.m_u32Code != CANACC_CODE_29BIT) || (opts.m_XtdFilter.m_u32Mask != CANACC_MASK_29BIT)) && !opts.m_OpMode.nxtd) {
        retVal = canDevice.SetFilter29Bit(opts.m_XtdFilter.m_u32Code, opts.m_XtdFilter.m_u32Mask);
        if (retVal != CCanApi::NoError) {
            fprintf(stdout, "FAILED!\n");
            fprintf(stderr, "+++ error: CAN acceptance filter could not be set (%i)\n", retVal);
            goto teardown;
        }
    }
    fprintf(stdout, "OK!\n");
    /* - start communication */
    if (opts.m_Bitrate.btr.frequency > 0) {
        fprintf(stdout, "Bit-rate=%.0fkbps", opts.m_BusSpeed.nominal.speed / 1000.);
#if (CAN_FD_SUPPORTED != 0)
        if (opts.m_OpMode.byte & CANMODE_BRSE)
            fprintf(stdout, ":%.0fkbps", opts.m_BusSpeed.data.speed / 1000.);
        else if (opts.m_OpMode.byte & CANMODE_FDOE)
            fprintf(stdout, ":%.0fkbps", opts.m_BusSpeed.nominal.speed / 1000.);
#endif
        fprintf(stdout, "...");
    }
    else {
        fprintf(stdout, "Baudrate=%skbps...",
            opts.m_Bitrate.index == CANBTR_INDEX_1M   ? "1000" :
            opts.m_Bitrate.index == CANBTR_INDEX_800K ? "800" :
            opts.m_Bitrate.index == CANBTR_INDEX_500K ? "500" :
            opts.m_Bitrate.index == CANBTR_INDEX_250K ? "250" :
            opts.m_Bitrate.index == CANBTR_INDEX_125K ? "125" :
            opts.m_Bitrate.index == CANBTR_INDEX_100K ? "100" :
            opts.m_Bitrate.index == CANBTR_INDEX_50K  ? "50" :
            opts.m_Bitrate.index == CANBTR_INDEX_20K  ? "20" :
            opts.m_Bitrate.index == CANBTR_INDEX_10K  ? "10" : "?");
    }
    fflush(stdout);
    retVal = canDevice.StartController(opts.m_Bitrate);
    if (retVal != CCanApi::NoError) {
        fprintf(stdout, "FAILED!\n");
        fprintf(stderr, "+++ error: CAN Controller could not be started (%i)\n", retVal);
        goto teardown;
    }
    /* - start trace session (if enabled) */
#if (CAN_TRACE_SUPPORTED != 0)
    if (opts.m_eTraceMode != SOptions::eTraceOff) {
        /* -- set trace format */
        switch (opts.m_eTraceMode) {
            case SOptions::eTraceVendor:
                property[0] = CANPARA_TRACE_TYPE_VENDOR;
                break;
            case SOptions::eTraceLogger:
                property[0] = CANPARA_TRACE_TYPE_LOGGER;
                break;
            case SOptions::eTraceBinary:
            default:
                property[0] = CANPARA_TRACE_TYPE_BINARY;
                break;
        }
        (void)canDevice.SetProperty(CANPROP_SET_TRACE_TYPE, (void*)&property[0], sizeof(uint8_t));
        /* -- set trace active */
        property[0] = CANPARA_TRACE_ON;
        retVal = canDevice.SetProperty(CANPROP_SET_TRACE_ACTIVE, (void*)&property[0], sizeof(uint8_t));
        if (retVal != CCanApi::NoError) {
            fprintf(stdout, "FAILED!\n");
            fprintf(stderr, "+++ error: trace session could not be started (%i)\n", retVal);
            goto teardown;
        }
    }
#endif
    fprintf(stdout, "OK!\n");
    /* - reception loop */
    canDevice.ReceptionLoop();
    /* - stop trace session (if enabled) */
#if (CAN_TRACE_SUPPORTED != 0)
    if (opts.m_eTraceMode != SOptions::eTraceOff) {
        /* -- get trace file name */
        retVal = canDevice.GetProperty(CANPROP_GET_TRACE_FILE, (void*)property, CANPROP_MAX_BUFFER_SIZE);
        if (retVal == CCanApi::NoError) {
            property[CANPROP_MAX_BUFFER_SIZE] = '\0';
            fprintf(stdout, "Trace-file=%s\n", property);
        }
        /* -- set trace inactive */
        property[0] = CANPARA_TRACE_OFF;
        (void)canDevice.SetProperty(CANPROP_SET_TRACE_ACTIVE, (void*)&property[0], sizeof(uint8_t));
    }
#endif
    /* - show interface information */
    if ((string = canDevice.GetHardwareVersion()) != NULL)
        fprintf(stdout, "Hardware: %s\n", string);
    if ((string = canDevice.GetFirmwareVersion()) != NULL)
        fprintf(stdout, "Firmware: %s\n", string);
#if (OPTION_CANAPI_LIBRARY != 0)
    if ((string = canDevice.GetSoftwareVersion()) != NULL)
        fprintf(stdout, "Software: %s\n", string);
    if ((string = CCanDevice::GetVersion()) != NULL)
        fprintf(stdout, "          %s\n", string);
#else
    if ((string = CCanDevice::GetVersion()) != NULL)
        fprintf(stdout, "Software: %s\n", string);
#endif
teardown:
    /* - teardown the interface*/
    retVal = canDevice.TeardownChannel();
    if (retVal != CCanApi::NoError) {
        fprintf(stderr, "+++ error: CAN Controller could not be reset (%i)\n", retVal);
        goto farewell;
    }
farewell:
    /* So long and farewell! */
    opts.ShowFarewell(stdout);
    return retVal;
}

/*  List all supported CAN devices from CAN device list :
 *  - wrapper library: the device list is hard-wired (cf. can_boards[])
 *  - loader library: the device list is read from JSON configurations files
 *  return the number of supported CAN devices
 */
int CCanDevice::ListCanDevices(void) {
    int n = 0;

    fprintf(stdout, "Supported hardware:\n");
#if (OPTION_CANAPI_LIBRARY != 0)
    CCanDevice::SLibraryInfo library = { (-1), "", "" };
    bool iterLibrary = CCanDevice::GetFirstLibrary(library);
    while (iterLibrary) {
        CCanDevice::SChannelInfo channel = { (-1), "", "", (-1), "" };
        bool iterChannel = CCanDevice::GetFirstChannel(library.m_nLibraryId, channel);
        while (iterChannel) {
            fprintf(stdout, "\"%s\" (VendorName=\"%s\", LibraryId=%" PRIi32 ", ChannelNo=%" PRIi32 ")\n",
                            channel.m_szDeviceName, channel.m_szVendorName, channel.m_nLibraryId, channel.m_nChannelNo);
            n++;
            iterChannel = CCanDevice::GetNextChannel(channel);
        }
        iterLibrary = CCanDevice::GetNextLibrary(library);
    }
#else
    CCanDevice::SChannelInfo channel = { (-1), "", "", (-1), "" };
    bool iterChannel = CCanDevice::GetFirstChannel(channel);
    while (iterChannel) {
        fprintf(stdout, "\"%s\" (VendorName=\"%s\", LibraryId=%" PRIi32 ", ChannelNo=%" PRIi32 ")\n",
                        channel.m_szDeviceName, channel.m_szVendorName, channel.m_nLibraryId, channel.m_nChannelNo);
        n++;
        iterChannel = CCanDevice::GetNextChannel(channel);
    }
#if (SERIAL_CAN_SUPPORTED != 0)
    if (n == 0) {
        fprintf(stdout, "Check the Device Manager for compatible serial communication devices!\n");
    }
#endif
#endif
    return n;
}

/*  Test all supported CAN devices from CAN device list :
 *  - wrapper library: the device list is hard-wired (cf. can_boards[])
 *  - loader library: the device list is read from JSON configurations files
 *  * Also check if the given operation mode is supported by the CAN device.
 *  return the number of available CAN devices
 */
int CCanDevice::TestCanDevices(CANAPI_OpMode_t opMode) {
    int n = 0;

    fprintf(stdout, "Available hardware:\n");
#if (OPTION_CANAPI_LIBRARY != 0)
    int32_t blacklist[] = MONITOR_BLACKLIST;
    CCanDevice::SLibraryInfo library = { (-1), "", "" };
    bool iterLibrary = CCanDevice::GetFirstLibrary(library);
    while (iterLibrary) {
        CCanDevice::SChannelInfo channel = { (-1), "", "", (-1), "" };
        bool iterChannel = CCanDevice::GetFirstChannel(library.m_nLibraryId, channel);
        while (iterChannel && !IsBlacklisted(library.m_nLibraryId, blacklist)) {
            fprintf(stdout, "Hardware=%s...", channel.m_szDeviceName);
            fflush(stdout);
            EChannelState state;
            CANAPI_Return_t retVal = CCanDevice::ProbeChannel(library.m_nLibraryId, channel.m_nChannelNo, opMode, state);
            if ((retVal == CCanApi::NoError) || (retVal == CCanApi::IllegalParameter)) {
                CTimer::Delay(333U * CTimer::MSEC);  // to fake probing a hardware
                switch (state) {
                    case CCanApi::ChannelOccupied: fprintf(stdout, "occupied\n"); n++; break;
                    case CCanApi::ChannelAvailable: fprintf(stdout, "available\n"); n++; break;
                    case CCanApi::ChannelNotAvailable: fprintf(stdout, "not available\n"); break;
                    default: fprintf(stdout, "not testable\n"); break;
                }
                if (retVal == CCanApi::IllegalParameter)
                    fprintf(stderr, "+++ warning: CAN operation mode not supported (%02xh)\n", opMode.byte);
            } else
                fprintf(stdout, "FAILED!\n");
            iterChannel = CCanDevice::GetNextChannel(channel);
        }
        iterLibrary = CCanDevice::GetNextLibrary(library);
    }
#else
    CCanDevice::SChannelInfo channel = { (-1), "", "", (-1), "" };
    bool iterChannel = CCanDevice::GetFirstChannel(channel);
    while (iterChannel) {
        fprintf(stdout, "Hardware=%s...", channel.m_szDeviceName);
        fflush(stdout);
        EChannelState state;
        CANAPI_Return_t retVal = CCanDevice::ProbeChannel(channel.m_nChannelNo, opMode, state);
        if ((retVal == CCanApi::NoError) || (retVal == CCanApi::IllegalParameter)) {
            CTimer::Delay(333U * CTimer::MSEC);  // to fake probing a hardware
            switch (state) {
                case CCanApi::ChannelOccupied: fprintf(stdout, "occupied\n"); n++; break;
                case CCanApi::ChannelAvailable: fprintf(stdout, "available\n"); n++; break;
                case CCanApi::ChannelNotAvailable: fprintf(stdout, "not available\n"); break;
                default: fprintf(stdout, "not testable\n"); break;
            }
            if (retVal == CCanApi::IllegalParameter)
                fprintf(stderr, "+++ warning: CAN operation mode not supported (%02xh)\n", opMode.byte);
        } else
            fprintf(stdout, "FAILED!\n");
        iterChannel = CCanDevice::GetNextChannel(channel);
    }
#if (SERIAL_CAN_SUPPORTED != 0)
    if (n == 0) {
        fprintf(stdout, "Check the Device Manager for compatible serial communication devices!\n");
    }
#endif
#endif
    return n;
}

#if (OPTION_CANAPI_LIBRARY != 0)
bool CCanDevice::IsBlacklisted(int32_t library, int32_t blacklist[]) {
    for (int i = 0; blacklist[i] != EOF; i++)
        if (library == blacklist[i])
            return true;
    return false;
}
#endif

/*  List standard CAN bit-rate settings (only a choise):
 *  - CAN 2.0 (Classical CAN)
 *  - CAN FD w/0 Bit-rate Switching (BRS)
 *  - CAN FD with Bit-rate Switching (BRS)
 *  return the number of standard CAN bit-rate settings
 */
int CCanDevice::ListCanBitrates(CANAPI_OpMode_t opMode) {
    CANAPI_Bitrate_t bitrate[9];
    CANAPI_BusSpeed_t speed;

    char string[CANPROP_MAX_BUFFER_SIZE] = "";
    bool hasDataPhase = false;
    bool hasNoSamp = true;
    int i, n = 0;

#if (CAN_FD_SUPPORTED != 0)
    if (opMode.fdoe) {
        if (opMode.brse) {
            fprintf(stdout, "Bitrates - CAN FD with Bit-rate Switching (BRS):\n");
            BITRATE_FD_1M8M(bitrate[n]); n += 1;
            BITRATE_FD_500K4M(bitrate[n]); n += 1;
            BITRATE_FD_250K2M(bitrate[n]); n += 1;
            BITRATE_FD_125K1M(bitrate[n]); n += 1;
            hasDataPhase = true;
            hasNoSamp = false;
        }
        else {
            fprintf(stdout, "Bitrates - CAN FD without Bit-rate Switching (BRS):\n");
            BITRATE_FD_1M(bitrate[n]); n += 1;
            BITRATE_FD_500K(bitrate[n]); n += 1;
            BITRATE_FD_250K(bitrate[n]); n += 1;
            BITRATE_FD_125K(bitrate[n]); n += 1;
            hasDataPhase = false;
            hasNoSamp = false;
        }
    }
    else {
#else
    {
#endif
        fprintf(stdout, "Bitrates - CAN 2.0 (Classical CAN):\n");
        BITRATE_1M(bitrate[n]); n += 1;
#if (BITRATE_800K_UNSUPPORTED == 0)
        BITRATE_800K(bitrate[n]); n += 1;
#endif
        BITRATE_500K(bitrate[n]); n += 1;
        BITRATE_250K(bitrate[n]); n += 1;
        BITRATE_125K(bitrate[n]); n += 1;
        BITRATE_100K(bitrate[n]); n += 1;
        BITRATE_50K(bitrate[n]); n += 1;
        BITRATE_20K(bitrate[n]); n += 1;
        BITRATE_10K(bitrate[n]); n += 1;
        hasDataPhase = false;
        hasNoSamp = true;
    }
    for (i = 0; i < n; i++) {
        if (CCanDevice::MapBitrate2Speed(bitrate[i], speed) == CCanApi::NoError) {
            fprintf(stdout, "  %4.0fkbps@%.1f%%", speed.nominal.speed / 1000., speed.nominal.samplepoint * 100.);
#if (CAN_FD_SUPPORTED != 0)
            if (opMode.brse)
                fprintf(stdout, ":%4.0fkbps@%.1f%%", speed.data.speed / 1000., speed.data.samplepoint * 100.);
#else
            (void)opMode;  // to avoid compiler warnings
#endif
        }
        strcpy(string, "=oops, something went wrong!");
        (void)CCanDevice::MapBitrate2String(bitrate[i], string, CANPROP_MAX_BUFFER_SIZE, hasDataPhase, hasNoSamp);
        fprintf(stdout, "=\"%s\"\n", string);
    }
    return n;
}

#if (OPTION_CANAPI_LIBRARY == 0)
/*  Write the CAN device list into a JSON configuration file
 *  * only supported by CAN API wrapper libraries
 *  * the CAN API loader library read the JSON files
 */
bool CCanDevice::WriteJsonFile(const char* filename) {
    FILE* fp = fopen(filename, "w");
    if (!fp) {
        perror("");
        return false;
    }
    fprintf(fp,
            "{\n"
            "  \"format\": {\n"
            "    \"major\": 1,\n"
            "    \"minor\": 0\n"
            "  },\n"
            "  \"platform\": \"%s\",\n"
            "  \"vendor\": {\n",
            MONITOR_PLATFORM
           );
#if (SERIAL_CAN_SUPPORTED == 0)
    // oops, we need the name of wrapper library
    char wrapper[CANPROP_MAX_BUFFER_SIZE] = "";
    if (GetProperty(CANPROP_GET_LIBRARY_DLLNAME, (void*)wrapper, CANPROP_MAX_BUFFER_SIZE) != CCanApi::NoError)
        strcpy(wrapper, "(unknown)");
    // loop over the defive list
    CCanDevice::SChannelInfo channel = { (-1), "", "", (-1), "" };
    bool iterChannel = CCanDevice::GetFirstChannel(channel);
    if (iterChannel) {
        fprintf(fp,
            "    \"id\": %" PRIi32 ",\n"
            "    \"name\": \"%s\",\n"
            "    \"driver\": \"%s\",\n"
            "    \"library\": \"%s\",\n"
            "    \"legacy\": false\n",
            channel.m_nLibraryId,
            channel.m_szVendorName,
            channel.m_szDeviceDllName,
            wrapper
           );
    }
    fprintf(fp,
            "  },\n"
            "  \"boards\": [\n"
           );
    int n = 0;
    while (iterChannel) {
        fprintf(fp,
            "    {\n"
            "      \"id\": %" PRIi32 ",\n"
            "      \"name\": \"%s\",\n"
            "      \"alias\": \"%s%i\"\n",
            channel.m_nChannelNo,
            channel.m_szDeviceName,
            MONITOR_ALIASNAME, n++
           );
        iterChannel = CCanDevice::GetNextChannel(channel);
        fprintf(fp,
            "    %s\n", iterChannel ? "}," : "}"
           );
    }
#else
    /* SerialCAN: CAN-over-Serial-Line interfaces */
    fprintf(fp,
            "    \"id\": %i,\n"
            "    \"name\": \"%s\",\n"
            "    \"driver\": \"%s\",\n"
            "    \"library\": \"%s\",\n"
            "    \"legacy\": false\n"
            "  },\n"
            "  \"boards\": [\n",
            SLCAN_LIB_ID,
            "CAN-over-Serial-Line (SLCAN protocol)",
            SLCAN_LIB_DRIVER,
            SLCAN_LIB_WRAPPER
           );
    for (int i = 0; i < 8; i++) {
        fprintf(fp,
            "    {\n"
            "      \"id\": %i,\n"
            "      \"name\": \"%s%i\",\n"
            "      \"alias\": \"%s%i\"\n",
            CANDEV_SERIAL,
#if defined(_WIN32) || defined(_WIN64)
            MONITOR_TTYNAME, i + 1,
#else
            MONITOR_TTYNAME, i,
#endif
            MONITOR_ALIASNAME, i
           );
        fprintf(fp,
            "    %s\n", i < 7 ? "}," : "}"
           );
    }
#endif
    fprintf(fp,
            "  ]\n"
            "}\n"
           );
    if (fclose(fp) != 0) {
        perror("");
        return false;
    }
    return true;
}
#endif

/*  Reception loop: count received CAN messages until Ctrl-C
 */
uint64_t CCanDevice::ReceptionLoop() {
    CANAPI_Message_t message;
    uint64_t frames = 0U;

    char string[CANPROP_MAX_STRING_LENGTH+1];
    memset(string, 0, CANPROP_MAX_STRING_LENGTH+1);

    fprintf(stderr, "\nPress ^C to abort.\n\n");
    while(running) {
        if (ReadMessage(message) == CCanApi::NoError) {
            if ((((message.id < MAX_ID) && can_id[message.id]) || ((message.id >= MAX_ID) && can_id_xtd))) {
                (void)CCanMessage::Format(message, ++frames, string, CANPROP_MAX_STRING_LENGTH);
                fprintf(stdout, "%s\n", string);
            }
        }
    }
    fprintf(stdout, "\n");
    return frames;
}

// FIXME: matured code from can_moni for BerliOS SocketCAN
static int get_exclusion(const char* arg)
{
    char* val, *end;
    int i, inv = 0;
    long id, last = -1;

    if (!arg)
        return 0;

    val = (char* )arg;
    if (*val == '~') {
        inv = 1;
        val++;
    }
    for (;;) {
        errno = 0;
        id = strtol(val, &end, 0);

        if (errno == ERANGE && (id == LONG_MAX || id == LONG_MIN))
            return 0;
        if (errno != 0 && id == 0)
            return 0;
        if (val == end)
            return 0;

        if (id < MAX_ID)
            can_id[id] = 0;

        if (*end == '\0') {
            if (last != -1) {
                while (last != id) {
                    if (last < id)
                        last++;
                    else
                        last--;
                    can_id[last] = 0;
                }
                /*last = -1; <<< dead store */
            }
            break;
        }
        if (*end == ',') {
            if (last != -1) {
                while (last != id) {
                    if (last < id)
                        last++;
                    else
                        last--;
                    can_id[last] = 0;
                }
                last = -1;
            }
        }
        else if (*end == '-')
            last = id;
        else
            return 0;

        val = ++end;
    }
    if (inv) {
        for (i = 0; i < MAX_ID; i++)
            can_id[i] = can_id[i] ? 0 : 1;
    }
    can_id_xtd = inv ? 0 : 1;
    return 1;
}

/*  Signal handler to catch Ctrl+C:
 *  - signo: signal number (SIGINT, SIGHUP, SIGTERM)
 */
static void sigterm(int signo)
{
    //fprintf(stderr, "%s: got signal %d\n", __FILE__, signo);
    (void)canDevice.SignalChannel();
    running = 0;
    (void)signo;
}
