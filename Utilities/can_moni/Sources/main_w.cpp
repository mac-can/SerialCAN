//  SPDX-License-Identifier: GPL-3.0-or-later
//
//  CAN Monitor for generic Interfaces (CAN API V3)
//
//  Copyright (c) 2007,2016-2024 Uwe Vogt, UV Software, Berlin (info@uv-software.com)
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
#include "Timer.h"
#include "Message.h"
#include "Version.h"

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>
extern "C" {
#include "dosopt.h"
}
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
#define MODE_TIME_STR     17
#define MODE_TIME_CHR     18
#define MODE_ID_STR       19
#define MODE_ID_CHR       20
#define MODE_DATA_STR     21
#define MODE_DATA_CHR     22
#define MODE_ASCII_STR    23
#define MODE_ASCII_CHR    24
#define WRAPAROUND_STR    25
#define WRAPAROUND_CHR    26
#define EXCLUDE_STR       27
#define EXCLUDE_CHR       28
#define STD_CODE_STR      29
#define STD_MASK_CHR      30
#define XTD_CODE_STR      31
#define XTD_MASK_CHR      32
#define SCRIPT_STR        33
#define SCRIPT_CHR        34
#define LISTBITRATES_STR  35
#define LISTBOARDS_STR    36
#define LISTBOARDS_CHR    37
#define TESTBOARDS_STR    38
#define TESTBOARDS_CHR    39
#define HELP              40
#define QUESTION_MARK     41
#define ABOUT             42
#define CHARACTER_MJU     43
#define VERSION           44
#define MAX_OPTIONS       45

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
    (char*)"TIME", (char*)"t",
    (char*)"ID", (char*)"i",
    (char*)"DATA", (char*)"d",
    (char*)"ASCII", (char*)"a",
    (char*)"WARAPAROUND", (char*)"w",
    (char*)"EXCLUDE", (char*)"x",
    (char*)"CODE", (char*)"MASK",
    (char*)"XTD-CODE", (char*)"XTD-MASK",
    (char*)"SCRIPT", (char*)"s",
    (char*)"LIST-BITRATES",
    (char*)"LIST-BOARDS", (char*)"list",
    (char*)"TEST-BOARDS", (char*)"test",
    (char*)"HELP", (char*)"?",
    (char*)"ABOUT", (char*)"\xB5",
    (char*)"VERSION"
};

static int get_exclusion(const char *arg);

class CCanDevice : public CCanDriver {
public:
    uint64_t ReceptionLoop();
public:
    static int ListCanDevices(void);
    static int TestCanDevices(CANAPI_OpMode_t opMode);
    static int ListCanBitrates(CANAPI_OpMode_t opMode);
};

static void sigterm(int signo);
static void usage(FILE *stream, const char *program);
static void version(FILE *stream, const char *program);

static int can_id[MAX_ID];
static int can_id_xtd = 1;
static volatile int running = 1;

static CCanDevice canDevice = CCanDevice();

static const char APPLICATION[] = "CAN Monitor for " MONITOR_INTEFACE ", Version " VERSION_STRING;
static const char COPYRIGHT[]   = "Copyright (c) " MONITOR_COPYRIGHT;
static const char WARRANTY[]    = "This program comes with ABSOLUTELY NO WARRANTY!\n\n" \
                                  "This is free software, and you are welcome to redistribute it\n" \
                                  "under certain conditions; type '/VERSION' for details.";
static const char LICENSE[]     = "This program is free software: you can redistribute it and/or modify\n" \
                                  "it under the terms of the GNU General Public License as published by\n" \
                                  "the Free Software Foundation, either version 3 of the License, or\n" \
                                  "(at your option) any later version.\n\n" \
                                  "This program is distributed in the hope that it will be useful,\n" \
                                  "but WITHOUT ANY WARRANTY; without even the implied warranty of\n" \
                                  "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n" \
                                  "GNU General Public License for more details.\n\n" \
                                  "You should have received a copy of the GNU General Public License\n" \
                                  "along with this program.  If not, see <https://www.gnu.org/licenses/>.";
#define basename(x)  "can_moni" // FIXME: Where is my `basename' function?

int main(int argc, const char * argv[]) {
    int i;
    int optind;
    char *optarg;

#if (SERIAL_CAN_SUPPORTED == 0)
    CCanDevice::SChannelInfo channel; int hw = 0;
#else
    char *port = NULL; int hw = 0;
#endif
    int op = 0, rf = 0, xf = 0, ef = 0, lo = 0, sh = 0;
    int c11 = 0, m11 = 0, c29 = 0, m29 = 0;
    int baudrate = CANBDR_250; int bd = 0;
    CCanMessage::EFormatTimestamp modeTime = CCanMessage::OptionZero; int mt = 0;
    CCanMessage::EFormatNumber modeId = CCanMessage::OptionHex; int mi = 0;
    CCanMessage::EFormatNumber modeData = CCanMessage::OptionHex; int md = 0;
    CCanMessage::EFormatOption modeAscii = CCanMessage::OptionOn; int ma = 0;
    CCanMessage::EFormatWraparound wraparound = CCanMessage::OptionWraparoundNo; int mw = 0;
    int exclude = 0;
//    char *script_file = NULL;
    int verbose = 0;
    int num_boards = 0;
    int show_version = 0;
    char *device, *firmware, *software;
    char property[CANPROP_MAX_BUFFER_SIZE] = "";

    CANAPI_Bitrate_t bitrate = {};
    bitrate.index = CANBTR_INDEX_250K;
    bool hasDataPhase = false;
    bool hasNoSamp = true;
    CANAPI_OpMode_t opMode = {};
    opMode.byte = CANMODE_DEFAULT;
    CANAPI_OpMode_t brMode = {};
    brMode.byte = CANMODE_DEFAULT;
    CANAPI_Return_t retVal = 0;

    /* default bit-timing */
    CANAPI_BusSpeed_t speed = {};
    (void)CCanDevice::MapIndex2Bitrate(bitrate.index, bitrate);
    (void)CCanDevice::MapBitrate2Speed(bitrate, speed);
    (void)op;

    /* default format options */
    (void)CCanMessage::SetTimestampFormat(modeTime);
    (void)CCanMessage::SetIdentifierFormat(modeId);
    (void)CCanMessage::SetDataFormat(modeData);
    (void)CCanMessage::SetAsciiFormat(modeAscii);
    (void)CCanMessage::SetWraparound(wraparound);
    (void)mw;

#ifdef ACCEPTANCE_FILTERING
    /* default acceptance filter */
    uint32_t code11 = CANACC_CODE_11BIT, mask11 = CANACC_MASK_11BIT;
    uint32_t code29 = CANACC_CODE_29BIT, mask29 = CANACC_MASK_29BIT;
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
    while ((optind = getOption(argc, (char**)argv, MAX_OPTIONS, option)) != EOF) {
        switch (optind) {
        /* option `--baudrate=<baudrate>' (-b) */
        case BAUDRATE_STR:
        case BAUDRATE_CHR:
            if ((bd++)) {
                fprintf(stderr, "%s: duplicated option /BAUDRATE\n", basename(argv[0]));
                return 1;
            }
            if ((optarg = getOptionParameter()) == NULL) {
                fprintf(stderr, "%s: missing argument for option /BAUDRATE\n", basename(argv[0]));
                return 1;
            }
            if (sscanf_s(optarg, "%li", &baudrate) != 1) {
                fprintf(stderr, "%s: illegal argument for option /BAUDRATE\n", basename(argv[0]));
                return 1;
            }
            switch (baudrate) {
                case 0: case 1000: case 1000000: bitrate.index = (int32_t)CANBTR_INDEX_1M; break;
                case 1: case 800:  case 800000:  bitrate.index = (int32_t)CANBTR_INDEX_800K; break;
                case 2: case 500:  case 500000:  bitrate.index = (int32_t)CANBTR_INDEX_500K; break;
                case 3: case 250:  case 250000:  bitrate.index = (int32_t)CANBTR_INDEX_250K; break;
                case 4: case 125:  case 125000:  bitrate.index = (int32_t)CANBTR_INDEX_125K; break;
                case 5: case 100:  case 100000:  bitrate.index = (int32_t)CANBTR_INDEX_100K; break;
                case 6: case 50:   case 50000:   bitrate.index = (int32_t)CANBTR_INDEX_50K; break;
                case 7: case 20:   case 20000:   bitrate.index = (int32_t)CANBTR_INDEX_20K; break;
                case 8: case 10:   case 10000:   bitrate.index = (int32_t)CANBTR_INDEX_10K; break;
                default:                         bitrate.index = (int32_t)-baudrate; break;
            }
            if (CCanDevice::MapIndex2Bitrate(bitrate.index, bitrate) != CCanApi::NoError) {
                fprintf(stderr, "%s: illegal argument for option /BAUDRATE\n", basename(argv[0]));
                return 1;
            }
            if (CCanDevice::MapBitrate2Speed(bitrate, speed) != CCanApi::NoError) {
                fprintf(stderr, "%s: illegal argument for option /BAUDRATE\n", basename(argv[0]));
                return 1;
            }
            break;
        /* option `--bitrate=<bit-rate>' as string */
        case BITRATE_STR:
        case BITRATE_CHR:
            if ((bd++)) {
                fprintf(stderr, "%s: duplicated option /BITRATE\n", basename(argv[0]));
                return 1;
            }
            if ((optarg = getOptionParameter()) == NULL) {
                fprintf(stderr, "%s: missing argument for option /BITRATE\n", basename(argv[0]));
                return 1;
            }
            if (CCanDevice::MapString2Bitrate(optarg, bitrate, hasDataPhase, hasNoSamp) != CCanApi::NoError) {
                fprintf(stderr, "%s: illegal argument for option /BITRATE\n", basename(argv[0]));
                return 1;
            }
            if (CCanDevice::MapBitrate2Speed(bitrate, speed) != CCanApi::NoError) {
                fprintf(stderr, "%s: illegal argument for option /BITRATE\n", basename(argv[0]));
                return 1;
            }
            break;
        /* option `--verbose' (-v) */
        case VERBOSE_STR:
        case VERBOSE_CHR:
            if (verbose) {
                fprintf(stderr, "%s: duplicated option /VERBOSE\n", basename(argv[0]));
                return 1;
            }
            if ((optarg = getOptionParameter()) != NULL) {
                fprintf(stderr, "%s: illegal argument for option /VERBOSE\n", basename(argv[0]));
                return 1;
            }
            verbose = 1;
            break;
        /* option `--mode=(2.0|FDF[+BRS])' (-m) */
        case OP_MODE_STR:
        case OP_MODE_CHR:
            if ((op++)) {
                fprintf(stderr, "%s: duplicated option /MODE\n", basename(argv[0]));
                return 1;
            }
            if ((optarg = getOptionParameter()) == NULL) {
                fprintf(stderr, "%s: missing argument for option /MODE\n", basename(argv[0]));
                return 1;
            }
            if (!strcasecmp(optarg, "DEFAULT") || !strcasecmp(optarg, "CLASSIC") || !strcasecmp(optarg, "CLASSICAL") ||
                !strcasecmp(optarg, "CAN20") || !strcasecmp(optarg, "CAN2.0") || !strcasecmp(optarg, "2.0"))
                opMode.byte |= CANMODE_DEFAULT;
#if (CAN_FD_SUPPORTED != 0)
            else if (!strcasecmp(optarg, "CANFD") || !strcasecmp(optarg, "FD") || !strcasecmp(optarg, "FDF"))
                opMode.byte |= CANMODE_FDOE;
            else if (!strcasecmp(optarg, "CANFD+BRS") || !strcasecmp(optarg, "FDF+BRS") || !strcasecmp(optarg, "FD+BRS"))
                opMode.byte |= CANMODE_FDOE | CANMODE_BRSE;
#endif
            else {
                fprintf(stderr, "%s: illegal argument for option /MODE\n", basename(argv[0]));
                return 1;
            }
            break;
        /* option `--shared' */
        case OP_SHARED_STR:
        case OP_SHARED_CHR:
            if ((sh++)) {
                fprintf(stderr, "%s: duplicated option /SHARED\n", basename(argv[0]));
                return 1;
            }
            if ((optarg = getOptionParameter()) != NULL) {
                fprintf(stderr, "%s: illegal argument for option /SHARED\n", basename(argv[0]));
                return 1;
            }
            opMode.byte |= CANMODE_SHRD;
            break;
        /* option `--listen-only' */
        case OP_MON_STR:
        case OP_MONITOR_STR:
            if ((lo++)) {
                fprintf(stderr, "%s: duplicated option /MON\n", basename(argv[0]));
                return 1;
            }
            if ((optarg = getOptionParameter()) == NULL) {
                fprintf(stderr, "%s: missing argument for option /MON\n", basename(argv[0]));
                return 1;
            }
            if (!strcasecmp(optarg, "YES") || !strcasecmp(optarg, "Y") || !strcasecmp(optarg, "ON") || !strcasecmp(optarg, "1"))
                opMode.byte |= CANMODE_MON;
            else if (!strcasecmp(optarg, "NO") || !strcasecmp(optarg, "N") || !strcasecmp(optarg, "OFF") || !strcasecmp(optarg, "0"))
                opMode.byte &= ~CANMODE_MON;
            else {
                fprintf(stderr, "%s: illegal argument for option /MON\n", basename(argv[0]));
                return 1;
            }
            break;
        case OP_LSTNONLY_STR:
            if ((lo++)) {
                fprintf(stderr, "%s: duplicated option /LISTEN-ONLY\n", basename(argv[0]));
                return 1;
            }
            if ((optarg = getOptionParameter()) != NULL) {
                fprintf(stderr, "%s: illegal argument for option /LISTEN-ONLY\n", basename(argv[0]));
                return 1;
            }
            opMode.byte |= CANMODE_MON;
            break;
        /* option `--error-frames' */
        case OP_ERR_STR:
            if ((ef++)) {
                fprintf(stderr, "%s: duplicated option /ERR\n", basename(argv[0]));
                return 1;
            }
            if ((optarg = getOptionParameter()) == NULL) {
                fprintf(stderr, "%s: missing argument for option /ERR\n", basename(argv[0]));
                return 1;
            }
            if (!strcasecmp(optarg, "YES") || !strcasecmp(optarg, "Y") || !strcasecmp(optarg, "ON") || !strcasecmp(optarg, "1"))
                opMode.byte |= CANMODE_ERR;
            else if (!strcasecmp(optarg, "NO") || !strcasecmp(optarg, "N") || !strcasecmp(optarg, "OFF") || !strcasecmp(optarg, "0"))
                opMode.byte &= ~CANMODE_ERR;
            else {
                fprintf(stderr, "%s: illegal argument for option /ERR\n", basename(argv[0]));
                return 1;
            }
            break;
        case OP_ERRFRMS_STR:
            if ((ef++)) {
                fprintf(stderr, "%s: duplicated option /ERROR-FRAMES\n", basename(argv[0]));
                return 1;
            }
            if ((optarg = getOptionParameter()) != NULL) {
                fprintf(stderr, "%s: illegal argument for option /ERROR-FRAMES\n", basename(argv[0]));
                return 1;
            }
            opMode.byte |= CANMODE_ERR;
            break;
        /* option `--no-extended-frames' */
        case OP_XTD_STR:
            if ((xf++)) {
                fprintf(stderr, "%s: duplicated option /XTD\n", basename(argv[0]));
                return 1;
            }
            if ((optarg = getOptionParameter()) == NULL) {
                fprintf(stderr, "%s: missing argument for option /XTD\n", basename(argv[0]));
                return 1;
            }
            if (!strcasecmp(optarg, "NO") || !strcasecmp(optarg, "N") || !strcasecmp(optarg, "OFF") || !strcasecmp(optarg, "0"))
                opMode.byte |= CANMODE_NXTD;
            else if (!strcasecmp(optarg, "YES") || !strcasecmp(optarg, "Y") || !strcasecmp(optarg, "ON") || !strcasecmp(optarg, "1"))
                opMode.byte &= ~CANMODE_NXTD;
            else {
                fprintf(stderr, "%s: illegal argument for option /XTD\n", basename(argv[0]));
                return 1;
            }
            break;
        /* option `--no-remote-frames' */
        case OP_RTR_STR:
            if ((rf++)) {
                fprintf(stderr, "%s: duplicated option /RTR\n", basename(argv[0]));
                return 1;
            }
            if ((optarg = getOptionParameter()) == NULL) {
                fprintf(stderr, "%s: missing argument for option /RTR\n", basename(argv[0]));
                return 1;
            }
            if (!strcasecmp(optarg, "NO") || !strcasecmp(optarg, "N") || !strcasecmp(optarg, "OFF") || !strcasecmp(optarg, "0"))
                opMode.byte |= CANMODE_NRTR;
            else if (!strcasecmp(optarg, "YES") || !strcasecmp(optarg, "Y") || !strcasecmp(optarg, "ON") || !strcasecmp(optarg, "1"))
                opMode.byte &= ~CANMODE_NRTR;
            else {
                fprintf(stderr, "%s: illegal argument for option /RTR\n", basename(argv[0]));
                return 1;
            }
            break;
        /* option `--time=(ABS|REL|ZERO)' (-t) */
        case MODE_TIME_STR:
        case MODE_TIME_CHR:
            if ((mt++)) {
                fprintf(stderr, "%s: conflict for option /TIME\n", basename(argv[0]));
                return 1;
            }
            if ((optarg = getOptionParameter()) == NULL) {
                fprintf(stderr, "%s: missing argument for option /TIME\n", basename(argv[0]));
                return 1;
            }
            if (!strcasecmp(optarg, "ABSOLUTE") || !strcasecmp(optarg, "ABS") || !strcasecmp(optarg, "a"))
                modeTime = CCanMessage::OptionAbsolute;
            else if (!strcasecmp(optarg, "RELATIVE") || !strcasecmp(optarg, "REL") || !strcasecmp(optarg, "r"))
                modeTime = CCanMessage::OptionRelative;
            else if (!strcasecmp(optarg, "ZERO") || !strcasecmp(optarg, "0") || !strcasecmp(optarg, "z"))
                modeTime = CCanMessage::OptionZero;
            else {
                fprintf(stderr, "%s: illegal argument for option /TIME\n", basename(argv[0]));
                return 1;
            }
            if (!CCanMessage::SetTimestampFormat(modeTime)) {
                fprintf(stderr, "%s: illegal argument for option /TIME\n", basename(argv[0]));
                return 1;
            }
            break;
        /* option `--id=(HEX|DEC|OCT)' (-i) */
        case MODE_ID_STR:
        case MODE_ID_CHR:
            if ((mi++)) {
                fprintf(stderr, "%s: conflict for option /ID\n", basename(argv[0]));
                return 1;
            }
            if ((optarg = getOptionParameter()) == NULL) {
                fprintf(stderr, "%s: missing argument for option /ID\n", basename(argv[0]));
                return 1;
            }
            if (!strcasecmp(optarg, "HEXADECIMAL") || !strcasecmp(optarg, "HEX") || !strcasecmp(optarg, "h") || !strcasecmp(optarg, "16"))
                modeId = CCanMessage::OptionHex;
            else if (!strcasecmp(optarg, "DECIMAL") || !strcasecmp(optarg, "DEC") || !strcasecmp(optarg, "d") || !strcasecmp(optarg, "10"))
                modeId = CCanMessage::OptionDec;
            else if (!strcasecmp(optarg, "OCTAL") || !strcasecmp(optarg, "OCT") || !strcasecmp(optarg, "o") || !strcasecmp(optarg, "8"))
                modeId = CCanMessage::OptionOct;
            else {
                fprintf(stderr, "%s: illegal argument for option /ID\n", basename(argv[0]));
                return 1;
            }
            if (!CCanMessage::SetIdentifierFormat(modeId)) {
                fprintf(stderr, "%s: illegal argument for option /ID\n", basename(argv[0]));
                return 1;
            }
            break;
        /* option `--data=(HEX|DEC|OCT)' (-d) */
        case MODE_DATA_STR:
        case MODE_DATA_CHR:
            if ((md++)) {
                fprintf(stderr, "%s: conflict for option /DATA\n", basename(argv[0]));
                return 1;
            }
            if ((optarg = getOptionParameter()) == NULL) {
                fprintf(stderr, "%s: missing argument for option /DATA\n", basename(argv[0]));
                return 1;
            }
            if (!strcasecmp(optarg, "HEXADECIMAL") || !strcasecmp(optarg, "HEX") || !strcasecmp(optarg, "h") || !strcasecmp(optarg, "16"))
                modeData = CCanMessage::OptionHex;
            else if (!strcasecmp(optarg, "DECIMAL") || !strcasecmp(optarg, "DEC") || !strcasecmp(optarg, "d") || !strcasecmp(optarg, "10"))
                modeData = CCanMessage::OptionDec;
            else if (!strcasecmp(optarg, "OCTAL") || !strcasecmp(optarg, "OCT") || !strcasecmp(optarg, "o") || !strcasecmp(optarg, "8"))
                modeData = CCanMessage::OptionOct;
            else {
                fprintf(stderr, "%s: illegal argument for option /DATA\n", basename(argv[0]));
                return 1;
            }
            if (!CCanMessage::SetDataFormat(modeData)) {
                fprintf(stderr, "%s: illegal argument for option /DATA\n", basename(argv[0]));
                return 1;
            }
            break;
        /* option `--ascii=(ON|OFF)' (-a) */
        case MODE_ASCII_STR:
        case MODE_ASCII_CHR:
            if ((ma++)) {
                fprintf(stderr, "%s: conflict for option /ASCII\n", basename(argv[0]));
                return 1;
            }
            if ((optarg = getOptionParameter()) == NULL) {
                fprintf(stderr, "%s: missing argument for option /ASCII\n", basename(argv[0]));
                return 1;
            }
            if (!strcasecmp(optarg, "OFF") || !strcasecmp(optarg, "NO") || !strcasecmp(optarg, "n") || !strcasecmp(optarg, "0"))
                modeAscii = CCanMessage::OptionOff;
            else if (!strcasecmp(optarg, "ON") || !strcasecmp(optarg, "YES") || !strcasecmp(optarg, "y") || !strcasecmp(optarg, "1"))
                modeAscii = CCanMessage::OptionOn;
            else {
                fprintf(stderr, "%s: illegal argument for option /ASCII\n", basename(argv[0]));
                return 1;
            }
            if (!CCanMessage::SetAsciiFormat(modeAscii)) {
                fprintf(stderr, "%s: illegal argument for option /ASCII\n", basename(argv[0]));
                return 1;
            }
            break;
#if (CAN_FD_SUPPORTED != 0)
        /* option `--wrap=....' (-w) */
        case WRAPAROUND_STR:
        case WRAPAROUND_CHR:
            if ((mw++)) {
                fprintf(stderr, "%s: conflict for option /WRAPAROUND\n", basename(argv[0]));
                return 1;
            }
            if ((optarg = getOptionParameter()) == NULL) {
                fprintf(stderr, "%s: missing argument for option /WRAPAROUND\n", basename(argv[0]));
                return 1;
            }
            if (!strcasecmp(optarg, "NO") || !strcasecmp(optarg, "n") || !strcasecmp(optarg, "0"))
                wraparound = CCanMessage::OptionWraparoundNo;
            else if (!strcasecmp(optarg, "8"))
                wraparound = CCanMessage::OptionWraparound8;
            else if (!strcasecmp(optarg, "10"))
                wraparound = CCanMessage::OptionWraparound10;
            else if (!strcasecmp(optarg, "16"))
                wraparound = CCanMessage::OptionWraparound16;
            else if (!strcasecmp(optarg, "32"))
                wraparound = CCanMessage::OptionWraparound32;
            else if (!strcasecmp(optarg, "64"))
                wraparound = CCanMessage::OptionWraparound64;
            else {
                fprintf(stderr, "%s: illegal argument for option /WRAPAROUND\n", basename(argv[0]));
                return 1;
            }
            if (!CCanMessage::SetWraparound(wraparound)) {
                fprintf(stderr, "%s: illegal argument for option /WRAPAROUND\n", basename(argv[0]));
                return 1;
            }
            break;
#endif
        /* option `--exclude=[~]<id-list>' (-x) */
        case EXCLUDE_STR:
        case EXCLUDE_CHR:
            if ((exclude++)) {
                fprintf(stderr, "%s: conflict for option /EXCLUDE\n", basename(argv[0]));
                return 1;
            }
            if ((optarg = getOptionParameter()) == NULL) {
                fprintf(stderr, "%s: missing argument for option /EXCLUDE\n", basename(argv[0]));
                return 1;
            }
            if (!get_exclusion(optarg)) {
                fprintf(stderr, "%s: illegal argument for option /EXCLUDE\n", basename(argv[0]));
                return 1;
            }
            break;
#ifdef ACCEPTANCE_FILTERING
        /* option `--code=<11-bit-code>' */
        /* option `--mask=<11-bit-mask>' */
        /* option `--xtd-code=<29-bit-code>' */
        /* option `--xtd-mask=<29-bit-mask>' */
#endif
        /* option `--list-bitrates[=(2.0|FDF[+BRS])]' */
        case LISTBITRATES_STR:
            if ((optarg = getOptionParameter()) != NULL) {
                if (!strcasecmp(optarg, "DEFAULT") || !strcasecmp(optarg, "CLASSIC") || !strcasecmp(optarg, "CLASSICAL") ||
                    !strcasecmp(optarg, "CAN20") || !strcasecmp(optarg, "CAN2.0") || !strcasecmp(optarg, "2.0"))
                    brMode.byte |= CANMODE_DEFAULT;
#if (CAN_FD_SUPPORTED != 0)
                else if (!strcasecmp(optarg, "CANFD") || !strcasecmp(optarg, "FD") || !strcasecmp(optarg, "FDF"))
                    brMode.byte |= CANMODE_FDOE;
                else if (!strcasecmp(optarg, "CANFD+BRS") || !strcasecmp(optarg, "FDF+BRS") || !strcasecmp(optarg, "FD+BRS"))
                    brMode.byte |= CANMODE_FDOE | CANMODE_BRSE;
#endif
                else {
                    fprintf(stderr, "%s: illegal argument for option /LIST-BITRATES\n", basename(argv[0]));
                    return 1;
                }
            } else {
                brMode.byte = CANMODE_DEFAULT;
            }
            fprintf(stdout, "%s\n%s\n\n%s\n\n", APPLICATION, COPYRIGHT, WARRANTY);
            /* list standard bit-rates */
            (void)CCanDevice::ListCanBitrates(brMode);
            return 0;
        /* option `--list-boards[=<vendor>]' (-L) */
        case LISTBOARDS_STR:
        case LISTBOARDS_CHR:
            fprintf(stdout, "%s\n%s\n\n%s\n\n", APPLICATION, COPYRIGHT, WARRANTY);
            /* list all supported interfaces */
            num_boards = CCanDevice::ListCanDevices(/*getOptionParameter()*/);
            fprintf(stdout, "Number of supported CAN interfaces: %i\n", num_boards);
            return (num_boards >= 0) ? 0 : 1;
        /* option `--test-boards[=<vendor>]' (-T) */
        case TESTBOARDS_STR:
        case TESTBOARDS_CHR:
            fprintf(stdout, "%s\n%s\n\n%s\n\n", APPLICATION, COPYRIGHT, WARRANTY);
            /* list all available interfaces */
            num_boards = CCanDevice::TestCanDevices(opMode/*, getOptionParameter()*/);
            fprintf(stdout, "Number of present CAN interfaces: %i\n", num_boards);
            return (num_boards >= 0) ? 0 : 1;
        /* option `--help' (-h) */
        case HELP:
        case QUESTION_MARK:
            usage(stdout, basename(argv[0]));
            return 0;
        case ABOUT:
        case VERSION:
        case CHARACTER_MJU:
            version(stdout, basename(argv[0]));
            return 0;
        default:
            usage(stderr, basename(argv[0]));
            return 1;
        }
    }
    /* - check if one and only one <interface> is given */
    for (i = 1; i < argc; i++) {
        if (!isOption(argc, (char**)argv, MAX_OPTIONS, option, i)) {
            if ((hw++)) {
                fprintf(stderr, "%s: too many arguments\n", basename(argv[0]));
                return 1;
            }
#if (SERIAL_CAN_SUPPORTED == 0)
            /* - search the <interface> by its name in the device list */
            bool result = CCanDevice::GetFirstChannel(channel);
            while (result) {
                if (strcasecmp(argv[i], channel.m_szDeviceName) == 0) {
                    break;
                }
                result = CCanDevice::GetNextChannel(channel);
            }
            if (!result) {
                fprintf(stderr, "%s: illegal argument\n", basename(argv[0]));
                return 1;
            }
#else
            /* - take serial device name from command line */
            port = (char*)argv[i];
#endif
        }
    }
    if (!hw) {
        fprintf(stderr, "%s: not enough arguments\n", basename(argv[0]));
        return 1;
    }
#if (CAN_FD_SUPPORTED != 0)
    /* - check bit-timing index (n/a for CAN FD) */
    if (opMode.fdoe && (bitrate.btr.frequency <= 0)) {
        fprintf(stderr, "%s: illegal combination of options /MODE and /BAUDRATE\n", basename(argv[0]));
        return 1;
    }
#endif
    /* CAN Monitor for generic CAN interfaces */
    fprintf(stdout, "%s\n%s\n\n%s\n\n", APPLICATION, COPYRIGHT, WARRANTY);
    /* - show operation mode, bit-rate settings and acceptance filter (if set) */
    if (verbose) {
        fprintf(stdout, "Op.-mode=%s", (opMode.byte & CANMODE_FDOE) ? "CANFD" : "CAN2.0");
        if ((opMode.byte & CANMODE_BRSE)) fprintf(stdout, "+BRS");
        if ((opMode.byte & CANMODE_NISO)) fprintf(stdout, "+NISO");
        if ((opMode.byte & CANMODE_SHRD)) fprintf(stdout, "+SHRD");
        if ((opMode.byte & CANMODE_NXTD)) fprintf(stdout, "+NXTD");
        if ((opMode.byte & CANMODE_NRTR)) fprintf(stdout, "+NRTR");
        if ((opMode.byte & CANMODE_ERR)) fprintf(stdout, "+ERR");
        if ((opMode.byte & CANMODE_MON)) fprintf(stdout, "+MON");
        fprintf(stdout, " (op_mode=%02Xh)\n", opMode.byte);
        if (bitrate.btr.frequency > 0) {
            fprintf(stdout, "Bit-rate=%.0fkbps@%.1f%%", speed.nominal.speed / 1000., speed.nominal.samplepoint * 100.);
#if (CAN_FD_SUPPORTED != 0)
            if (opMode.byte & CANMODE_BRSE)
                fprintf(stdout, ":%.0fkbps@%.1f%%", speed.data.speed / 1000., speed.data.samplepoint * 100.);
#endif
            (void)CCanDevice::MapBitrate2String(bitrate, property, CANPROP_MAX_BUFFER_SIZE,
                                                (opMode.byte & CANMODE_BRSE), hasNoSamp);
            fprintf(stdout, " (%s)\n\n", property);
        }
        else {
            fprintf(stdout, "Baudrate=%.0fkbps@%.1f%% (index %i)\n\n",
                             speed.nominal.speed / 1000.,
                             speed.nominal.samplepoint * 100., -bitrate.index);
        }
#ifdef ACCEPTANCE_FILTERING
        if ((code11 != CANACC_CODE_11BIT) || (mask11 != CANACC_MASK_11BIT))
            fprintf(stdout, "Acc.-Filter 11-bit=set (code=%03lXh, mask=%03lXh)\n", code11, mask11);
        if (((code29 != CANACC_CODE_29BIT) || (mask29 != CANACC_MASK_29BIT)) && !opMode.nxtd)
            fprintf(stdout, "Acc.-Filter 29-bit=set (code=%08lXh, mask=%08lXh)\n", code29, mask29);
#endif
        fputc('\n', stdout);
    }
    /* - initialize interface */
#if (SERIAL_CAN_SUPPORTED == 0)
    fprintf(stdout, "Hardware=%s...", channel.m_szDeviceName);
    fflush (stdout);
    retVal = canDevice.InitializeChannel(channel.m_nChannelNo, opMode);
#else
    fprintf(stdout, "Hardware=%s...", port);
    fflush (stdout);
    retVal = canDevice.InitializeChannel(port, opMode);
#endif
    if (retVal != CCanApi::NoError) {
        fprintf(stdout, "FAILED!\n");
        fprintf(stderr, "+++ error: CAN Controller could not be initialized (%i)", retVal);
        if (retVal == CCanApi::NotSupported)
            fprintf(stderr, "\n           - possibly CAN operating mode %02Xh not supported", opMode.byte);
        fputc('\n', stderr);
        goto finalize;
    }
#ifdef ACCEPTANCE_FILTERING
    /* -- set acceptance filter for 11-bit IDs */
    if ((code11 != CANACC_CODE_11BIT) || (mask11 != CANACC_MASK_11BIT)) {
        retVal = canDevice.SetFilter11Bit(code11, mask11);
        if (retVal != CCanApi::NoError) {
            fprintf(stdout, "FAILED!\n");
            fprintf(stderr, "+++ error: CAN acceptance filter could not be set (%i)\n", retVal);
            goto teardown;
        }
    }
    /* -- set acceptance filter for 29-bit IDs */
    if (((code29 != CANACC_CODE_29BIT) || (mask29 != CANACC_MASK_29BIT)) && !opMode.nxtd) {
        retVal = canDevice.SetFilter29Bit(code29, mask29);
        if (retVal != CCanApi::NoError) {
            fprintf(stdout, "FAILED!\n");
            fprintf(stderr, "+++ error: CAN acceptance filter could not be set (%i)\n", retVal);
            goto teardown;
        }
    }
#endif
    fprintf(stdout, "OK!\n");
    /* - start communication */
    if (bitrate.btr.frequency > 0) {
        fprintf(stdout, "Bit-rate=%.0fkbps", speed.nominal.speed / 1000.);
#if (CAN_FD_SUPPORTED != 0)
        if (opMode.byte & CANMODE_BRSE)
            fprintf(stdout, ":%.0fkbps", speed.data.speed / 1000.);
        else if (opMode.byte & CANMODE_FDOE)
            fprintf(stdout, ":%.0fkbps", speed.nominal.speed / 1000.);
#endif
        fprintf(stdout, "...");
    }
    else {
        fprintf(stdout, "Baudrate=%skbps...",
            bitrate.index == CANBTR_INDEX_1M   ? "1000" :
            bitrate.index == CANBTR_INDEX_800K ? "800" :
            bitrate.index == CANBTR_INDEX_500K ? "500" :
            bitrate.index == CANBTR_INDEX_250K ? "250" :
            bitrate.index == CANBTR_INDEX_125K ? "125" :
            bitrate.index == CANBTR_INDEX_100K ? "100" :
            bitrate.index == CANBTR_INDEX_50K  ? "50" :
            bitrate.index == CANBTR_INDEX_20K  ? "20" :
            bitrate.index == CANBTR_INDEX_10K  ? "10" : "?");
    }
    fflush(stdout);
    retVal = canDevice.StartController(bitrate);
    if (retVal != CCanApi::NoError) {
        fprintf(stdout, "FAILED!\n");
        fprintf(stderr, "+++ error: CAN Controller could not be started (%i)\n", retVal);
        goto teardown;
    }
    fprintf(stdout, "OK!\n");
    /* - reception loop */
    canDevice.ReceptionLoop();
    /* - show interface information */
    if ((device = canDevice.GetHardwareVersion()) != NULL)
        fprintf(stdout, "Hardware: %s\n", device);
    if ((firmware = canDevice.GetFirmwareVersion()) != NULL)
        fprintf(stdout, "Firmware: %s\n", firmware);
    if ((software = CCanDevice::GetVersion()) != NULL)
        fprintf(stdout, "Software: %s\n", software);
teardown:
    /* - teardown the interface*/
    retVal = canDevice.TeardownChannel();
    if (retVal != CCanApi::NoError) {
        fprintf(stderr, "+++ error: CAN Controller could not be reset (%i)\n", retVal);
        goto finalize;
    }
finalize:
    /* So long and farewell! */
    fprintf(stdout, "%s\n", COPYRIGHT);
    return retVal;
}

int CCanDevice::ListCanDevices(void) {
    CCanDevice::SChannelInfo info;
    int n = 0;

    fprintf(stdout, "Suppored hardware:\n");
    bool result = CCanDevice::GetFirstChannel(info);
    while (result) {
        fprintf(stdout, "\"%s\" (VendorName=\"%s\", LibraryId=%" PRIi32 ", ChannelNo=%" PRIi32 ")\n",
                         info.m_szDeviceName, info.m_szVendorName, info.m_nLibraryId, info.m_nChannelNo);
        n++;
        result = CCanDevice::GetNextChannel(info);
    }
#if (SERIAL_CAN_SUPPORTED != 0)
    if (n == 0) {
        fprintf(stdout, "Check the Device Manager for compatible serial communication devices!\n");
    }
#endif
    return n;
}

int CCanDevice::TestCanDevices(CANAPI_OpMode_t opMode) {
    CCanDevice::SChannelInfo info;
    int n = 0;

    bool result = CCanDevice::GetFirstChannel(info);
    while (result) {
        fprintf(stdout, "Hardware=%s...", info.m_szDeviceName);
        fflush(stdout);
        EChannelState state;
        CANAPI_Return_t retVal = CCanDevice::ProbeChannel(info.m_nChannelNo, opMode, state);
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
        result = CCanDevice::GetNextChannel(info);
    }
#if (SERIAL_CAN_SUPPORTED != 0)
    if (n == 0) {
        fprintf(stdout, "Check the Device Manager for compatible serial communication devices!\n");
    }
#endif
    return n;
}

int CCanDevice::ListCanBitrates(CANAPI_OpMode_t opMode) {
    CANAPI_Bitrate_t bitrate[9];
    CANAPI_BusSpeed_t speed;
    CANAPI_Return_t retVal;

    char string[CANPROP_MAX_BUFFER_SIZE] = "";
    bool hasDataPhase = false;
    bool hasNoSamp = true;
    int i, n = 0;

#if (CAN_FD_SUPPORTED != 0)
    if (opMode.fdoe) {
        if (opMode.brse) {
            fprintf(stdout, "CAN FD with Bit-rate Switching (BRS):\n");
            BITRATE_FD_1M8M(bitrate[n]); n += 1;
            BITRATE_FD_500K4M(bitrate[n]); n += 1;
            BITRATE_FD_250K2M(bitrate[n]); n += 1;
            BITRATE_FD_125K1M(bitrate[n]); n += 1;
            hasDataPhase = true;
            hasNoSamp = false;
        }
        else {
            fprintf(stdout, "CAN FD without Bit-rate Switching (BRS):\n");
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
        fprintf(stdout, "Classical CAN:\n");
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
        if ((retVal = CCanDevice::MapBitrate2Speed(bitrate[i], speed)) == CCanApi::NoError) {
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

uint64_t CCanDevice::ReceptionLoop() {
    CANAPI_Message_t message;
    CANAPI_Return_t retVal;
    uint64_t frames = 0U;

    char string[CANPROP_MAX_STRING_LENGTH+1];
    memset(string, 0, CANPROP_MAX_STRING_LENGTH+1);

    fprintf(stderr, "\nPress ^C to abort.\n\n");
    while(running) {
        if ((retVal = ReadMessage(message)) == CCanApi::NoError) {
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
static int get_exclusion(const char *arg)
{
    char *val, *end;
    int i, inv = 0;
    long id, last = -1;

    if (!arg)
        return 0;

    val = (char *)arg;
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
            can_id[i] = !can_id[i];
    }
    can_id_xtd = !inv;
    return 1;
}

/** @brief       signal handler to catch Ctrl+C.
 *
 *  @param[in]   signo - signal number (SIGINT, SIGHUP, SIGTERM)
 */
static void sigterm(int signo)
{
    //fprintf(stderr, "%s: got signal %d\n", __FILE__, signo);
    (void)canDevice.SignalChannel();
    running = 0;
    (void)signo;
}

/** @brief       shows a help screen with all command-line options.
 *
 *  @param[in]   stream  - output stream (e.g. stdout)
 *  @param[in]   program - base name of the program
 */
static void usage(FILE *stream, const char *program)
{
    fprintf(stream, "Usage: %s <interface> [<option>...]\n", program);
    fprintf(stream, "Options:\n");
    fprintf(stream, "  /Time:(ZERO|ABS|REL)                absolute or relative time (default=0)\n");
    fprintf(stream, "  /Id:(HEX|DEC|OCT)                   display mode of CAN-IDs (default=HEX)\n");
    fprintf(stream, "  /Data:(HEX|DEC|OCT)                 display mode of data bytes (default=HEX)\n");
    fprintf(stream, "  /Ascii:(ON|OFF)                     display data bytes in ASCII (default=ON)\n");
#if (CAN_FD_SUPPORTED != 0)
    fprintf(stream, "  /Wraparound:(No|8|10|16|32|64)      wraparound after n data bytes (default=NO)\n");
#endif
    fprintf(stream, "  /eXclude:[~]<id-list>               exclude CAN-IDs: <id-list> = <id>[-<id>]{,<id>[-<id>]}\n");
#ifdef ACCEPTANCE_FILTERING
    fprintf(stream, "  /CODE:<id>                          acceptance code for 11-bit IDs (default=0x%03lx)\n", CANACC_CODE_11BIT);
    fprintf(stream, "  /MASK:<id>                          acceptance mask for 11-bit IDs (default=0x%03lx)\n", CANACC_MASK_11BIT);
    fprintf(stream, "  /XTD-CODE:<id>                      acceptance code for 29-bit IDs (default=0x%08lx)\n", CANACC_CODE_29BIT);
    fprintf(stream, "  /XTD-MASK:<id>                      acceptance mask for 29-bit IDs (default=0x%08lx)\n", CANACC_MASK_29BIT);
#endif
//    fprintf(stream, "  /Script:<filename>                  execute a script file\n"); // TODO: script engine
#if (CAN_FD_SUPPORTED != 0)
    fprintf(stream, "  /Mode:(2.0|FDf[+BRS])               CAN operation mode: CAN 2.0 or CAN FD mode\n");
#else
    fprintf(stream, "  /Mode:2.0                           CAN operation mode: CAN 2.0\n");
#endif
    fprintf(stream, "  /SHARED                             shared CAN controller access (if supported)\n");
    fprintf(stream, "  /MONitor:(No|Yes) | /LISTEN-ONLY    monitor mode (listen-only, transmitter is off)\n");
    fprintf(stream, "  /ERR:(No|Yes) | /ERROR-FRAMES       allow reception of error frames\n");
    fprintf(stream, "  /RTR:(Yes|No)                       allow remote frames (RTR frames)\n");
    fprintf(stream, "  /XTD:(Yes|No)                       allow extended frames (29-bit identifier)\n");
    fprintf(stream, "  /BauDrate:<baudrate>                CAN bit-timing in kbps (default=250), or\n");
    fprintf(stream, "  /BitRate:<bitrate>                  CAN bit-rate settings (as a string)\n");
    fprintf(stream, "  /Verbose                            show detailed bit-rate settings\n");
#if (CAN_FD_SUPPORTED != 0)
    fprintf(stream, "  /LIST-BITRATES[:(2.0|FDf[+BRS])]    list standard bit-rate settings\n");
#else
    fprintf(stream, "  /LIST-BITRATES[:2.0]                list standard bit-rate settings\n");
#endif
#if (OPTION_CANAPI_LIBRARY != 0)
    fprintf(stream, "  /LIST[-BOARDS][:<vendor>]           list all supported CAN interfaces and exit\n");
    fprintf(stream, "  /TEST[-BOARDS][:<vendor>]           list all available CAN interfaces and exit\n");
#elif (SERIAL_CAN_SUPPORTED == 0)
    fprintf(stream, "  /LIST-BOARDS | /LIST                list all supported CAN interfaces and exit\n");
    fprintf(stream, "  /TEST-BOARDS | /TEST                list all available CAN interfaces and exit\n");
#endif
    fprintf(stream, "  /HELP | /?                          display this help screen and exit\n");
    fprintf(stream, "  /VERSION                            show version information and exit\n");
#if (0)
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
    fprintf(stream, "  <bitrate>      comma-separated <key>=<value>-list:\n");
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
#endif
    fprintf(stream, "Hazard note:\n");
    fprintf(stream, "  If you connect your CAN device to a real CAN network when using this program,\n");
    fprintf(stream, "  you might damage your application.\n");
}

/** @brief       shows version information of the program.
 *
 *  @param[in]   stream  - output stream (e.g. stdout)
 *  @param[in]   program - base name of the program
 */
static void version(FILE *stream, const char *program)
{
    fprintf(stdout, "%s\n%s\n\n%s\n\n", APPLICATION, COPYRIGHT, LICENSE);
    (void)program;
    fprintf(stream, "Written by Uwe Vogt, UV Software, Berlin <https://www.uv-software.com/>\n");
}
