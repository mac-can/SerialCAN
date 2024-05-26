//  SPDX-License-Identifier: GPL-3.0-or-later
//
//  CAN Tester for generic Interfaces (CAN API V3)
//
//  Copyright (c) 2005-2010 Uwe Vogt, UV Software, Friedrichshafen
//  Copyright (c) 2012-2024 Uwe Vogt, UV Software, Berlin (info@uv-software.com)
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
#include "Options.h"

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <getopt.h>
#include <libgen.h>
#include <inttypes.h>
#include <errno.h>
#include <time.h>

#if defined(__linux__)
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
#define USE_BASENAME  1

#define DEFAULT_OP_MODE   CANMODE_DEFAULT
#define DEFAULT_BAUDRATE  CANBTR_INDEX_250K
#define DEFAULT_CAN_ID    0x100
#define DEFAULT_LENGTH    8

static const char* c_szApplication = CAN_TEST_APPLICATION;
static const char* c_szCopyright = CAN_TEST_COPYRIGHT;
static const char* c_szWarranty = CAN_TEST_WARRANTY;
static const char* c_szLicense = CAN_TEST_LICENSE;
static const char* c_szBasename = CAN_TEST_PROGRAM;
static const char* c_szInterface = "(unknown)";

SOptions::SOptions() {
    // to have default bus speed from bit-timing index
    (void)CCanDriver::MapIndex2Bitrate(DEFAULT_BAUDRATE, m_Bitrate);
    (void)CCanDriver::MapBitrate2Speed(m_Bitrate, m_BusSpeed);
    // initialization
    m_szBasename = (char*)c_szBasename;
    m_szInterface = (char*)c_szInterface;
#if (OPTION_CANAPI_LIBRARY != 0)
    m_szSearchPath = (char*)NULL;
#else
    m_szJsonFilename = (char*)NULL;
#endif
    m_OpMode.byte = DEFAULT_OP_MODE;
    m_Bitrate.index = DEFAULT_BAUDRATE;
    m_bHasDataPhase = false;
    m_bHasNoSamp = false;
    m_StdFilter.m_u32Code = CANACC_CODE_11BIT;
    m_StdFilter.m_u32Mask = CANACC_MASK_11BIT;
    m_XtdFilter.m_u32Code = CANACC_CODE_29BIT;
    m_XtdFilter.m_u32Mask = CANACC_MASK_29BIT;
    m_TestMode = SOptions::RxMODE;
    m_nStartNumber = (uint64_t)0;
    m_fCheckNumber = false;
    m_fStopOnError = false;
    m_nTxTime = (time_t)0;
    m_nTxFrames = (uint64_t)0;
    m_nTxDelay = (uint64_t)0;
    m_nTxCanId = (uint32_t)DEFAULT_CAN_ID;
    m_nTxCanDlc = (uint8_t)DEFAULT_LENGTH;
    m_fListBitrates = false;
    m_fListBoards = false;
    m_fTestBoards = false;
    m_fVerbose = false;
    m_fVerbose = false;
    m_fExit = false;
}

int SOptions::ScanCommanline(int argc, const char* argv[], FILE* err, FILE* out) {
    int opt;
    int64_t intarg;

    int optBitrate = 0;
    int optVerbose = 0;
    int optMode = 0;
    int optShared = 0;
    int optListenOnly = 0;
    int optErrorFrames = 0;
    int optExtendedFrames = 0;
    int optRemoteFrames = 0;
    int optStdCode = 0;
    int optStdMask = 0;
    int optXtdCode = 0;
    int optXtdMask = 0;
    int optReceive = 0;
    int optNumber = 0;
    int optStop = 0;
    int optTransmit = 0;
    int optFrames = 0;
    int optRandom = 0;
    int optCycle = 0;
    int optDlc = 0;
    int optId = 0;
    int optListBitrates = 0;
    int optListBoards = 0;
    int optTestBoards = 0;
#if (OPTION_CANAPI_LIBRARY != 0)
    int optPath = 0;
#else
    int optJson = 0;
#endif
    // command-line options
    int show_version = 0;
    struct option long_options[] = {
        {"baudrate", required_argument, 0, 'b'},
        {"bitrate", required_argument, 0, 'B'},
        {"verbose", no_argument, 0, 'v'},
        {"mode", required_argument, 0, 'm'},
        {"shared", no_argument, 0, 'S'},
        {"listen-only", no_argument, 0, 'M'},
        {"error-frames", no_argument, 0, 'E'},
        {"no-remote-frames", no_argument, 0, 'R'},
        {"no-extended-frames", no_argument, 0, 'X'},
        {"code", required_argument, 0, '1'},
        {"mask", required_argument, 0, '2'},
        {"xtd-code", required_argument, 0, '3'},
        {"xtd-mask", required_argument, 0, '4'},
        {"receive", no_argument, 0, 'r'},
        {"number", required_argument, 0, 'n'},
        {"stop", no_argument, 0, 's'},
        {"transmit", required_argument, 0, 't'},
        {"frames", required_argument, 0, 'f'},
        {"random", required_argument, 0, 'F'},
        {"cycle", required_argument, 0, 'c'},
        {"usec", required_argument, 0, 'u'},
        {"dlc", required_argument, 0, 'd'},
        {"data", required_argument, 0, 'd'},
        {"id", required_argument, 0, 'i'},
        {"list-bitrates", optional_argument, 0, 'l'},
#if (OPTION_CANAPI_LIBRARY != 0)
        {"list-boards", optional_argument, 0, 'L'},
        {"test-boards", optional_argument, 0, 'T'},
        {"path", required_argument, 0, 'p'},
#else
        {"list-boards", no_argument, 0, 'L'},
        {"test-boards", no_argument, 0, 'T'},
        {"json", required_argument, 0, 'j'},
#endif
        {"help", no_argument, 0, 'h'},
        {"version", no_argument, &show_version, 1},
        {0, 0, 0, 0}
    };
    // (0) sanity check
    if ((argc <= 0) || (argv == NULL))
        return (-1);
    if ((err == NULL) || (out == NULL))
        return (-1);
    // (1) get basename from command-line
#if (USE_BASENAME != 0)
    m_szBasename = basename((char*)argv[0]);
#endif
    // (2) scan command-line for options
#if (OPTION_CANAPI_LIBRARY != 0)
    while ((opt = getopt_long(argc, (char * const *)argv, "b:vp:m:rn:st:f:R:c:u:d:i:lLaTh", long_options, NULL)) != -1) {
#else
    while ((opt = getopt_long(argc, (char * const *)argv, "b:vm:rn:st:f:R:c:u:d:i:lLaTj:h", long_options, NULL)) != -1) {
#endif
        switch (opt) {
        /* option '--baudrate=<baudrate>' (-b) */
        case 'b':
            if (optBitrate++) {
                fprintf(err, "%s: duplicated option `--baudrate' (%c)\n", m_szBasename, opt);
                return 1;
            }
            if (optarg == NULL) {
                fprintf(err, "%s: missing argument for option `--baudrate' (%c)\n", m_szBasename, opt);
                return 1;
            }
            if (sscanf(optarg, "%" SCNi64, &intarg) != 1) {
                fprintf(err, "%s: illegal argument for option `--baudrate' (%c)\n", m_szBasename, opt);
                return 1;
            }
            switch (intarg) {
            case 0: case 1000: case 1000000: m_Bitrate.index = (int32_t)CANBTR_INDEX_1M; break;
            case 1: case 800:  case 800000:  m_Bitrate.index = (int32_t)CANBTR_INDEX_800K; break;
            case 2: case 500:  case 500000:  m_Bitrate.index = (int32_t)CANBTR_INDEX_500K; break;
            case 3: case 250:  case 250000:  m_Bitrate.index = (int32_t)CANBTR_INDEX_250K; break;
            case 4: case 125:  case 125000:  m_Bitrate.index = (int32_t)CANBTR_INDEX_125K; break;
            case 5: case 100:  case 100000:  m_Bitrate.index = (int32_t)CANBTR_INDEX_100K; break;
            case 6: case 50:   case 50000:   m_Bitrate.index = (int32_t)CANBTR_INDEX_50K; break;
            case 7: case 20:   case 20000:   m_Bitrate.index = (int32_t)CANBTR_INDEX_20K; break;
            case 8: case 10:   case 10000:   m_Bitrate.index = (int32_t)CANBTR_INDEX_10K; break;
            default:                         m_Bitrate.index = (int32_t)-intarg; break;
            }
            CANAPI_Bitrate_t bitrate;  // in order not to overwrite the index
            if (CCanDriver::MapIndex2Bitrate(m_Bitrate.index, bitrate) != CCanApi::NoError) {
                fprintf(err, "%s: illegal argument for option `--baudrate' (%c)\n", m_szBasename, opt);
                return 1;
            }
            if (CCanDriver::MapBitrate2Speed(bitrate, m_BusSpeed) != CCanApi::NoError) {
                fprintf(err, "%s: illegal argument for option `--baudrate' (%c)\n", m_szBasename, opt);
                return 1;
            }
            break;
        /* option '--bitrate=<bit-rate>' as string */
        case 'B':
            if (optBitrate++) {
                fprintf(err, "%s: duplicated option `--bitrate'\n", m_szBasename);
                return 1;
            }
            if (optarg == NULL) {
                fprintf(err, "%s: missing argument for option `--bitrate'\n", m_szBasename);
                return 1;
            }
            if (CCanDriver::MapString2Bitrate(optarg, m_Bitrate, m_bHasDataPhase, m_bHasNoSamp) != CCanApi::NoError) {
                fprintf(err, "%s: illegal argument for option `--bitrate'\n", m_szBasename);
                return 1;
            }
            if (CCanDriver::MapBitrate2Speed(m_Bitrate, m_BusSpeed) != CCanApi::NoError) {
                fprintf(err, "%s: illegal argument for option `--bitrate'\n", m_szBasename);
                return 1;
            }
            break;
        /* option '--verbose' (-v) */
        case 'v':
            if (optVerbose) {
                fprintf(err, "%s: duplicated option `--verbose' (%c)\n", m_szBasename, opt);
                return 1;
            }
            if (optarg != NULL) {
                fprintf(err, "%s: illegal argument for option `--verbose' (%c)\n", m_szBasename, opt);
                return 1;
            }
            m_fVerbose = true;
            break;
#if (OPTION_CANAPI_LIBRARY != 0)
        /* option '--path=<pathname>' (-p) */
        case 'p':
            if (optPath++) {
                fprintf(err, "%s: duplicated option `--path' (%c)\n", m_szBasename, opt);
                return 1;
            }
            if (optarg == NULL) {
                fprintf(err, "%s: missing argument for option `--path' (%c)\n", m_szBasename, opt);
                return 1;
            }
            m_szSearchPath = optarg;
            break;
#endif
        /* option '--mode=(2.0|FDF[+BRS])' (-m) */
        case 'm':
            if (optMode++) {
                fprintf(err, "%s: duplicated option `--mode' (%c)\n", m_szBasename, opt);
                return 1;
            }
            if (optarg == NULL) {
                fprintf(err, "%s: missing argument for option `--mode' (%c)\n", m_szBasename, opt);
                return 1;
            }
            if (!strcasecmp(optarg, "DEFAULT") || !strcasecmp(optarg, "CLASSIC") || !strcasecmp(optarg, "CLASSICAL") ||
                !strcasecmp(optarg, "CAN20") || !strcasecmp(optarg, "CAN2.0") || !strcasecmp(optarg, "2.0"))
                m_OpMode.byte |= CANMODE_DEFAULT;
#if (CAN_FD_SUPPORTED != 0)
            else if (!strcasecmp(optarg, "CANFD") || !strcasecmp(optarg, "FD") || !strcasecmp(optarg, "FDF"))
                m_OpMode.byte |= CANMODE_FDOE;
            else if (!strcasecmp(optarg, "CANFD+BRS") || !strcasecmp(optarg, "FDF+BRS") || !strcasecmp(optarg, "FD+BRS"))
                m_OpMode.byte |= CANMODE_FDOE | CANMODE_BRSE;
#endif
            else {
                fprintf(err, "%s: illegal argument for option `--mode' (%c)\n", m_szBasename, opt);
                return 1;
            }
            break;
        /* option '--shared' */
        case 'S':
            if (optShared++) {
                fprintf(err, "%s: duplicated option `--shared'\n", m_szBasename);
                return 1;
            }
            if (optarg != NULL) {
                fprintf(err, "%s: illegal argument for option `--shared'\n", m_szBasename);
                return 1;
            }
            m_OpMode.byte |= CANMODE_SHRD;
            break;
        /* option '--listen-only' */
        case 'M':
            if (optListenOnly++) {
                fprintf(err, "%s: duplicated option `--listen-only'\n", m_szBasename);
                return 1;
            }
            if (optarg != NULL) {
                fprintf(err, "%s: illegal argument for option `--listen-only'\n", m_szBasename);
                return 1;
            }
            m_OpMode.byte |= CANMODE_MON;
            break;
        /* option '--error-frames' */
        case 'E':
            if (optErrorFrames++) {
                fprintf(err, "%s: duplicated option `--error-frames'\n", m_szBasename);
                return 1;
            }
            if (optarg != NULL) {
                fprintf(err, "%s: illegal argument for option `--error-frames'\n", m_szBasename);
                return 1;
            }
           m_OpMode.byte |= CANMODE_ERR;
 
            break;
        /* option '--no-extended-frames' */
        case 'X':
            if (optExtendedFrames++) {
                fprintf(err, "%s: duplicated option `--no-extended-frames'\n", m_szBasename);
                return 1;
            }
            if (optarg != NULL) {
                fprintf(err, "%s: illegal argument for option `--no-extended-frames'\n", m_szBasename);
                return 1;
            }
           m_OpMode.byte |= CANMODE_NXTD;
            break;
        /* option '--no-remote-frames' */
        case 'R':
            if (optRemoteFrames++) {
                fprintf(err, "%s: duplicated option `--no-remote-frames'\n", m_szBasename);
                return 1;
            }
            if (optarg != NULL) {
                fprintf(err, "%s: missing argument for option `--no-remote-frames'\n", m_szBasename);
                return 1;
            }
           m_OpMode.byte |= CANMODE_NRTR;
            break;
        /* option '--code=<11-bit-code>' */
        case '1':
            if (optStdCode++) {
                fprintf(err, "%s: duplicated option `--code'\n", m_szBasename);
                return 1;
            }
            if (optarg == NULL) {
                fprintf(err, "%s: missing argument for option `--code'\n", m_szBasename);
                return 1;
            }
            if (sscanf(optarg, "%" SCNx64, &intarg) != 1) {
                fprintf(err, "%s: illegal argument for option `--code'\n", m_szBasename);
                return 1;
            }
            if ((intarg & ~CAN_MAX_STD_ID) != 0) {
                fprintf(err, "%s: illegal argument for option --code'\n", m_szBasename);
                return 1;
            }
            m_StdFilter.m_u32Code = (uint32_t)intarg;
            break;
        /* option '--mask=<11-bit-mask>' */
        case '2':
            if (optStdMask++) {
                fprintf(err, "%s: duplicated option `--mask'\n", m_szBasename);
                return 1;
            }
            if (optarg == NULL) {
                fprintf(err, "%s: missing argument for option `--mask'\n", m_szBasename);
                return 1;
            }
            if (sscanf(optarg, "%" SCNx64, &intarg) != 1) {
                fprintf(err, "%s: illegal argument for option --mask'\n", m_szBasename);
                return 1;
            }
            if ((intarg & ~CAN_MAX_STD_ID) != 0) {
                fprintf(err, "%s: illegal argument for option --mask'\n", m_szBasename);
                return 1;
            }
            m_StdFilter.m_u32Mask = (uint32_t)intarg;
            break;
        /* option '--xtd-code=<29-bit-code>' */
        case '3':
            if (optXtdCode++) {
                fprintf(err, "%s: duplicated option `--xtd-code'\n", m_szBasename);
                return 1;
            }
            if (optarg == NULL) {
                fprintf(err, "%s: missing argument for option `--xtd-code'\n", m_szBasename);
                return 1;
            }
            if (sscanf(optarg, "%" SCNx64, &intarg) != 1) {
                fprintf(err, "%s: illegal argument for option --xtd-code'\n", m_szBasename);
                return 1;
            }
            if ((intarg & ~CAN_MAX_XTD_ID) != 0) {
                fprintf(err, "%s: illegal argument for option --xtd-code'\n", m_szBasename);
                return 1;
            }
            m_XtdFilter.m_u32Code = (uint32_t)intarg;
            break;
        /* option '--xtd-mask=<29-bit-mask>' */
        case '4':
            if (optXtdMask++) {
                fprintf(err, "%s: duplicated option `--xtd-mask'\n", m_szBasename);
                return 1;
            }
            if (optarg == NULL) {
                fprintf(err, "%s: missing argument for option `--xtd-mask'\n", m_szBasename);
                return 1;
            }
            if (sscanf(optarg, "%" SCNx64, &intarg) != 1) {
                fprintf(err, "%s: illegal argument for option --xtd-mask'\n", m_szBasename);
                return 1;
            }
            if ((intarg & ~CAN_MAX_XTD_ID) != 0) {
                fprintf(err, "%s: illegal argument for option --xtd-mask'\n", m_szBasename);
                return 1;
            }
            m_XtdFilter.m_u32Mask = (uint32_t)intarg;
            break;
        /* option '--receive' (-r) */
        case 'r':
            if (optReceive++) {
                fprintf(err, "%s: duplicated option `--receive' (%c)\n", m_szBasename, opt);
                return 1;
            }
            if (optarg != NULL) {
                fprintf(err, "%s: illegal argument for option `--receive' (%c)\n", m_szBasename, opt);
                return 1;
            }
            m_TestMode = SOptions::RxMODE;
            break;
        /* option '--number=<offset>' (-n) */
        case 'n':
            if (optNumber++) {
                fprintf(err, "%s: duplicated option `--number' (%c)\n", m_szBasename, opt);
                return 1;
            }
            if (optarg == NULL) {
                fprintf(err, "%s: missing argument for option `--number' (%c)\n", m_szBasename, opt);
                return 1;
            }
            if (sscanf(optarg, "%" SCNi64, &intarg) != 1) {
                fprintf(err, "%s: illegal argument for option `--number' (%c)\n", m_szBasename, opt);
                return 1;
            }
            if (intarg < 0) {
                fprintf(err, "%s: illegal argument for option `--number' (%c)\n", m_szBasename, opt);
                return 1;
            }
            m_nStartNumber = (uint64_t)intarg;
            m_fCheckNumber = true;
            break;
        /* option '--stop' (-s) */
        case 's':
            if (optStop++) {
                fprintf(err, "%s: duplicated option `--stop' (%c)\n", m_szBasename, opt);
                return 1;
            }
            if (optarg != NULL) {
                fprintf(err, "%s: illegal argument for option `--stop' (%c)\n", m_szBasename, opt);
                return 1;
            }
            m_fStopOnError = 1;
            break;
        /* option '--transmit=<duration>' (-t) in [s] */
        case 't':
            if (optTransmit++) {
                fprintf(err, "%s: duplicated option `--transmit' (%c)\n", m_szBasename, opt);
                return 1;
            }
            if (optarg == NULL) {
                fprintf(err, "%s: missing argument for `--transmit' (%c)\n", m_szBasename, opt);
                return 1;
            }
            if (sscanf(optarg, "%" SCNi64, &intarg) != 1) {
                fprintf(err, "%s: illegal argument for option `--transmit' (%c)\n", m_szBasename, opt);
                return 1;
            }
            if (intarg < 0) {
                fprintf(err, "%s: illegal argument for option `--transmit' (%c)\n", m_szBasename, opt);
                return 1;
            }
            m_nTxTime = (time_t)intarg;
            m_TestMode = SOptions::TxMODE;
            break;
        /* option '--frames=<frames>' (-f) */
        case 'f':
            if (optFrames++) {
                fprintf(err, "%s: duplicated option `--frames' (%c)\n", m_szBasename, opt);
                return 1;
            }
            if (optarg == NULL) {
                fprintf(err, "%s: missing argument for option `--frames' (%c)\n", m_szBasename, opt);
                return 1;
            }
            if (sscanf(optarg, "%" SCNi64, &intarg) != 1) {
                fprintf(err, "%s: illegal argument for option `--frames' (%c)\n", m_szBasename, opt);
                return 1;
            }
            if (intarg < 0) {
                fprintf(err, "%s: illegal argument for option `--frames' (%c)\n", m_szBasename, opt);
                return 1;
            }
            m_nTxFrames = (uint64_t)intarg;
            m_TestMode = SOptions::TxFRAMES;
            break;
        /* option '--random=<frames>' */
        case 'F':
            if (optRandom++) {
                fprintf(err, "%s: duplicated option `--random'\n", m_szBasename);
                return 1;
            }
            if (optarg == NULL) {
                fprintf(err, "%s: missing argument for option `--random'\n", m_szBasename);
                return 1;
            }
            if (sscanf(optarg, "%" SCNi64, &intarg) != 1) {
                fprintf(err, "%s: illegal argument for option `--random'\n", m_szBasename);
                return 1;
            }
            if (intarg < 0) {
                fprintf(err, "%s: illegal argument for option `--random'\n", m_szBasename);
                return 1;
            }
            if (!optDlc) /* let the tester generate messages of arbitrary length */
                m_nTxCanDlc = (uint8_t)0;
            m_nTxFrames = (uint64_t)intarg;
            m_TestMode = SOptions::TxRANDOM;
            break;
        /* option '--cycle=<msec>' (-c) */
        case 'c':
            if (optCycle++) {
                fprintf(err, "%s: duplicated option `--cycle' (%c)\n", m_szBasename, opt);
                return 1;
            }
            if (optarg == NULL) {
                fprintf(err, "%s: missing argument for option `--cycle' (%c)\n", m_szBasename, opt);
                return 1;
            }
            if (sscanf(optarg, "%" SCNi64, &intarg) != 1) {
                fprintf(err, "%s: illegal argument for option `--cycle' (%c)\n", m_szBasename, opt);
                return 1;
            }
            if ((intarg < 0) || (intarg > (INT_MAX / 1000))) {
                fprintf(err, "%s: illegal argument for option `--cycle' (%c)\n", m_szBasename, opt);
                return 1;
            }
            m_nTxDelay = (uint64_t)(intarg * 1000l);
            break;
        /* option '--usec=<usec>' (-u) */
        case 'u':
            if (optCycle++) {
                fprintf(err, "%s: duplicated option `--usec' (%c)\n", m_szBasename, opt);
                return 1;
            }
            if (optarg == NULL) {
                fprintf(err, "%s: missing argument for option `--usec' (%c)\n", m_szBasename, opt);
                return 1;
            }
            if (sscanf(optarg, "%" SCNi64, &intarg) != 1) {
                fprintf(err, "%s: illegal argument for option `--usec' (%c)\n", m_szBasename, opt);
                return 1;
            }
            if (intarg < 0) {
                fprintf(err, "%s: illegal argument for option `--usec' (%c)\n", m_szBasename, opt);
                return 1;
            }
            m_nTxDelay = (uint64_t)intarg;
            break;
        /* option '--dlc=<length>' (-d) */
        case 'd':
            if (optDlc++) {
                fprintf(err, "%s: duplicated option `--dlc' (%c)\n", m_szBasename, opt);
                return 1;
            }
            if (optarg == NULL) {
                fprintf(err, "%s: missing argument for option `--dlc' (%c)\n", m_szBasename, opt);
                return 1;
            }
            if (sscanf(optarg, "%" SCNi64, &intarg) != 1) {
                fprintf(err, "%s: illegal argument for option `--dlc' (%c)\n", m_szBasename, opt);
                return 1;
            }
#if (CAN_FD_SUPPORTED != 0)
            if ((intarg < 0) || (CANFD_MAX_LEN < intarg)) {
#else
            if ((intarg < 0) || (CAN_MAX_LEN < intarg)) {
#endif
                fprintf(err, "%s: illegal argument for option `--dlc' (%c)\n", m_szBasename, opt);
                return 1;
            }
            m_nTxCanDlc = (uint8_t)intarg;
            break;
        /* option '--id=<identifier>' (-i) */
        case 'i':
            if (optId++) {
                fprintf(err, "%s: duplicated option `--id' (%c)\n", m_szBasename, opt);
                return 1;
            }
            if (optarg == NULL) {
                fprintf(err, "%s: missing argument for option `--id' (%c)\n", m_szBasename, opt);
                return 1;
            }
            if (sscanf(optarg, "%" SCNi64, &intarg) != 1) {
                fprintf(err, "%s: illegal argument for option `--id' (%c)\n", m_szBasename, opt);
                return 1;
            }
            if ((intarg < 0x000) || (CAN_MAX_XTD_ID < intarg)) { // TODO: to be checked with --mode=NXTD
                fprintf(err, "%s: illegal argument for option `--id' (%c)\n", m_szBasename, opt);
                return 1;
            }
            m_nTxCanId = (uint32_t)intarg;
            break;
        /* option '--list-bitrates[=(2.0|FDF[+BRS])]' */
        case 'l':
            if (optListBitrates++) {
                fprintf(err, "%s: duplicated option `--list-bitrates'\n", m_szBasename);
                return 1;
            }
            if (optarg != NULL) {
                if (optMode++) {
                    fprintf(err, "%s: option `--list-bitrates' - operation mode already set'\n", m_szBasename);
                    return 1;
                }
                if (!strcasecmp(optarg, "DEFAULT") || !strcasecmp(optarg, "CLASSIC") || !strcasecmp(optarg, "CLASSICAL") ||
                    !strcasecmp(optarg, "CAN20") || !strcasecmp(optarg, "CAN2.0") || !strcasecmp(optarg, "2.0"))
                    m_OpMode.byte |= CANMODE_DEFAULT;
#if (CAN_FD_SUPPORTED != 0)
                else if (!strcasecmp(optarg, "CANFD") || !strcasecmp(optarg, "FD") || !strcasecmp(optarg, "FDF"))
                    m_OpMode.byte |= CANMODE_FDOE;
                else if (!strcasecmp(optarg, "CANFD+BRS") || !strcasecmp(optarg, "FDF+BRS") || !strcasecmp(optarg, "FD+BRS"))
                    m_OpMode.byte |= CANMODE_FDOE | CANMODE_BRSE;
#endif
                else {
                    fprintf(err, "%s: illegal argument for option `--list-bitrates'\n", m_szBasename);
                    return 1;
                }
            }
            m_fListBitrates = true;
            m_fExit = true;
            break;
        /* option '--list-boards' (-L) */
        case 'a':  /* (-a, deprecated) */
        case 'L':
            if (optListBoards++) {
                fprintf(err, "%s: duplicated option `--list-boards' (%c)\n", m_szBasename, opt);
                return 1;
            }
#if (OPTION_CANAPI_LIBRARY != 0)
            if (optarg != NULL) {
                if (optPath++) {
                    fprintf(err, "%s: option `--list-boards' - search path already set' (%c)\n", m_szBasename, opt);
                    return 1;
                }
                m_szSearchPath = optarg;  // option '--list-boards=<pathname>' (-L)
            }
#else
            if (optarg != NULL) {
                fprintf(err, "%s: illegal argument for option `--list-boards' (%c)\n", m_szBasename, opt);
                return 1;
            }
#endif
            m_fListBoards = true;
            m_fExit = true;
            break;
        /* option '--test-boards' (-T) */
        case 'T':
            if (optTestBoards++) {
                fprintf(err, "%s: duplicated option `--test-boards' (%c)\n", m_szBasename, opt);
                return 1;
            }
#if (OPTION_CANAPI_LIBRARY != 0)
            if (optarg != NULL) {
                if (optPath++) {
                    fprintf(err, "%s: option `--test-boards' - search path already set' (%c)\n", m_szBasename, opt);
                    return 1;
                }
                m_szSearchPath = optarg;  // option '--test-boards=<pathname>' (-L)
            }
#else
            if (optarg != NULL) {
                fprintf(err, "%s: illegal argument for option `--test-boards' (%c)\n", m_szBasename, opt);
                return 1;
            }
#endif
            m_fTestBoards = true;
            m_fExit = true;
            break;
#if (OPTION_CANAPI_LIBRARY == 0)
        /* option '--json=<filename>' (-j) */
        case 'j':
            if (optJson++) {
                fprintf(err, "%s: duplicated option `--json' (%c)\n", m_szBasename, opt);
                return 1;
            }
            if (optarg == NULL) {
                fprintf(err, "%s: missing argument for option `--json' (%c)\n", m_szBasename, opt);
                return 1;
            }
            m_szJsonFilename = optarg;
            m_fExit = true;
            break;
#endif
        /* option '--help' (-h) */
        case 'h':
            ShowHelp(out);
            m_fExit = true;
            return 1;
        case '?':
            if (!opterr)
                ShowUsage(out);
            m_fExit = true;
            return 1;
        default:
            if (show_version)
                ShowVersion(out);
            else
                ShowUsage(out);
            m_fExit = true;
            return 1;
        }
    }
    // (3) scan command-line for argument <interface>
    // - check if one and only one <interface> is given
    if (optind + 1 != argc) {
        if (optind != argc) {
            fprintf(err, "%s: too many arguments given\n", m_szBasename);
            return 1;
        } else if (!m_fExit) {
            fprintf(err, "%s: no interface given\n", m_szBasename);
            return 1;
        } else {
            // no interface given, but --list-boards, --test-boards or --list-bitrates
            return 0;
        }
    } else {
        m_szInterface = (char*)argv[optind];
    }
    // (4) check for illegal combinations
#if (CAN_FD_SUPPORTED != 0)
    /* - check bit-timing index (n/a for CAN FD) */
    if (m_OpMode.fdoe && (m_Bitrate.btr.frequency <= CANBTR_INDEX_1M) && !m_fExit) {
        fprintf(err, "%s: illegal combination of options `--mode' (m) and `--bitrate'\n", m_szBasename);
        return 1;
    }
    /* - check data length and make CAN FD DLC (0x0..0xF) */
    if (!m_OpMode.fdoe && (m_nTxCanDlc > CAN_MAX_LEN) && !m_fExit) {
        fprintf(err, "%s: illegal combination of options `--mode' (m) and `--dlc' (d)\n", m_szBasename);
        return 1;
    } else {
        /* convert length into DLC */
        if (m_nTxCanDlc > 48) m_nTxCanDlc = 0xF;
        else if (m_nTxCanDlc > 32) m_nTxCanDlc = 0xE;
        else if (m_nTxCanDlc > 24) m_nTxCanDlc = 0xD;
        else if (m_nTxCanDlc > 20) m_nTxCanDlc = 0xC;
        else if (m_nTxCanDlc > 16) m_nTxCanDlc = 0xB;
        else if (m_nTxCanDlc > 12) m_nTxCanDlc = 0xA;
        else if (m_nTxCanDlc > 8) m_nTxCanDlc = 0x9;
    }
#endif
    /* - check operation mode flags */
    if ((m_TestMode != SOptions::RxMODE) && m_OpMode.mon && !m_fExit) {
        fprintf(err, "%s: illegal option `--listen-only' for transmitter test\n", m_szBasename);
        return 1;
    }
    if ((m_TestMode != SOptions::RxMODE) && m_OpMode.err && !m_fExit) {
        fprintf(err, "%s: illegal option `--error-frames' for transmitter test\n", m_szBasename);
        return 1;
    }
    if ((m_TestMode != SOptions::RxMODE) && m_OpMode.nxtd && !m_fExit) {
        fprintf(err, "%s: illegal option `--no-extended-frames' for transmitter test\n", m_szBasename);
        return 1;
    }
    if ((m_TestMode != SOptions::RxMODE) && m_OpMode.nrtr && !m_fExit) {
        fprintf(err, "%s: illegal option `--no-remote-frames' for transmitter test\n", m_szBasename);
        return 1;
    }
    return 0;
}

void SOptions::ShowGreetings(FILE* stream) {
    if (!stream)
        return;
    fprintf(stream, "%s\n%s\n\n%s\n\n", c_szApplication, c_szCopyright, c_szWarranty);
}

void SOptions::ShowFarewell(FILE* stream) {
    if (!stream)
        return;
    fprintf(stream, "%s\n", c_szCopyright);
}

void SOptions::ShowVersion(FILE* stream) {
    if (!stream)
        return;
    fprintf(stream, "%s\n%s\n\n%s\n\n", c_szApplication, c_szCopyright, c_szLicense);
    fprintf(stream, "Written by Uwe Vogt, UV Software, Berlin <https://www.uv-software.com/>\n");
}

void SOptions::ShowUsage(FILE* stream, bool args) {
    if(!stream)
        return;
    fprintf(stream, "Usage: %s <interface> [<option>...]\n", m_szBasename);
    fprintf(stream, "Options for receiver test (default test mode):\n");
    fprintf(stream, " -r, --receive                        count received messages until ^C is pressed\n");
    fprintf(stream, " -n, --number=<number>                check up-counting numbers starting with <number>\n");
    fprintf(stream, " -s, --stop                           stop on error (with option --number)\n");
#if (OPTION_CANAPI_LIBRARY != 0)
    fprintf(stream, " -p, --path=<pathname>                search path for JSON configuration files\n");
#endif
#if (CAN_FD_SUPPORTED != 0)
    fprintf(stream, " -m, --mode=(2.0|FDF[+BRS])           CAN operation mode: CAN 2.0 or CAN FD mode\n");
#else
    fprintf(stream, " -m, --mode=2.0                       CAN operation mode: CAN 2.0\n");
#endif
    fprintf(stream, "     --shared                         shared CAN controller access (if supported)\n");
    fprintf(stream, "     --listen-only                    monitor mode (listen-only mode)\n");
    fprintf(stream, "     --error-frames                   allow reception of error frames\n");
    fprintf(stream, "     --no-remote-frames               suppress remote frames (RTR frames)\n");
    fprintf(stream, "     --no-extended-frames             suppress extended frames (29-bit identifier)\n");
    fprintf(stream, " -b, --baudrate=<baudrate>            CAN bit-timing in kbps (default=250), or\n");
    fprintf(stream, "     --bitrate=<bit-rate>             CAN bit-rate settings (as key/value list)\n");
    fprintf(stream, " -v, --verbose                        show detailed bit-rate settings\n");
    fprintf(stream, "Options for transmitter test:\n");
    fprintf(stream, " -t, --transmit=<time>                send messages for the given time in seconds, or\n");
    fprintf(stream, " -f, --frames=<number>,               alternatively send the given number of messages, or\n");
    fprintf(stream, "     --random=<number>                optionally with random cycle time and data length\n");
    fprintf(stream, " -c, --cycle=<cycle>                  cycle time in milliseconds (default=0) or\n");
    fprintf(stream, " -u, --usec=<cycle>                   cycle time in microseconds (default=0)\n");
    fprintf(stream, " -d, --dlc=<length>                   send messages of given length (default=8)\n");
    fprintf(stream, " -i, --id=<can-id>                    use given identifier (default=100h)\n");
    fprintf(stream, " -n, --number=<number>                set first up-counting number (default=0)\n");
#if (OPTION_CANAPI_LIBRARY != 0)
    fprintf(stream, " -p, --path=<pathname>                search path for JSON configuration files\n");
#endif
#if (CAN_FD_SUPPORTED != 0)
    fprintf(stream, " -m, --mode=(2.0|FDF[+BRS])           CAN operation mode: CAN 2.0 or CAN FD mode\n");
#else
    fprintf(stream, " -m, --mode=2.0                       CAN operation mode: CAN 2.0\n");
#endif
    fprintf(stream, "     --shared                         shared CAN controller access (if supported)\n");
    fprintf(stream, " -b, --baudrate=<baudrate>            CAN bit-timing in kbps (default=250), or\n");
    fprintf(stream, "     --bitrate=<bit-rate>             CAN bit-rate settings (as key/value list)\n");
    fprintf(stream, " -v, --verbose                        show detailed bit-rate settings\n");
    fprintf(stream, "Other options:\n");
#if (CAN_FD_SUPPORTED != 0)
    fprintf(stream, "     --list-bitrates[=<mode>]         list standard bit-rate settings and exit\n");
#else
    fprintf(stream, "     --list-bitrates[=2.0]            list standard bit-rate settings and exit\n");
#endif
#if (OPTION_CANAPI_LIBRARY != 0)
    fprintf(stream, " -L, --list-boards[=<pathname>]       list all supported CAN interfaces and exit\n");
    fprintf(stream, " -T, --test-boards[=<pathname>]       list all available CAN interfaces and exit\n");
#elif (SERIAL_CAN_SUPPORTED == 0)
    fprintf(stream, " -L, --list-boards                    list all supported CAN interfaces and exit\n");
    fprintf(stream, " -T, --test-boards                    list all available CAN interfaces and exit\n");
    fprintf(stream, " -J, --json=<filename>                write configuration into JSON file and exit\n");
#endif
    fprintf(stream, " -h, --help                           display this help screen and exit\n");
    fprintf(stream, "     --version                        show version information and exit\n");
    if (args) {
        fprintf(stream, "Arguments:\n");
        fprintf(stream, "  <frames>       send this number of messages (frames), or\n");
        fprintf(stream, "  <time>         send messages for the given time in seconds\n");
        fprintf(stream, "  <msec>         cycle time in milliseconds (default=0), or \n");
        fprintf(stream, "  <usec>         cycle time in microseconds (default=0)\n");
        fprintf(stream, "  <can-id>       send with given identifier (default=100h)\n");
        fprintf(stream, "  <length>       send data of given length (default=8)\n");
        fprintf(stream, "  <number>       set first up-counting number (default=0)\n");
        fprintf(stream, "  <interface>    CAN interface board (list all with /LIST)\n");
        fprintf(stream, "  <baudrate>     CAN baud rate index (default=3):\n");
        fprintf(stream, "                 0 = 1000 kbps\n");
        fprintf(stream, "                 1 = 800 kbps\n");
        fprintf(stream, "                 2 = 500 kbps\n");
        fprintf(stream, "                 3 = 250 kbps\n");
        fprintf(stream, "                 4 = 125 kbps\n");
        fprintf(stream, "                 5 = 100 kbps\n");
        fprintf(stream, "                 6 = 50 kbps\n");
        fprintf(stream, "                 7 = 20 kbps\n");
        fprintf(stream, "                 8 = 10 kbps\n");
        fprintf(stream, "  <bitrate>      comma-separated key/value list:\n");
        fprintf(stream, "                 f_clock=<value>      frequency in Hz or\n");
        fprintf(stream, "                 f_clock_mhz=<value>  frequency in MHz\n");
        fprintf(stream, "                 nom_brp=<value>      bit-rate prescaler (nominal)\n");
        fprintf(stream, "                 nom_tseg1=<value>    time segment 1 (nominal)\n");
        fprintf(stream, "                 nom_tseg2=<value>    time segment 2 (nominal)\n");
        fprintf(stream, "                 nom_sjw=<value>      sync. jump width (nominal)\n");
        fprintf(stream, "                 nom_sam=<value>      sampling (only SJA1000)\n");
        fprintf(stream, "                 data_brp=<value>     bit-rate prescaler (FD data)\n");
        fprintf(stream, "                 data_tseg1=<value>   time segment 1 (FD data)\n");
        fprintf(stream, "                 data_tseg2=<value>   time segment 2 (FD data)\n");
        fprintf(stream, "                 data_sjw=<value>     sync. jump width (FD data).\n");
    }
    fprintf(stream, "Hazard note:\n");
    fprintf(stream, "  If you connect your CAN device to a real CAN network when using this program,\n");
    fprintf(stream, "  you might damage your application.\n");
}

void SOptions::ShowHelp(FILE* stream) {
    ShowGreetings(stream);
    ShowUsage(stream);
}
