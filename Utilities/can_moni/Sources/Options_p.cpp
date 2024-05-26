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
#include "Options.h"
#include "Message.h"

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <getopt.h>
#include <libgen.h>
#include <inttypes.h>
#include <errno.h>

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

static const char* c_szApplication = CAN_MONI_APPLICATION;
static const char* c_szCopyright = CAN_MONI_COPYRIGHT;
static const char* c_szWarranty = CAN_MONI_WARRANTY;
static const char* c_szLicense = CAN_MONI_LICENSE;
static const char* c_szBasename = CAN_MONI_PROGRAM;
static const char* c_szInterface = "(unknown)";
static const char* c_szExcludeList = "~0x00-0x7FF";

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
    m_szExcludeList = (char*)c_szExcludeList;
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
    int optFmtTime = 0;
    int optFmtId = 0;
    int optFmtData = 0;
    int optFmtAscii = 0;
#if (CAN_FD_SUPPORTED != 0)
    int optFmtWrap = 0;
#endif
    int optExclude = 0;
    int optListBitrates = 0;
    int optListBoards = 0;
    int optTestBoards = 0;
#if (OPTION_CANAPI_LIBRARY != 0)
    int optPath = 0;
#else
    int optJson = 0;
#endif
    // default format options
    CCanMessage::EFormatTimestamp fmtModeTime = CCanMessage::OptionZero;
    CCanMessage::EFormatNumber fmtModeId = CCanMessage::OptionHex;
    CCanMessage::EFormatNumber fmtModeData = CCanMessage::OptionHex;
    CCanMessage::EFormatOption fmtModeAscii = CCanMessage::OptionOn;
    CCanMessage::EFormatWraparound fmtWraparound = CCanMessage::OptionWraparoundNo;
    (void)CCanMessage::SetTimestampFormat(fmtModeTime);
    (void)CCanMessage::SetIdentifierFormat(fmtModeId);
    (void)CCanMessage::SetDataFormat(fmtModeData);
    (void)CCanMessage::SetAsciiFormat(fmtModeAscii);
    (void)CCanMessage::SetWraparound(fmtWraparound);

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
        {"time", required_argument, 0, 't'},
        {"id", required_argument, 0, 'i'},
        {"data", required_argument, 0, 'd'},
        {"ascii", required_argument, 0, 'a'},
        {"wrap", required_argument, 0, 'w'},
        {"wraparound", required_argument, 0, 'w'},
        {"exclude", required_argument, 0, 'x'},
        {"script", required_argument, 0, 's'},
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
    while ((opt = getopt_long(argc, (char * const *)argv, "b:vp:m:t:i:d:a:w:x:s:lLTh", long_options, NULL)) != -1) {
#else
    while ((opt = getopt_long(argc, (char * const *)argv, "b:vm:t:i:d:a:w:x:s:lLTj:h", long_options, NULL)) != -1) {
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
        /* option '--time=(ABS|REL|ZERO)' (-t) */
        case 't':
            if (optFmtTime++) {
                fprintf(err, "%s: duplicated option `--time' (%c)\n", m_szBasename, opt);
                return 1;
            }
            if (optarg == NULL) {
                fprintf(err, "%s: missing argument for option `--time' (%c)\n", m_szBasename, opt);
                return 1;
            }
            if (!strcasecmp(optarg, "ABSOLUTE") || !strcasecmp(optarg, "ABS") || !strcasecmp(optarg, "a"))
                fmtModeTime = CCanMessage::OptionAbsolute;
            else if (!strcasecmp(optarg, "RELATIVE") || !strcasecmp(optarg, "REL") || !strcasecmp(optarg, "r"))
                fmtModeTime = CCanMessage::OptionRelative;
            else if (!strcasecmp(optarg, "ZERO") || !strcasecmp(optarg, "0") || !strcasecmp(optarg, "z"))
                fmtModeTime = CCanMessage::OptionZero;
            else {
                fprintf(err, "%s: illegal argument for option `--time' (%c)\n", m_szBasename, opt);
                return 1;
            }
            if (!CCanMessage::SetTimestampFormat(fmtModeTime)) {
                fprintf(err, "%s: illegal argument for option `--time' (%c)\n", m_szBasename, opt);
                return 1;
            }
            break;
        /* option '--id=(HEX|DEC|OCT)' (-i) */
        case 'i':
            if (optFmtId++) {
                fprintf(err, "%s: duplicated option `--id' (%c)\n", m_szBasename, opt);
                return 1;
            }
            if (optarg == NULL) {
                fprintf(err, "%s: missing argument for option `--id' (%c)\n", m_szBasename, opt);
                return 1;
            }
            if (!strcasecmp(optarg, "HEXADECIMAL") || !strcasecmp(optarg, "HEX") || !strcasecmp(optarg, "h") || !strcasecmp(optarg, "16"))
                fmtModeId = CCanMessage::OptionHex;
            else if (!strcasecmp(optarg, "DECIMAL") || !strcasecmp(optarg, "DEC") || !strcasecmp(optarg, "d") || !strcasecmp(optarg, "10"))
                fmtModeId = CCanMessage::OptionDec;
            else if (!strcasecmp(optarg, "OCTAL") || !strcasecmp(optarg, "OCT") || !strcasecmp(optarg, "o") || !strcasecmp(optarg, "8"))
                fmtModeId = CCanMessage::OptionOct;
            else {
                fprintf(err, "%s: illegal argument for option `--id' (%c)\n", m_szBasename, opt);
                return 1;
            }
            if (!CCanMessage::SetIdentifierFormat(fmtModeId)) {
                fprintf(err, "%s: illegal argument for option `--id' (%c)\n", m_szBasename, opt);
                return 1;
            }
            break;
        /* option '--data=(HEX|DEC|OCT)' (-d) */
        case 'd':
            if (optFmtData++) {
                fprintf(err, "%s: duplicated option `--data' (%c)\n", m_szBasename, opt);
                return 1;
            }
            if (optarg == NULL) {
                fprintf(err, "%s: missing argument for option `--data' (%c)\n", m_szBasename, opt);
                return 1;
            }
            if (!strcasecmp(optarg, "HEXADECIMAL") || !strcasecmp(optarg, "HEX") || !strcasecmp(optarg, "h") || !strcasecmp(optarg, "16"))
                fmtModeData = CCanMessage::OptionHex;
            else if (!strcasecmp(optarg, "DECIMAL") || !strcasecmp(optarg, "DEC") || !strcasecmp(optarg, "d") || !strcasecmp(optarg, "10"))
                fmtModeData = CCanMessage::OptionDec;
            else if (!strcasecmp(optarg, "OCTAL") || !strcasecmp(optarg, "OCT") || !strcasecmp(optarg, "o") || !strcasecmp(optarg, "8"))
                fmtModeData = CCanMessage::OptionOct;
            else {
                fprintf(err, "%s: illegal argument for option `--data' (%c)\n", m_szBasename, opt);
                return 1;
            }
            if (!CCanMessage::SetDataFormat(fmtModeData)) {
                fprintf(err, "%s: illegal argument for option `--data' (%c)\n", m_szBasename, opt);
                return 1;
            }
            break;
        /* option '--ascii=(ON|OFF)' (-a) */
        case 'a':
            if (optFmtAscii++) {
                fprintf(err, "%s: duplicated option `--ascii' (%c)\n", m_szBasename, opt);
                return 1;
            }
            if (optarg == NULL) {
                fprintf(err, "%s: missing argument for option `--ascii' (%c)\n", m_szBasename, opt);
                return 1;
            }
            if (!strcasecmp(optarg, "OFF") || !strcasecmp(optarg, "NO") || !strcasecmp(optarg, "n") || !strcasecmp(optarg, "0"))
                fmtModeAscii = CCanMessage::OptionOff;
            else if (!strcasecmp(optarg, "ON") || !strcasecmp(optarg, "YES") || !strcasecmp(optarg, "y") || !strcasecmp(optarg, "1"))
                fmtModeAscii = CCanMessage::OptionOn;
            else {
                fprintf(err, "%s: illegal argument for option `--ascii' (%c)\n", m_szBasename, opt);
                return 1;
            }
            if (!CCanMessage::SetAsciiFormat(fmtModeAscii)) {
                fprintf(err, "%s: illegal argument for option `--ascii' (%c)\n", m_szBasename, opt);
                return 1;
            }
            break;
#if (CAN_FD_SUPPORTED != 0)
        /* option '--wrap=(No|8|10|16|32|64)' (-w) */
        case 'w':
            if (optFmtWrap++) {
                fprintf(err, "%s: duplicated option `--wrap' (%c)\n", m_szBasename, opt);
                return 1;
            }
            if (optarg == NULL) {
                fprintf(err, "%s: missing argument for option `--wrap' (%c)\n", m_szBasename, opt);
                return 1;
            }
            if (!strcasecmp(optarg, "NO") || !strcasecmp(optarg, "n") || !strcasecmp(optarg, "0"))
                fmtWraparound = CCanMessage::OptionWraparoundNo;
            else if (!strcasecmp(optarg, "8"))
                fmtWraparound = CCanMessage::OptionWraparound8;
            else if (!strcasecmp(optarg, "10"))
                fmtWraparound = CCanMessage::OptionWraparound10;
            else if (!strcasecmp(optarg, "16"))
                fmtWraparound = CCanMessage::OptionWraparound16;
            else if (!strcasecmp(optarg, "32"))
                fmtWraparound = CCanMessage::OptionWraparound32;
            else if (!strcasecmp(optarg, "64"))
                fmtWraparound = CCanMessage::OptionWraparound64;
            else {
                fprintf(err, "%s: illegal argument for option `--wrap' (%c)\n", m_szBasename, opt);
                return 1;
            }
            if (!CCanMessage::SetWraparound(fmtWraparound)) {
                fprintf(err, "%s: illegal argument for option `--wrap' (%c)\n", m_szBasename, opt);
                return 1;
            }
            break;
#endif
        /* option '--exclude=[~]<id-list>' (-x) */
        case 'x':
            if (optExclude++) {
                fprintf(err, "%s: duplicated option `--exclude' (%c)\n", m_szBasename, opt);
                return 1;
            }
            if (optarg == NULL) {
                fprintf(err, "%s: missing argument for option `--exclude' (%c)\n", m_szBasename, opt);
                return 1;
            }
            m_szExcludeList = optarg;
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
#endif
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
    fprintf(stream, "Options:\n");
    fprintf(stream, " -t, --time=(ZERO|ABS|REL)            absolute or relative time (default=0)\n");
    fprintf(stream, " -i  --id=(HEX|DEC|OCT)               display mode of CAN-IDs (default=HEX)\n");
    fprintf(stream, " -d, --data=(HEX|DEC|OCT)             display mode of data bytes (default=HEX)\n");
    fprintf(stream, " -a, --ascii=(ON|OFF)                 display data bytes in ASCII (default=ON)\n");
#if (CAN_FD_SUPPORTED != 0)
    fprintf(stream, " -w, --wrap=(NO|8|10|16|32|64)        wraparound after n data bytes (default=NO)\n");
#endif
    fprintf(stream, " -x, --exclude=[~]<id-list>           exclude CAN-IDs: <id-list> = <id>[-<id>]{,<id>[-<id>]}\n");
    fprintf(stream, "     --code=<id>                      acceptance code for 11-bit IDs (default=0x%03x)\n", CANACC_CODE_11BIT);
    fprintf(stream, "     --mask=<id>                      acceptance mask for 11-bit IDs (default=0x%03x)\n", CANACC_MASK_11BIT);
    fprintf(stream, "     --xtd-code=<id>                  acceptance code for 29-bit IDs (default=0x%08x)\n", CANACC_CODE_29BIT);
    fprintf(stream, "     --xtd-mask=<id>                  acceptance mask for 29-bit IDs (default=0x%08x)\n", CANACC_MASK_29BIT);
//    fprintf(stream, " -s, --script=<filename>              execute a script file\n"); // TODO: script engine
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
        fprintf(stream, "  <id>           CAN identifier (11-bit)\n");
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
