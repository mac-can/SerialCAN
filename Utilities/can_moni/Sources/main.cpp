//  SPDX-License-Identifier: GPL-3.0-or-later
//
//  CAN Monitor for CAN-over-Serial-Line Interfaces
//
//  Copyright (C) 2007,2016-2020  Uwe Vogt, UV Software, Berlin (info@uv-software.com)
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
//  along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
#include "SerialCAN.h"
#include "Timer.h"
#include "Message.h"

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>
#include <getopt.h>
#include <signal.h>
#include <errno.h>
#include <time.h>

#include <inttypes.h>

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
static const char APPLICATION[] = "CAN Monitor for CAN-over-Serial-Line Interfaces, Version " VERSION_STRING " _DEBUG";
#else
static const char APPLICATION[] = "CAN Monitor for CAN-over-Serial-Line Interfaces, Version " VERSION_STRING;
#endif
static const char COPYRIGHT[]   = "Copyright (c) 2007,2016-2022 by Uwe Vogt, UV Software, Berlin";
static const char WARRANTY[]    = "This program comes with ABSOLUTELY NO WARRANTY!\n\n" \
                                  "This is free software, and you are welcome to redistribute it\n" \
                                  "under certain conditions; type `--version' for details.";
static const char LICENSE[]     = "This program is free software: you can redistribute it and/or modify\n" \
                                  "it under the terms of the GNU General Public License as published by\n" \
                                  "the Free Software Foundation, either version 3 of the License, or\n" \
                                  "(at your option) any later version.\n\n" \
                                  "This program is distributed in the hope that it will be useful,\n" \
                                  "but WITHOUT ANY WARRANTY; without even the implied warranty of\n" \
                                  "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n" \
                                  "GNU General Public License for more details.\n\n" \
                                  "You should have received a copy of the GNU General Public License\n" \
                                  "along with this program.  If not, see <http://www.gnu.org/licenses/>.";
#define basename(x)  "can_moni" // FIXME: Where is my `basename' function?

#define MAX_ID      (CAN_MAX_STD_ID + 1)

static int get_exclusion(const char *arg);  // TODO: make it a member function

class CCanDriver : public CSerialCAN {
public:
    uint64_t ReceptionLoop();
public:
    static int ListCanDevices(const char *vendor = NULL);
    static int TestCanDevices(CANAPI_OpMode_t opMode, const char *vendor = NULL);
    // list of CAN interface vendors
    static const struct TCanVendor {
        int32_t id;
        char *name;
    } m_CanVendors[];
    // list of CAN interface devices
    static const struct TCanDevice {
        int32_t library;
        int32_t adapter;
        char *name;
    } m_CanDevices[];
};
const CCanDriver::TCanVendor CCanDriver::m_CanVendors[] = {
    {SERIALCAN_LIBRARY_ID, (char *)"Serial Device" },
    {EOF, NULL}
};
const CCanDriver::TCanDevice CCanDriver::m_CanDevices[] = {
    {EOF, EOF, NULL}
};

static void sigterm(int signo);
static void usage(FILE *stream, const char *program);
static void version(FILE *stream, const char *program);

static int can_id[MAX_ID];
static int can_id_xtd = 1;
static volatile int running = 1;

static CCanDriver canDriver = CCanDriver();

// TODO: this code could be made more C++ alike
int main(int argc, const char * argv[]) {
    int opt;
    char *port = NULL;
    int op = 0, rf = 0, xf = 0, ef = 0, lo = 0, sh = 0;
    long baudrate = CANBDR_250; int bd = 0;
    CCanMessage::EFormatTimestamp modeTime = CCanMessage::OptionZero; int mt = 0;
    CCanMessage::EFormatNumber modeId = CCanMessage::OptionHex; int mi = 0;
    CCanMessage::EFormatNumber modeData = CCanMessage::OptionHex; int md = 0;
    CCanMessage::EFormatOption modeAscii = CCanMessage::OptionOn; int ma = 0;
    CCanMessage::EFormatWraparound wraparound = CCanMessage::OptionWraparoundNo; int mw = 0;
    int exclude = 0;
//    char *script_file = NULL;
    int verbose = 0;
    int show_version = 0;
    char *device, *firmware, *software;
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
        {"time", required_argument, 0, 't'},
        {"id", required_argument, 0, 'i'},
        {"data", required_argument, 0, 'd'},
        {"ascii", required_argument, 0, 'a'},
        {"wrap", required_argument, 0, 'w'},
        {"wraparound", required_argument, 0, 'w'},
        {"exclude", required_argument, 0, 'x'},
        {"script", required_argument, 0, 's'},
        {"help", no_argument, 0, 'h'},
        {"version", no_argument, &show_version, 1},
        {0, 0, 0, 0}
    };
    CANAPI_Bitrate_t bitrate = {};
    CANAPI_BusSpeed_t speed = {};
    CANAPI_OpMode_t opMode = {};
    CANAPI_Return_t retVal = 0;

    /* default mode and bit-timing */
    opMode.byte = CANMODE_DEFAULT;
    bitrate.index = CANBTR_INDEX_250K;
    (void)CCanDriver::MapIndex2Bitrate(bitrate.index, bitrate);
    (void)CCanDriver::MapBitrate2Speed(bitrate, speed);
    (void)op;

    /* default format options */
    (void)CCanMessage::SetTimestampFormat(modeTime);
    (void)CCanMessage::SetIdentifierFormat(modeId);
    (void)CCanMessage::SetDataFormat(modeData);
    (void)CCanMessage::SetAsciiFormat(modeAscii);
    (void)CCanMessage::SetWraparound(wraparound);

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
    while ((opt = getopt_long(argc, (char * const *)argv, "b:vm:t:i:d:a:w:x:s:h", long_options, NULL)) != -1) {
        switch (opt) {
        case 'b':  /* option `--baudrate=<baudrate>' (-b) */
            if (bd++) {
                fprintf(stderr, "%s: duplicated option `--baudrate' (%c)\n", basename(argv[0]), opt);
                return 1;
            }
            if (optarg && optarg[0] == '-') {
                fprintf(stderr, "%s: missing argument for option `--baudrate' (%c)\n", basename(argv[0]), opt);
                return 1;
            }
            if (sscanf(optarg, "%li", &baudrate) != 1) {
                fprintf(stderr, "%s: illegal argument for option `--baudrate' (%c)\n", basename(argv[0]), opt);
                return 1;
            }
            switch (baudrate) {
                case 1000: case 1000000: bitrate.index = (int32_t)CANBTR_INDEX_1M; break;
                case 800:  case 800000:  bitrate.index = (int32_t)CANBTR_INDEX_800K; break;
                case 500:  case 500000:  bitrate.index = (int32_t)CANBTR_INDEX_500K; break;
                case 250:  case 250000:  bitrate.index = (int32_t)CANBTR_INDEX_250K; break;
                case 125:  case 125000:  bitrate.index = (int32_t)CANBTR_INDEX_125K; break;
                case 100:  case 100000:  bitrate.index = (int32_t)CANBTR_INDEX_100K; break;
                case 50:   case 50000:   bitrate.index = (int32_t)CANBTR_INDEX_50K; break;
                case 20:   case 20000:   bitrate.index = (int32_t)CANBTR_INDEX_20K; break;
                case 10:   case 10000:   bitrate.index = (int32_t)CANBTR_INDEX_10K; break;
                default:                 bitrate.index = (int32_t)-baudrate; break;
            }
            if (CCanDriver::MapIndex2Bitrate(bitrate.index, bitrate) != CCanApi::NoError) {
                fprintf(stderr, "%s: illegal argument for option `--baudrate' (%c)\n", basename(argv[0]), opt);
                return 1;
            }
            if (CCanDriver::MapBitrate2Speed(bitrate, speed) != CCanApi::NoError) {
                fprintf(stderr, "%s: illegal argument for option `--baudrate' (%c)\n", basename(argv[0]), opt);
                return 1;
            }
            break;
        case 'B':
            if (bd++) {
                fprintf(stderr, "%s: duplicated option `--bitrate'\n", basename(argv[0]));
                return 1;
            }
            if (optarg && optarg[0] == '-') {
                fprintf(stderr, "%s: missing argument for option `--bitrate' (%c)\n", basename(argv[0]), opt);
                return 1;
            }
            if (CCanDriver::MapString2Bitrate(optarg, bitrate) != CCanApi::NoError) {
                fprintf(stderr, "%s: illegal argument for option `--bitrate'\n", basename(argv[0]));
                return 1;
            }
            if (CCanDriver::MapBitrate2Speed(bitrate, speed) != CCanApi::NoError) {
                fprintf(stderr, "%s: illegal argument for option `--bitrate'\n", basename(argv[0]));
                return 1;
            }
            break;
        case 'v':  /* option `--verbose' (-v) */
            if (verbose) {
                fprintf(stderr, "%s: duplicated option `--verbose' (%c)\n", basename(argv[0]), opt);
                return 1;
            }
            verbose = 1;
            break;
#if (OPTION_CAN_2_0_ONLY == 0)
        case 'm':
            if (op++) {
                fprintf(stderr, "%s: duplicated option `--mode' (%c)\n", basename(argv[0]), opt);
                return 1;
            }
            if (optarg && optarg[0] == '-') {
                fprintf(stderr, "%s: missing argument for option `--mode' (%c)\n", basename(argv[0]), opt);
                return 1;
            }
            if (!strcasecmp(optarg, "default") || !strcasecmp(optarg, "classic") ||
                !strcasecmp(optarg, "CAN2.0") || !strcasecmp(optarg, "CAN20") || !strcasecmp(optarg, "2.0"))
                opMode.byte |= CANMODE_DEFAULT;
            else if (!strcasecmp(optarg, "CANFD") || !strcasecmp(optarg, "FDF") || !strcasecmp(optarg, "FD"))
                opMode.byte |= CANMODE_FDOE;
            else if (!strcasecmp(optarg, "CANFD+BRS") || !strcasecmp(optarg, "FDF+BRS") || !strcasecmp(optarg, "FD+BRS"))
                opMode.byte |= CANMODE_FDOE | CANMODE_BRSE;
            else {
                fprintf(stderr, "%s: illegal argument for option `--mode' (%c)\n", basename(argv[0]), opt);
                return 1;
            }
            break;
#endif
        case 'S':  /* option `--shared' */
            if (sh++) {
                fprintf(stderr, "%s: duplicated option `--shared'\n", basename(argv[0]));
                return 1;
            }
            opMode.shrd = 1;
            break;
        case 'M':  /* option `--listen-only' */
            if (lo++) {
                fprintf(stderr, "%s: duplicated option `--listen-only'\n", basename(argv[0]));
                return 1;
            }
            opMode.mon = 1;
            break;
        case 'E':  /* option `--error-frames' */
            if (ef++) {
                fprintf(stderr, "%s: duplicated option `--error-frames'\n", basename(argv[0]));
                return 1;
            }
            opMode.err = 1;
            break;
        case 'X':  /* option `--no-extended-frames' */
            if (xf++) {
                fprintf(stderr, "%s: duplicated option `--no-extended-frames'\n", basename(argv[0]));
                return 1;
            }
            opMode.nxtd = 1;
            break;
        case 'R':  /* option `--no-remote-frames' */
            if (rf++) {
                fprintf(stderr, "%s: duplicated option `--no-remote-frames'\n", basename(argv[0]));
                return 1;
            }
            opMode.nrtr = 1;
            break;
        case 't':
            if (mt++) {
                fprintf(stderr, "%s: duplicated option `--time' (%c)\n", basename(argv[0]), opt);
                return 1;
            }
            if (optarg && optarg[0] == '-') {
                fprintf(stderr, "%s: missing argument for option `--time' (%c)\n", basename(argv[0]), opt);
                return 1;
            }
            if (!strcasecmp(optarg, "ABSOLUTE") || !strcasecmp(optarg, "ABS") || !strcasecmp(optarg, "a"))
                modeTime = CCanMessage::OptionAbsolute;
            else if (!strcasecmp(optarg, "RELATIVE") || !strcasecmp(optarg, "REL") || !strcasecmp(optarg, "r"))
                modeTime = CCanMessage::OptionRelative;
            else if (!strcasecmp(optarg, "ZERO") || !strcasecmp(optarg, "0") || !strcasecmp(optarg, "z"))
                modeTime = CCanMessage::OptionZero;
            else {
                fprintf(stderr, "%s: illegal argument for option `--time' (%c)\n", basename(argv[0]), opt);
                return 1;
            }
            if (!CCanMessage::SetTimestampFormat(modeTime)) {
                fprintf(stderr, "%s: illegal argument for option `--time' (%c)\n", basename(argv[0]), opt);
                return 1;
            }
            break;
        case 'i':
            if (mi++) {
                fprintf(stderr, "%s: duplicated option `--id' (%c)\n", basename(argv[0]), opt);
                return 1;
            }
            if (optarg && optarg[0] == '-') {
                fprintf(stderr, "%s: missing argument for option `--id' (%c)\n", basename(argv[0]), opt);
                return 1;
            }
            if (!strcasecmp(optarg, "HEXADECIMAL") || !strcasecmp(optarg, "HEX") || !strcasecmp(optarg, "h") || !strcasecmp(optarg, "16"))
                modeId = CCanMessage::OptionHex;
            else if (!strcasecmp(optarg, "DECIMAL") || !strcasecmp(optarg, "DEC") || !strcasecmp(optarg, "d") || !strcasecmp(optarg, "10"))
                modeId = CCanMessage::OptionDec;
            else if (!strcasecmp(optarg, "OCTAL") || !strcasecmp(optarg, "OCT") || !strcasecmp(optarg, "o") || !strcasecmp(optarg, "8"))
                modeId = CCanMessage::OptionOct;
            else {
                fprintf(stderr, "%s: illegal argument for option `--id' (%c)\n", basename(argv[0]), opt);
                return 1;
            }
            if (!CCanMessage::SetIdentifierFormat(modeId)) {
                fprintf(stderr, "%s: illegal argument for option `--id' (%c)\n", basename(argv[0]), opt);
                return 1;
            }
            break;
        case 'd':
            if (md++) {
                fprintf(stderr, "%s: duplicated option `--data' (%c)\n", basename(argv[0]), opt);
                return 1;
            }
            if (optarg && optarg[0] == '-') {
                fprintf(stderr, "%s: missing argument for option `--data' (%c)\n", basename(argv[0]), opt);
                return 1;
            }
            if (!strcasecmp(optarg, "HEXADECIMAL") || !strcasecmp(optarg, "HEX") || !strcasecmp(optarg, "h") || !strcasecmp(optarg, "16"))
                modeData = CCanMessage::OptionHex;
            else if (!strcasecmp(optarg, "DECIMAL") || !strcasecmp(optarg, "DEC") || !strcasecmp(optarg, "d") || !strcasecmp(optarg, "10"))
                modeData = CCanMessage::OptionDec;
            else if (!strcasecmp(optarg, "OCTAL") || !strcasecmp(optarg, "OCT") || !strcasecmp(optarg, "o") || !strcasecmp(optarg, "8"))
                modeData = CCanMessage::OptionOct;
            else {
                fprintf(stderr, "%s: illegal argument for option `--data' (%c)\n", basename(argv[0]), opt);
                return 1;
            }
            if (!CCanMessage::SetDataFormat(modeData)) {
                fprintf(stderr, "%s: illegal argument for option `--data' (%c)\n", basename(argv[0]), opt);
                return 1;
            }
            break;
        case 'a':
            if (ma++) {
                fprintf(stderr, "%s: duplicated option `--ascii' (%c)\n", basename(argv[0]), opt);
                return 1;
            }
            if (optarg && optarg[0] == '-') {
                fprintf(stderr, "%s: missing argument for option `--ascii' (%c)\n", basename(argv[0]), opt);
                return 1;
            }
            if (!strcasecmp(optarg, "OFF") || !strcasecmp(optarg, "NO") || !strcasecmp(optarg, "n") || !strcasecmp(optarg, "0"))
                modeAscii = CCanMessage::OptionOff;
            else if (!strcasecmp(optarg, "ON") || !strcasecmp(optarg, "YES") || !strcasecmp(optarg, "y") || !strcasecmp(optarg, "1"))
                modeAscii = CCanMessage::OptionOn;
            else {
                fprintf(stderr, "%s: illegal argument for option `--ascii' (%c)\n", basename(argv[0]), opt);
                return 1;
            }
            if (!CCanMessage::SetAsciiFormat(modeAscii)) {
                fprintf(stderr, "%s: illegal argument for option `--ascii' (%c)\n", basename(argv[0]), opt);
                return 1;
            }
            break;
        case 'w':
            if (mw++) {
                fprintf(stderr, "%s: duplicated option `--wrap' (%c)\n", basename(argv[0]), opt);
                return 1;
            }
            if (optarg && optarg[0] == '-') {
                fprintf(stderr, "%s: missing argument for option `--wrap' (%c)\n", basename(argv[0]), opt);
                return 1;
            }
            if (!strcasecmp(optarg, "NO") || !strcasecmp(optarg, "n") || !strcasecmp(optarg, "0"))
                wraparound = CCanMessage::OptionWraparoundNo;
            else if (!strcasecmp(optarg, "8"))
                wraparound = CCanMessage::OptionWraparound8;
#if (OPTION_CAN_2_0_ONLY == 0)
            else if (!strcasecmp(optarg, "10"))
                wraparound = CCanMessage::OptionWraparound10;
            else if (!strcasecmp(optarg, "16"))
                wraparound = CCanMessage::OptionWraparound16;
            else if (!strcasecmp(optarg, "32"))
                wraparound = CCanMessage::OptionWraparound32;
            else if (!strcasecmp(optarg, "64"))
                wraparound = CCanMessage::OptionWraparound64;
#endif
            else {
                fprintf(stderr, "%s: illegal argument for option `--wrap' (%c)\n", basename(argv[0]), opt);
                return 1;
            }
            if (!CCanMessage::SetWraparound(wraparound)) {
                fprintf(stderr, "%s: illegal argument for option `--wrap' (%c)\n", basename(argv[0]), opt);
                return 1;
            }
            break;
        case 'x':
            if (exclude++) {
                fprintf(stderr, "%s: duplicated option `--exclude' (%c)\n", basename(argv[0]), opt);
                return 1;
            }
            if (optarg && optarg[0] == '-') {
                fprintf(stderr, "%s: missing argument for option `--exclude' (%c)\n", basename(argv[0]), opt);
                return 1;
            }
            if (!get_exclusion(optarg)) {
                fprintf(stderr, "%s: illegal argument for option `--exclude' (%c)\n", basename(argv[0]), opt);
                return 1;
            }
            break;
        case 'h':
            usage(stdout, basename(argv[0]));
            return 0;
        case '?':
            if (!opterr)
                usage(stderr, basename(argv[0]));
            return 1;
        default:
            if (show_version) {
                version(stdout, basename(argv[0]));
                return 0;
            }
            else {
                usage(stderr, basename(argv[0]));
                return 1;
            }
        }
    }
    /* - check if one and only one <interface> is given */
    if (optind + 1 != argc) {
        if (optind == argc)
            fprintf(stderr, "%s: no interface given\n", basename(argv[0]));
        else
            fprintf(stderr, "%s: too many arguments given\n", basename(argv[0]));
        return 1;
    }
#if (OPTION_CAN_2_0_ONLY == 0)
    /* - check bit-timing index (n/a for CAN FD) */
    if (opMode.fdoe && (bitrate.btr.frequency <= 0)) {
        fprintf(stderr, "%s: illegal combination of options `--mode' (m) and `--bitrate'\n", basename(argv[0]));
        return 1;
    }
#endif
    /* - take serial device name from command line */
    port = (char*)argv[optind];
    /* CAN Monitor for CAN-over-Serial-Line interfaces */
    fprintf(stdout, "%s\n%s\n\n%s\n\n", APPLICATION, COPYRIGHT, WARRANTY);

    /* - show operation mode and bit-rate settings */
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
            fprintf(stdout, "Bit-rate=%.0fkbps@%.1f%%",
                speed.nominal.speed / 1000.,
                speed.nominal.samplepoint * 100.);
#if (OPTION_CAN_2_0_ONLY == 0)
            if (speed.data.brse)
                fprintf(stdout, ":%.0fkbps@%.1f%%",
                    speed.data.speed / 1000.,
                    speed.data.samplepoint * 100.);
#endif
            fprintf(stdout, " (f_clock=%i,nom_brp=%u,nom_tseg1=%u,nom_tseg2=%u,nom_sjw=%u",
                bitrate.btr.frequency,
                bitrate.btr.nominal.brp,
                bitrate.btr.nominal.tseg1,
                bitrate.btr.nominal.tseg2,
                bitrate.btr.nominal.sjw);
#if (OPTION_CAN_2_0_ONLY == 0)
            if (speed.data.brse)
                fprintf(stdout, ",data_brp=%u,data_tseg1=%u,data_tseg2=%u,data_sjw=%u",
                    bitrate.btr.data.brp,
                    bitrate.btr.data.tseg1,
                    bitrate.btr.data.tseg2,
                    bitrate.btr.data.sjw);
            else
#endif
                fprintf(stdout, ",nom_sam=%u", bitrate.btr.nominal.sam);
            fprintf(stdout, ")\n\n");
        }
        else {
            fprintf(stdout, "Baudrate=%.0fkbps@%.1f%% (index %i)\n\n",
                             speed.nominal.speed / 1000.,
                             speed.nominal.samplepoint * 100., -bitrate.index);
        }
    }
    /* - initialize interface */
    fprintf(stdout, "Hardware=%s...", port);
    fflush (stdout);
    retVal = canDriver.InitializeChannel(port, opMode);
    if (retVal != CCanApi::NoError) {
        fprintf(stdout, "FAILED!\n");
        fprintf(stderr, "+++ error: CAN Controller could not be initialized (%i)\n", retVal);
        goto finalize;
    }
    fprintf(stdout, "OK!\n");
    /* - start communication */
    if (bitrate.btr.frequency > 0) {
        fprintf(stdout, "Bit-rate=%.0fkbps",
            speed.nominal.speed / 1000.);
#if (OPTION_CAN_2_0_ONLY == 0)
        if (speed.data.brse)
            fprintf(stdout, ":%.0fkbps",
                speed.data.speed / 1000.);
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
    retVal = canDriver.StartController(bitrate);
    if (retVal != CCanApi::NoError) {
        fprintf(stdout, "FAILED!\n");
        fprintf(stderr, "+++ error: CAN Controller could not be started (%i)\n", retVal);
        goto teardown;
    }
    fprintf(stdout, "OK!\n");
    /* - do your job well: */
    canDriver.ReceptionLoop();
    /* - show interface information */
    if ((device = canDriver.GetHardwareVersion()) != NULL)
        fprintf(stdout, "Hardware: %s\n", device);
    if ((firmware = canDriver.GetFirmwareVersion()) != NULL)
        fprintf(stdout, "Firmware: %s\n", firmware);
    if ((software = CCanDriver::GetVersion()) != NULL)
        fprintf(stdout, "Software: %s\n", software);
teardown:
    /* - teardown the interface*/
    retVal = canDriver.TeardownChannel();
    if (retVal != CCanApi::NoError) {
        fprintf(stderr, "+++ error: CAN Controller could not be reset (%i)\n", retVal);
        goto finalize;
    }
finalize:
    /* So long and farewell! */
    fprintf(stdout, "%s\n", COPYRIGHT);
    return retVal;
}

int CCanDriver::ListCanDevices(const char *vendor) {
    int32_t library = EOF; int n = 0;

    if (vendor != NULL) {
        /* search library ID in the vendor list */
        for (int32_t i = 0; CCanDriver::m_CanVendors[i].id != EOF; i++) {
            if (!strcmp(vendor, CCanDriver::m_CanVendors[i].name)) {
                library = CCanDriver::m_CanVendors[i].id;
                break;
            }
        }
        fprintf(stdout, "Suppored hardware from \"%s\":\n", vendor);
    }
    else
        fprintf(stdout, "Suppored hardware:\n");
    for (int32_t i = 0; CCanDriver::m_CanDevices[i].library != EOF; i++) {
        /* list all boards or from a specific vendor */
        if ((vendor == NULL) || (library == CCanDriver::m_CanDevices[i].library) ||
            !strcmp(vendor, "*")) { // TODO: pattern matching
            fprintf(stdout, "\"%s\" ", CCanDriver::m_CanDevices[i].name);
            /* search vendor name in the vendor list */
            for (int32_t j = 0; CCanDriver::m_CanVendors[j].id != EOF; j++) {
                if (CCanDriver::m_CanDevices[i].library == CCanDriver::m_CanVendors[j].id) {
                    fprintf(stdout, "(VendorName=\"%s\", LibraryId=%" PRIi32 ", AdapterId=%" PRIi32 ")",
                            CCanDriver::m_CanVendors[j].name, CCanDriver::m_CanDevices[i].library, CCanDriver::m_CanDevices[i].adapter);
                    break;
                }
            }
            fprintf(stdout, "\n");
            n++;
        }
    }
    return n;
}

int CCanDriver::TestCanDevices(CANAPI_OpMode_t opMode, const char *vendor) {
    int32_t library = EOF; int n = 0;

    if (vendor != NULL) {
        /* search library ID in the vendor list */
        for (int32_t i = 0; CCanDriver::m_CanVendors[i].id != EOF; i++) {
            if (!strcmp(vendor, CCanDriver::m_CanVendors[i].name)) {
                library = CCanDriver::m_CanVendors[i].id;
                break;
            }
        }
    }
    for (int32_t i = 0; CCanDriver::m_CanDevices[i].library != EOF; i++) {
        /* test all boards or from a specific vendor */
        if ((vendor == NULL) || (library == CCanDriver::m_CanDevices[i].library) ||
            !strcmp(vendor, "*")) { // TODO: pattern matching
            fprintf(stdout, "Hardware=%s...", CCanDriver::m_CanDevices[i].name);
            fflush(stdout);
            EChannelState state;
            CANAPI_Return_t retVal = CCanDriver::ProbeChannel(CCanDriver::m_CanDevices[i].adapter, opMode, state);
            if ((retVal == CCanApi::NoError) || (retVal == CCanApi::IllegalParameter)) {
                CTimer::Delay(333U * CTimer::MSEC);  // to fake probing a hardware
                switch (state) {
                    case CCanApi::ChannelOccupied: fprintf(stdout, "occupied\n"); n++; break;
                    case CCanApi::ChannelAvailable: fprintf(stdout, "available\n"); n++; break;
                    case CCanApi::ChannelNotAvailable: fprintf(stdout, "not available\n"); break;
                    default: fprintf(stdout, "not testable\n"); break;
                }
                if (retVal == CCanApi::IllegalParameter)
                    fprintf(stderr, "+++ warning: CAN operation mode not supported (%02x)\n", opMode.byte);
            } else
                fprintf(stdout, "FAILED!\n");
        }
    }
    return n;
}

uint64_t CCanDriver::ReceptionLoop() {
    CANAPI_Message_t message;
    CANAPI_Return_t retVal;
    uint64_t frames = 0U;

    char string[CANPROP_MAX_STRING_LENGTH+1];
    memset(string, 0, CANPROP_MAX_STRING_LENGTH+1);

    fprintf(stderr, "\nPress ^C to abort.\n\n");
    while(running) {
        if ((retVal = ReadMessage(message)) == CCanApi::NoError) {
            if ((((message.id < MAX_ID) && can_id[message.id]) || ((message.id >= MAX_ID) && can_id_xtd)) &&
                !message.sts) {
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
    (void)canDriver.SignalChannel();
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
    fprintf(stream, " -t, --time=(ZERO|ABS|REL)     absolute or relative time (default=0)\n");
    fprintf(stream, " -i  --id=(HEX|DEC|OCT)        display mode of CAN-IDs (default=HEX)\n");
    fprintf(stream, " -d, --data=(HEX|DEC|OCT)      display mode of data bytes (default=HEX)\n");
    fprintf(stream, " -a, --ascii=(ON|OFF)          display data bytes in ASCII (default=ON)\n");
#if (OPTION_CAN_2_0_ONLY == 0)
    fprintf(stream, " -w, --wrap=(NO|8|10|16|32|64) wraparound after n data bytes (default=NO)\n");
#endif
    fprintf(stream, " -x, --exclude=[~]<id-list>    exclude CAN-IDs: <id>[-<id>]{,<id>[-<id>]}\n");
//    fprintf(stream, " -s, --script=<filename>       execute a script file\n"); // TODO: script engine
#if (OPTION_CAN_2_0_ONLY == 0)
    fprintf(stream, " -m, --mode=(2.0|FDF[+BSR])    CAN operation mode: CAN 2.0 or CAN FD format\n");
#endif
    fprintf(stream, "     --shared                  shared CAN controller access (when supported)\n");
    fprintf(stream, "     --listen-only             monitor mode (listen-only, transmitter is off)\n");
    fprintf(stream, "     --error-frames            allow reception of error frames\n");
    fprintf(stream, "     --no-remote-frames        suppress remote frames (RTR frames)\n");
    fprintf(stream, "     --no-extended-frames      suppress extended frames (29-bit identifier)\n");
    fprintf(stream, " -b, --baudrate=<baudrate>     CAN 2.0 bit timing in kbps (default=250)\n");
#if (OPTION_CAN_2_0_ONLY == 0)
    fprintf(stream, "     --bitrate=<bit-rate>      CAN FD bit rate (as a string)\n");
#endif
    fprintf(stream, " -v, --verbose                 show detailed bit rate settings\n");
    fprintf(stream, " -h, --help                    display this help screen and exit\n");
    fprintf(stream, "     --version                 show version information and exit\n");
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
    fprintf(stream, "Written by Uwe Vogt, UV Software, Berlin <https://uv-software.com/>\n");
}
