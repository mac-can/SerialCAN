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
#include <errno.h>
extern "C" {
#include "dosopt.h"
}
#ifdef _MSC_VER
//not #if defined(_WIN32) || defined(_WIN64) because we have strncasecmp in mingw
#define strncasecmp _strnicmp
#define strcasecmp _stricmp
#endif
#define USE_BASENAME  0

#define DEFAULT_OP_MODE   CANMODE_DEFAULT
#define DEFAULT_BAUDRATE  CANBTR_INDEX_250K
#define DEFAULT_CAN_ID    0x100
#define DEFAULT_LENGTH    8
#define BAUDRATE_STR      0
#define BAUDRATE_CHR      1
#define BITRATE_STR       2
#define BITRATE_CHR       3
#define VERBOSE_STR       4
#define VERBOSE_CHR       5
#define OP_MODE_STR       6
#define OP_MODE_CHR       7
#define OP_RTR_STR        8
#define OP_XTD_STR        9
#define OP_ERR_STR        10
#define OP_ERRFRMS_STR    11
#define OP_MON_STR        12
#define OP_MONITOR_STR    13
#define OP_LSTNONLY_STR   14
#define OP_SHARED_STR     15
#define OP_SHARED_CHR     16
#define STD_CODE_STR      17
#define STD_MASK_CHR      18
#define XTD_CODE_STR      19
#define XTD_MASK_CHR      20
#define RECEIVE_STR       21
#define RECEIVE_CHR       22
#define NUMBER_STR        23
#define NUMBER_CHR        24
#define STOP_STR          25
#define STOP_CHR          26
#define TRANSMIT_STR      27
#define TRANSMIT_CHR      28
#define FRAMES_STR        29
#define FRAMES_CHR        30
#define RANDOM_STR        31
#define RANDOM_CHR        32
#define CYCLE_STR         33
#define CYCLE_CHR         34
#define USEC_STR          35
#define USEC_CHR          36
#define DLC_STR           37
#define DLC_CHR           38
#define DLC_LEN           39
#define CAN_STR           40
#define CAN_CHR           41
#define CAN_ID            42
#define COB_ID            43
#define LISTBITRATES_STR  44
#define LISTBOARDS_STR    45
#define LISTBOARDS_CHR    46
#define TESTBOARDS_STR    47
#define TESTBOARDS_CHR    48
#define JSON_STR          49
#define JSON_CHR          50
#define HELP              51
#define QUESTION_MARK     52
#define ABOUT             53
#define CHARACTER_MJU     54
#define VERSION           55
#define MAX_OPTIONS       56

static char* option[MAX_OPTIONS] = {
    (char*)"BAUDRATE", (char*)"bd",
    (char*)"BITRATE", (char*)"br",
    (char*)"VERBOSE", (char*)"v",
    (char*)"MODE", (char*)"m",
    (char*)"RTR",
    (char*)"XTD",
    (char*)"ERR", (char*)"ERROR-FRAMES",
    (char*)"MON", (char*)"MONITOR", (char*)"LISTEN-ONLY",
    (char*)"SHARED", (char*)"shrd",
    (char*)"CODE", (char*)"MASK",
    (char*)"XTD-CODE", (char*)"XTD-MASK",
    (char*)"RECEIVE", (char*)"rx",
    (char*)"NUMBER", (char*)"n",
    (char*)"STOP", (char*)"s",
    (char*)"TRANSMIT", (char*)"tx",
    (char*)"FRAMES", (char*)"fr",
    (char*)"RANDOM", (char*)"rand",
    (char*)"CYCLE", (char*)"c",
    (char*)"USEC", (char*)"u",
    (char*)"DLC", (char*)"d", (char*)"DATA",
    (char*)"CAN-ID", (char*)"id", (char*)"i", (char*)"COP-ID",
    (char*)"LIST-BITRATES",
    (char*)"LIST-BOARDS", (char*)"list",
    (char*)"TEST-BOARDS", (char*)"test",
#if (OPTION_CANAPI_LIBRARY != 0)
    (char*)"PATH", (char*)"p",
#else
    (char*)"JSON-FILE", (char*)"json",
#endif
    (char*)"HELP", (char*)"?",
    (char*)"ABOUT", (char*)"\xB5",
    (char*)"VERSION"
};
static const char* c_szApplication = CAN_TEST_APPLICATION;
static const char* c_szCopyright = CAN_TEST_COPYRIGHT;
static const char* c_szWarranty = CAN_TEST_WARRANTY;
static const char* c_szLicense = CAN_TEST_LICENSE;
static const char* c_szBasename = CAN_TEST_PROGRAM;
static const char* c_szInterface = "(unknown)";

#if (USE_BASENAME != 0)
static char* basename(char* path);
#endif

SOptions::SOptions() {
    // to have dault bus speed from bit-timing index
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
    int optind;
    char* optarg;
    int64_t intarg;

    int argInterface = 0;
    int optBitrate = 0;
    int optVerbose = 0;
    int optMode = 0;
    int optShared = 0;
    int optListenOnly = 0;
    int optErrorFrames = 0;
    int optExtendedFrames = 0;
    int optRemoteFrames = 0;
    int optCode = 0;
    int optMask = 0;
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
    // (0) sanity check
    if ((argc <= 0) || (argv == NULL))
        return (-1);
    if ((err == NULL) || (out == NULL))
        return (-1);
    // (1) get basename from command-line
#if (USE_BASENAME != 0)
    m_szBasename = Basename(argv[0]);
#endif
    // (2) scan command-line for options
    while ((optind = getOption(argc, (char**)argv, MAX_OPTIONS, option)) != EOF) {
        switch (optind) {
        /* option '--baudrate=<baudrate>' (-b) */
        case BAUDRATE_STR:
        case BAUDRATE_CHR:
            if ((optBitrate++)) {
                fprintf(err, "%s: duplicated option /BAUDRATE\n", m_szBasename);
                return 1;
            }
            if ((optarg = getOptionParameter()) == NULL) {
                fprintf(err, "%s: missing argument for option /BAUDRATE\n", m_szBasename);
                return 1;
            }
            if (sscanf_s(optarg, "%lli", &intarg) != 1) {
                fprintf(err, "%s: illegal argument for option /BAUDRATE\n", m_szBasename);
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
                fprintf(err, "%s: illegal argument for option /BAUDRATE\n", m_szBasename);
                return 1;
            }
            if (CCanDriver::MapBitrate2Speed(bitrate, m_BusSpeed) != CCanApi::NoError) {
                fprintf(err, "%s: illegal argument for option /BAUDRATE\n", m_szBasename);
                return 1;
            }
            break;
        /* option '--bitrate=<bit-rate>' as string */
        case BITRATE_STR:
        case BITRATE_CHR:
            if ((optBitrate++)) {
                fprintf(err, "%s: duplicated option /BITRATE\n", m_szBasename);
                return 1;
            }
            if ((optarg = getOptionParameter()) == NULL) {
                fprintf(err, "%s: missing argument for option /BITRATE\n", m_szBasename);
                return 1;
            }
            if (CCanDriver::MapString2Bitrate(optarg, m_Bitrate, m_bHasDataPhase, m_bHasNoSamp) != CCanApi::NoError) {
                fprintf(err, "%s: illegal argument for option /BITRATE\n", m_szBasename);
                return 1;
            }
            if (CCanDriver::MapBitrate2Speed(m_Bitrate, m_BusSpeed) != CCanApi::NoError) {
                fprintf(err, "%s: illegal argument for option /BITRATE\n", m_szBasename);
                return 1;
            }
            break;
        /* option '--verbose' (-v) */
        case VERBOSE_STR:
        case VERBOSE_CHR:
            if (optVerbose) {
                fprintf(err, "%s: duplicated option /VERBOSE\n", m_szBasename);
                return 1;
            }
            if ((optarg = getOptionParameter()) != NULL) {
                fprintf(err, "%s: illegal argument for option /VERBOSE\n", m_szBasename);
                return 1;
            }
            m_fVerbose = true;
            break;
#if (OPTION_CANAPI_LIBRARY != 0)
        /* option '--path' (-p) */
        case JSON_STR:
        case JSON_CHR:
            if ((optPath++)) {
                fprintf(err, "%s: duplicated option /PATH\n", m_szBasename);
                return 1;
            }
            if ((optarg = getOptionParameter()) == NULL) {
                fprintf(err, "%s: missing argument for option /PATH\n", m_szBasename);
                return 1;
            }
            m_szSearchPath = optarg;
            break;
#endif
        /* option '--mode=(2.0|FDF[+BRS])' (-m) */
        case OP_MODE_STR:
        case OP_MODE_CHR:
            if ((optMode++)) {
                fprintf(err, "%s: duplicated option /MODE\n", m_szBasename);
                return 1;
            }
            if ((optarg = getOptionParameter()) == NULL) {
                fprintf(err, "%s: missing argument for option /MODE\n", m_szBasename);
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
                fprintf(err, "%s: illegal argument for option /MODE\n", m_szBasename);
                return 1;
            }
            break;
        /* option '--shared' */
        case OP_SHARED_STR:
        case OP_SHARED_CHR:
            if ((optShared++)) {
                fprintf(err, "%s: duplicated option /SHARED\n", m_szBasename);
                return 1;
            }
            if ((optarg = getOptionParameter()) != NULL) {
                fprintf(err, "%s: illegal argument for option /SHARED\n", m_szBasename);
                return 1;
            }
            m_OpMode.byte |= CANMODE_SHRD;
            break;
        /* option '--listen-only' */
        case OP_MON_STR:
        case OP_MONITOR_STR:
            if ((optListenOnly++)) {
                fprintf(err, "%s: duplicated option /MON\n", m_szBasename);
                return 1;
            }
            if ((optarg = getOptionParameter()) == NULL) {
                fprintf(err, "%s: missing argument for option /MON\n", m_szBasename);
                return 1;
            }
            if (!strcasecmp(optarg, "YES") || !strcasecmp(optarg, "Y") || !strcasecmp(optarg, "ON") || !strcasecmp(optarg, "1"))
                m_OpMode.byte |= CANMODE_MON;
            else if (!strcasecmp(optarg, "NO") || !strcasecmp(optarg, "N") || !strcasecmp(optarg, "OFF") || !strcasecmp(optarg, "0"))
                m_OpMode.byte &= ~CANMODE_MON;
            else {
                fprintf(err, "%s: illegal argument for option /MON\n", m_szBasename);
                return 1;
            }
            break;
        case OP_LSTNONLY_STR:
            if ((optListenOnly++)) {
                fprintf(err, "%s: duplicated option /LISTEN-ONLY\n", m_szBasename);
                return 1;
            }
            if ((optarg = getOptionParameter()) != NULL) {
                fprintf(err, "%s: illegal argument for option /LISTEN-ONLY\n", m_szBasename);
                return 1;
            }
            m_OpMode.byte |= CANMODE_MON;
            break;
        /* option '--error-frames' */
        case OP_ERR_STR:
            if ((optErrorFrames++)) {
                fprintf(err, "%s: duplicated option /ERR\n", m_szBasename);
                return 1;
            }
            if ((optarg = getOptionParameter()) == NULL) {
                fprintf(err, "%s: missing argument for option /ERR\n", m_szBasename);
                return 1;
            }
            if (!strcasecmp(optarg, "YES") || !strcasecmp(optarg, "Y") || !strcasecmp(optarg, "ON") || !strcasecmp(optarg, "1"))
                m_OpMode.byte |= CANMODE_ERR;
            else if (!strcasecmp(optarg, "NO") || !strcasecmp(optarg, "N") || !strcasecmp(optarg, "OFF") || !strcasecmp(optarg, "0"))
                m_OpMode.byte &= ~CANMODE_ERR;
            else {
                fprintf(err, "%s: illegal argument for option /ERR\n", m_szBasename);
                return 1;
            }
            break;
        case OP_ERRFRMS_STR:
            if ((optErrorFrames++)) {
                fprintf(err, "%s: duplicated option /ERROR-FRAMES\n", m_szBasename);
                return 1;
            }
            if ((optarg = getOptionParameter()) != NULL) {
                fprintf(err, "%s: illegal argument for option /ERROR-FRAMES\n", m_szBasename);
                return 1;
            }
            m_OpMode.byte |= CANMODE_ERR;
            break;
        /* option '--no-extended-frames' */
        case OP_XTD_STR:
            if ((optExtendedFrames++)) {
                fprintf(err, "%s: duplicated option /XTD\n", m_szBasename);
                return 1;
            }
            if ((optarg = getOptionParameter()) == NULL) {
                fprintf(err, "%s: missing argument for option /XTD\n", m_szBasename);
                return 1;
            }
            if (!strcasecmp(optarg, "NO") || !strcasecmp(optarg, "N") || !strcasecmp(optarg, "OFF") || !strcasecmp(optarg, "0"))
                m_OpMode.byte |= CANMODE_NXTD;
            else if (!strcasecmp(optarg, "YES") || !strcasecmp(optarg, "Y") || !strcasecmp(optarg, "ON") || !strcasecmp(optarg, "1"))
                m_OpMode.byte &= ~CANMODE_NXTD;
            else {
                fprintf(err, "%s: illegal argument for option /XTD\n", m_szBasename);
                return 1;
            }
            break;
        /* option '--no-remote-frames' */
        case OP_RTR_STR:
            if ((optRemoteFrames++)) {
                fprintf(err, "%s: duplicated option /RTR\n", m_szBasename);
                return 1;
            }
            if ((optarg = getOptionParameter()) == NULL) {
                fprintf(err, "%s: missing argument for option /RTR\n", m_szBasename);
                return 1;
            }
            if (!strcasecmp(optarg, "NO") || !strcasecmp(optarg, "N") || !strcasecmp(optarg, "OFF") || !strcasecmp(optarg, "0"))
                m_OpMode.byte |= CANMODE_NRTR;
            else if (!strcasecmp(optarg, "YES") || !strcasecmp(optarg, "Y") || !strcasecmp(optarg, "ON") || !strcasecmp(optarg, "1"))
                m_OpMode.byte &= ~CANMODE_NRTR;
            else {
                fprintf(err, "%s: illegal argument for option /RTR\n", m_szBasename);
                return 1;
            }
            break;
        /* option '--code=<11-bit-code>' */
        case STD_CODE_STR:
            if ((optCode++)) {
                fprintf(err, "%s: duplicated option /CODE\n", m_szBasename);
                return 1;
            }
            if ((optarg = getOptionParameter()) == NULL) {
                fprintf(err, "%s: missing argument for option /CODE\n", m_szBasename);
                return 1;
            }
            if (sscanf_s(optarg, "%lli", &intarg) != 1) {
                fprintf(err, "%s: illegal argument for option /CODE\n", m_szBasename);
                return 1;
            }
            if ((intarg & ~CAN_MAX_STD_ID) != 0) {
                fprintf(err, "%s: illegal argument for option /CODE\n", m_szBasename);
                return 1;
            }
            m_StdFilter.m_u32Code = (uint32_t)intarg;
            break;
        /* option '--mask=<11-bit-mask>' */
        case STD_MASK_CHR:
            if ((optMask++)) {
                fprintf(err, "%s: duplicated option /MASK\n", m_szBasename);
                return 1;
            }
            if ((optarg = getOptionParameter()) == NULL) {
                fprintf(err, "%s: missing argument for option /MASK\n", m_szBasename);
                return 1;
            }
            if (sscanf_s(optarg, "%lli", &intarg) != 1) {
                fprintf(err, "%s: illegal argument for option /MASK\n", m_szBasename);
                return 1;
            }
            if ((intarg & ~CAN_MAX_STD_ID) != 0) {
                fprintf(err, "%s: illegal argument for option /MASK\n", m_szBasename);
                return 1;
            }
            m_StdFilter.m_u32Mask = (uint32_t)intarg;
            break;
        /* option '--xtd-code=<29-bit-code>' */
        case XTD_CODE_STR:
            if ((optXtdCode++)) {
                fprintf(err, "%s: duplicated option /XTD-CODE\n", m_szBasename);
                return 1;
            }
            if ((optarg = getOptionParameter()) == NULL) {
                fprintf(err, "%s: missing argument for option /XTD-CODE\n", m_szBasename);
                return 1;
            }
            if (sscanf_s(optarg, "%lli", &intarg) != 1) {
                fprintf(err, "%s: illegal argument for option /XTD-CODE\n", m_szBasename);
                return 1;
            }
            if ((intarg & ~CAN_MAX_XTD_ID) != 0) {
                fprintf(err, "%s: illegal argument for option /XTD-CODE\n", m_szBasename);
                return 1;
            }
            m_XtdFilter.m_u32Code = (uint32_t)intarg;
            break;
        /* option '--xtd-mask=<29-bit-mask>' */
        case XTD_MASK_CHR:
            if ((optXtdMask++)) {
                fprintf(err, "%s: duplicated option /XTD-MASK\n", m_szBasename);
                return 1;
            }
            if ((optarg = getOptionParameter()) == NULL) {
                fprintf(err, "%s: missing argument for option /XTD-MASK\n", m_szBasename);
                return 1;
            }
            if (sscanf_s(optarg, "%lli", &intarg) != 1) {
                fprintf(err, "%s: illegal argument for option /XTD-MASK\n", m_szBasename);
                return 1;
            }
            if ((intarg & ~CAN_MAX_XTD_ID) != 0) {
                fprintf(err, "%s: illegal argument for option /XTD-MASK\n", m_szBasename);
                return 1;
            }
            m_XtdFilter.m_u32Mask = (uint32_t)intarg;
            break;
        /* option '--receive' (-r) */
        case RECEIVE_STR:
        case RECEIVE_CHR:
            if ((optReceive++)) {
                fprintf(err, "%s: duplicated option /RECEIVE\n", m_szBasename);
                return 1;
            }
            if ((optarg = getOptionParameter()) != NULL) {
                fprintf(err, "%s: illegal argument for option /RECEIVE\n", m_szBasename);
                return 1;
            }
            m_TestMode = ETestMode::RxMODE;
            break;
        /* option '--number=<offset>' (-n) */
        case NUMBER_STR:
        case NUMBER_CHR:
            if (optNumber++) {
                fprintf(err, "%s: duplicated option /NUMBER\n", m_szBasename);
                return 1;
            }
            if ((optarg = getOptionParameter()) == NULL) {
                fprintf(err, "%s: missing argument for option /NUMBER\n", m_szBasename);
                return 1;
            }
            if (sscanf_s(optarg, "%lli", &intarg) != 1) {
                fprintf(err, "%s: illegal argument for option /NUMBER\n", m_szBasename);
                return 1;
            }
            if (intarg < 0) {
                fprintf(err, "%s: illegal argument for option /NUMBER\n", m_szBasename);
                return 1;
            }
            m_nStartNumber = (uint64_t)intarg;
            m_fCheckNumber = true;
            break;
        /* option '--stop' (-s) */
        case STOP_STR:
        case STOP_CHR:
            if ((optStop++)) {
                fprintf(err, "%s: duplicated option /STOP\n", m_szBasename);
                return 1;
            }
            if ((optarg = getOptionParameter()) != NULL) {
                fprintf(err, "%s: illegal argument for option /STOP\n", m_szBasename);
                return 1;
            }
            m_fStopOnError = 1;
            break;
        /* option '--transmit=<duration>' (-t) in [s] */
        case TRANSMIT_STR:
        case TRANSMIT_CHR:
            if ((optTransmit++)) {
                fprintf(err, "%s: duplicated option /TRANSMIT\n", m_szBasename);
                return 1;
            }
            if ((optarg = getOptionParameter()) == NULL) {
                fprintf(err, "%s: missing argument for option /TRANSMIT\n", m_szBasename);
                return 1;
            }
            if (sscanf_s(optarg, "%lli", &intarg) != 1) {
                fprintf(err, "%s: illegal argument for option /TRANSMIT\n", m_szBasename);
                return 1;
            }
            if (intarg < 0) {
                fprintf(err, "%s: illegal argument for option /TRANSMIT\n", m_szBasename);
                return 1;
            }
            m_nTxTime = (time_t)intarg;
            m_TestMode = ETestMode::TxMODE;
            break;
        /* option '--frames=<frames>' (-f) */
        case FRAMES_STR:
        case FRAMES_CHR:
            if ((optFrames++)) {
                fprintf(err, "%s: duplicated option /FRAMES\n", m_szBasename);
                return 1;
            }
            if ((optarg = getOptionParameter()) == NULL) {
                fprintf(err, "%s: missing argument for option /FRAMES\n", m_szBasename);
                return 1;
            }
            if (sscanf_s(optarg, "%lli", &intarg) != 1) {
                fprintf(err, "%s: illegal argument for option /FRAMES\n", m_szBasename);
                return 1;
            }
            if (intarg < 0) {
                fprintf(err, "%s: illegal argument for option /FRAMES\n", m_szBasename);
                return 1;
            }
            m_nTxFrames = (uint64_t)intarg;
            m_TestMode = ETestMode::TxFRAMES;
            break;
        /* option '--random=<frames>' */
        case RANDOM_STR:
        case RANDOM_CHR:
            if ((optRandom++)) {
                fprintf(err, "%s: duplicated option /RANDOM\n", m_szBasename);
                return 1;
            }
            if ((optarg = getOptionParameter()) == NULL) {
                fprintf(err, "%s: missing argument for option /RANDOM\n", m_szBasename);
                return 1;
            }
            if (sscanf_s(optarg, "%lli", &intarg) != 1) {
                fprintf(err, "%s: illegal argument for option /RANDOM\n", m_szBasename);
                return 1;
            }
            if (intarg < 0) {
                fprintf(err, "%s: illegal argument for option /RANDOM\n", m_szBasename);
                return 1;
            }
            if (!optDlc) /* let the tester generate messages of arbitrary length */
                m_nTxCanDlc = (uint8_t)0;
            m_nTxFrames = (uint64_t)intarg;
            m_TestMode = ETestMode::TxRANDOM;
            break;
        /* option '--cycle=<msec>' (-c) */
        case CYCLE_STR:
        case CYCLE_CHR:
            if ((optCycle++)) {
                fprintf(err, "%s: duplicated option /CYCLE\n", m_szBasename);
                return 1;
            }
            if ((optarg = getOptionParameter()) == NULL) {
                fprintf(err, "%s: missing argument for option /CYCLE\n", m_szBasename);
                return 1;
            }
            if (sscanf_s(optarg, "%lli", &intarg) != 1) {
                fprintf(err, "%s: illegal argument for option /CYCLE\n", m_szBasename);
                return 1;
            }
            if ((intarg < 0) || (intarg > (INT_MAX / 1000))) {
                fprintf(err, "%s: illegal argument for option /CYCLE\n", m_szBasename);
                return 1;
            }
            m_nTxDelay = (uint64_t)(intarg * 1000l);
            break;
        /* option '--usec=<usec>' (-u) */
        case USEC_STR:
        case USEC_CHR:
            if ((optCycle++)) {
                fprintf(err, "%s: duplicated option /USEC\n", m_szBasename);
                return 1;
            }
            if ((optarg = getOptionParameter()) == NULL) {
                fprintf(err, "%s: missing argument for option /USEC\n", m_szBasename);
                return 1;
            }
            if (sscanf_s(optarg, "%lli", &intarg) != 1) {
                fprintf(err, "%s: illegal argument for option /USEC\n", m_szBasename);
                return 1;
            }
            if (intarg < 0) {
                fprintf(err, "%s: illegal argument for option /USEC\n", m_szBasename);
                return 1;
            }
            m_nTxDelay = (uint64_t)intarg;
            break;
        /* option '--dlc=<length>' (-d) */
        case DLC_STR:
        case DLC_CHR:
        case DLC_LEN:
            if ((optDlc++)) {
                fprintf(err, "%s: duplicated option /DLC\n", m_szBasename);
                return 1;
            }
            if ((optarg = getOptionParameter()) == NULL) {
                fprintf(err, "%s: missing argument for option /DLC\n", m_szBasename);
                return 1;
            }
            if (sscanf_s(optarg, "%lli", &intarg) != 1) {
                fprintf(err, "%s: illegal argument for option /DLC\n", m_szBasename);
                return 1;
            }
#if (CAN_FD_SUPPORTED != 0)
            if ((intarg < 0) || (CANFD_MAX_LEN < intarg)) {
#else
            if ((intarg < 0) || (CAN_MAX_LEN < intarg)) {
#endif
                fprintf(err, "%s: illegal argument for option /DLC\n", m_szBasename);
                return 1;
            }
            m_nTxCanDlc = (uint8_t)intarg;
            break;
        /* option '--id=<identifier>' (-i) */
        case CAN_STR:
        case CAN_CHR:
        case CAN_ID:
        case COB_ID:
            if ((optId++)) {
                fprintf(err, "%s: duplicated option /CAN-ID\n", m_szBasename);
                return 1;
            }
            if ((optarg = getOptionParameter()) == NULL) {
                fprintf(err, "%s: missing argument for option /CAN-ID\n", m_szBasename);
                return 1;
            }
            if (sscanf_s(optarg, "%lli", &intarg) != 1) {
                fprintf(err, "%s: illegal argument for option /CAN-ID\n", m_szBasename);
                return 1;
            }
            if ((intarg < 0x000) || (CAN_MAX_XTD_ID < intarg)) { // TODO: to be checked with --mode=NXTD
                fprintf(err, "%s: illegal argument for option /CAN-ID\n", m_szBasename);
                return 1;
            }
            m_nTxCanId = (uint32_t)intarg;
            break;
        /* option '--list-bitrates[=(2.0|FDF[+BRS])]' */
        case LISTBITRATES_STR:
            if ((optListBitrates++)) {
                fprintf(err, "%s: duplicated option /LIST-BITRATES\n", m_szBasename);
                return 1;
            }
            if ((optarg = getOptionParameter()) != NULL) {
                if ((optMode++)) {
                    fprintf(err, "%s: option /MODE already set\n", m_szBasename);
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
                    fprintf(err, "%s: illegal argument for option /LIST-BITRATES\n", m_szBasename);
                    return 1;
                }
            }
            m_fListBitrates = true;
            m_fExit = true;
            break;
        /* option '--list-boards' (-L) */
        case LISTBOARDS_STR:
        case LISTBOARDS_CHR:
            if ((optListBoards++)) {
                fprintf(err, "%s: duplicated option /LIST-BOARDS\n", m_szBasename);
                return 1;
            }
#if (OPTION_CANAPI_LIBRARY != 0)
            if ((optarg = getOptionParameter()) != NULL) {
                if ((optPath++)) {
                    fprintf(err, "%s: option /PATH already set\n", m_szBasename);
                    return 1;
                }
                m_szSearchPath = optarg;
            }
#endif
            m_fListBoards = true;
            m_fExit = true;
            break;
        /* option '--test-boards' (-T) */
        case TESTBOARDS_STR:
        case TESTBOARDS_CHR:
            if ((optTestBoards++)) {
                fprintf(err, "%s: duplicated option /TEST-BOARDS\n", m_szBasename);
                return 1;
            }
#if (OPTION_CANAPI_LIBRARY != 0)
            if ((optarg = getOptionParameter()) != NULL) {
                if ((optPath++)) {
                    fprintf(err, "%s: option /PATH already set\n", m_szBasename);
                    return 1;
                }
                m_szSearchPath = optarg;
            }
#endif
            m_fTestBoards = true;
            m_fExit = true;
            break;
#if (OPTION_CANAPI_LIBRARY == 0)
        /* option '--json=<filename>' (-j) */
        case JSON_STR:
        case JSON_CHR:
            if ((optJson++)) {
                fprintf(err, "%s: duplicated option /JSON-FILE\n", m_szBasename);
                return 1;
            }
            if ((optarg = getOptionParameter()) == NULL) {
                fprintf(err, "%s: missing argument for option /JSON-FILE\n", m_szBasename);
                return 1;
            }
            m_szJsonFilename = optarg;
            m_fExit = true;
            break;
#endif
        /* option '--help' (-h) */
        case HELP:
        case QUESTION_MARK:
            ShowHelp(out);;
            return 1;
        case ABOUT:
        case VERSION:
        case CHARACTER_MJU:
            ShowVersion(out);;
            return 1;
        default:
            ShowUsage(out);;
            return 1;
        }
    }
    // (3) scan command-line for argument <interface>
    for (int i = 1; i < argc; i++) {
        if (!isOption(argc, (char**)argv, MAX_OPTIONS, option, i)) {
            if ((argInterface++)) {
                fprintf(err, "%s: too many arguments\n", m_szBasename);
                return 1;
            }
            m_szInterface = (char*)argv[i];
        }
    }
    // - check if one and only one <interface> is given
    if (!argInterface && !m_fExit) {
        fprintf(err, "%s: no interface given\n", m_szBasename);
        return 1;
    }
    // (4) check for illegal combinations
#if (CAN_FD_SUPPORTED != 0)
    /* - check bit-timing index (n/a for CAN FD) */
    if (m_OpMode.fdoe && (m_Bitrate.btr.frequency <= CANBTR_INDEX_1M) && !m_fExit) {
        fprintf(err, "%s: illegal combination of options /MODE and /BAUDRATE\n", m_szBasename);
        return 1;
    }
    /* - check data length and make CAN FD DLC (0x0..0xF) */
    if (!m_OpMode.fdoe && (m_nTxCanDlc > CAN_MAX_LEN) && !m_fExit) {
        fprintf(err, "%s: illegal combination of options /MODE and /DLC\n", m_szBasename);
        return 1;
    }
    else {
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
    if ((m_TestMode != ETestMode::RxMODE) && m_OpMode.mon && !m_fExit) {
        fprintf(err, "%s: illegal option /MON:YES alias /LISTEN-ONLY for transmitter test\n", m_szBasename);
        return 1;
    }
    if ((m_TestMode != ETestMode::RxMODE) && m_OpMode.err && !m_fExit) {
        fprintf(err, "%s: illegal option /ERR:YES alias /ERROR-FRAMES for transmitter test\n", m_szBasename);
        return 1;
    }
    if ((m_TestMode != ETestMode::RxMODE) && m_OpMode.nxtd && !m_fExit) {
        fprintf(err, "%s: illegal option /XTD:NO for transmitter test\n", m_szBasename);
        return 1;
    }
    if ((m_TestMode != ETestMode::RxMODE) && m_OpMode.nrtr && !m_fExit) {
        fprintf(err, "%s: illegal option /RTR:NO for transmitter test\n", m_szBasename);
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
    fprintf(stream, "  /RECEIVE | /RX                      count received messages until ^C is pressed\n");
    fprintf(stream, "  /Number:<number>                    check up-counting numbers starting with <number>\n");
    fprintf(stream, "  /Stop                               stop on error (with option /NUMBER)\n");
#if (CAN_FD_SUPPORTED != 0)
    fprintf(stream, "  /Mode:(2.0|FDf[+BRS])               CAN operation mode: CAN 2.0 or CAN FD mode\n");
#else
    fprintf(stream, "  /Mode:2.0                           CAN operation mode: CAN 2.0\n");
#endif
    fprintf(stream, "  /SHARED                             shared CAN controller access (if supported)\n");
    fprintf(stream, "  /MONitor:(No|Yes) | /LISTEN-ONLY    monitor mode (listen-only mode)\n");
    fprintf(stream, "  /ERR:(No|Yes) | /ERROR-FRAMES       allow reception of error frames\n");
    fprintf(stream, "  /RTR:(Yes|No)                       allow remote frames (RTR frames)\n");
    fprintf(stream, "  /XTD:(Yes|No)                       allow extended frames (29-bit identifier)\n");
    fprintf(stream, "  /CODE:<id>                          acceptance code for 11-bit IDs (default=0x%03lx)\n", CANACC_CODE_11BIT);
    fprintf(stream, "  /MASK:<id>                          acceptance mask for 11-bit IDs (default=0x%03lx)\n", CANACC_MASK_11BIT);
    fprintf(stream, "  /XTD-CODE:<id>                      acceptance code for 29-bit IDs (default=0x%08lx)\n", CANACC_CODE_29BIT);
    fprintf(stream, "  /XTD-MASK:<id>                      acceptance mask for 29-bit IDs (default=0x%08lx)\n", CANACC_MASK_29BIT);
    fprintf(stream, "  /BauDrate:<baudrate>                CAN bit-timing in kbps (default=250), or\n");
    fprintf(stream, "  /BitRate:<bitrate>                  CAN bit-rate settings (as key/value list)\n");
#if (OPTION_CANAPI_LIBRARY != 0)
    fprintf(stream, "  /Path:<pathname>                    search path for JSON configuration files\n");
#endif
    fprintf(stream, "  /Verbose                            show detailed bit-rate settings\n");
    fprintf(stream, "Options for transmitter test:\n");
    fprintf(stream, "  /TRANSMIT:<time> | /TX=<time>       send messages for the given time in seconds, or\n");
    fprintf(stream, "  /FRames:<frames>                    alternatively send the given number of messages, or\n");
    fprintf(stream, "  /RANDom:<frames>                    optionally with random cycle time and data length\n");
    fprintf(stream, "  /Cycle:<msec>                       cycle time in milliseconds (default=0), or\n");
    fprintf(stream, "  /Usec:<usec>                        cycle time in microseconds (default=0)\n");
    fprintf(stream, "  /Dlc:<length>                       send messages of given length (default=8)\n");
    fprintf(stream, "  /can-Id:<can-id>                    use given identifier (default=100h)\n");
    fprintf(stream, "  /Number:<number>                    set first up-counting number (default=0)\n");
#if (CAN_FD_SUPPORTED != 0)
    fprintf(stream, "  /Mode:(2.0|FDf[+BRS])               CAN operation mode: CAN 2.0 or CAN FD mode\n");
#else
    fprintf(stream, "  /Mode:2.0                           CAN operation mode: CAN 2.0\n");
#endif
    fprintf(stream, "  /SHARED                             shared CAN controller access (if supported)\n");
    fprintf(stream, "  /BauDrate:<baudrate>                CAN bit-timing in kbps (default=250), or\n");
    fprintf(stream, "  /BitRate:<bitrate>                  CAN bit-rate settings (as key/value list)\n");
#if (OPTION_CANAPI_LIBRARY != 0)
    fprintf(stream, "  /Path:<pathname>                    search path for JSON configuration files\n");
#endif
    fprintf(stream, "  /Verbose                            show detailed bit-rate settings\n");
    fprintf(stream, "Other options:\n");
#if (CAN_FD_SUPPORTED != 0)
    fprintf(stream, "  /LIST-BITRATES[:(2.0|FDf[+BRS])]    list standard bit-rate settings and exit\n");
#else
    fprintf(stream, "  /LIST-BITRATES[:2.0]                list standard bit-rate settings and exit\n");
#endif
#if (OPTION_CANAPI_LIBRARY != 0)
    fprintf(stream, "  /LIST-boards[:<pathname>]           list all supported CAN interfaces and exit\n");
    fprintf(stream, "  /TEST-boards[:<pathname>]           list all available CAN interfaces and exit\n");
#else
    fprintf(stream, "  /LIST-BOARDS | /LIST                list all supported CAN interfaces and exit\n");
    fprintf(stream, "  /TEST-BOARDS | /TEST                list all available CAN interfaces and exit\n");
    fprintf(stream, "  /JSON-file:<filename>               write configuration into JSON file and exit\n");
#endif
    fprintf(stream, "  /HELP | /?                          display this help screen and exit\n");
    fprintf(stream, "  /VERSION                            show version information and exit\n");
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

#if (USE_BASENAME != 0)
/* see man basename(3) */
static char* basename(char* path) {
    static char exe[] = "agimus.exe";
    char* ptr = NULL;
    if (path)
        ptr = strrchr(path, '\\');
    if (ptr)
        ptr++;
    return (ptr ? ptr : exe);
}
#endif
