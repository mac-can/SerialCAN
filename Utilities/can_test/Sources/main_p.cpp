//  SPDX-License-Identifier: GPL-3.0-or-later
//
//  CAN Tester for generic Interfaces (CAN API V3)
//
//  Copyright (c) 2008-2010,2012-2024 Uwe Vogt, UV Software, Berlin (info@mac-can.com)
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
#include "Version.h"

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

#define RxMODE  (0)
#define TxMODE  (1)
#define TxFRAMES  (2)
#define TxRANDOM  (3)
class CCanDevice : public CCanDriver {
public:
    uint64_t ReceiverTest(bool checkCounter = false, uint64_t expectedNumber = 0U, bool stopOnError = false);
    uint64_t TransmitterTest(time_t duration, CANAPI_OpMode_t opMode, uint32_t id = 0x100U, uint8_t dlc = 0U, uint64_t delay = 0U, uint64_t offset = 0U);
    uint64_t TransmitterTest(uint64_t count, CANAPI_OpMode_t opMode, bool random = false, uint32_t id = 0x100U, uint8_t dlc = 0U, uint64_t delay = 0U, uint64_t offset = 0U);
public:
    static int ListCanDevices(void);
    static int TestCanDevices(CANAPI_OpMode_t opMode);
    static int ListCanBitrates(CANAPI_OpMode_t opMode);
};

static void sigterm(int signo);
static void usage(FILE *stream, const char *program);
static void version(FILE *stream, const char *program);

static const char *prompt[4] = {"-\b", "/\b", "|\b", "\\\b"};
static volatile int running = 1;

static CCanDevice canDevice = CCanDevice();

static const char APPLICATION[] = "CAN Tester for " TESTER_INTEFACE ", Version " VERSION_STRING;
static const char COPYRIGHT[]   = "Copyright (c) " TESTER_COPYRIGHT;
static const char WARRANTY[]    = "This program comes with ABSOLUTELY NO WARRANTY!\n\n" \
                                  "This is free software, and you are welcome to redistribute it\n" \
                                  "under certain conditions; type '--version' for details.";
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
#define basename(x)  "can_test" // FIXME: Where is my `basename' function?

int main(int argc, const char * argv[]) {
    int opt;
#if (SERIAL_CAN_SUPPORTED == 0)
    CCanDevice::SChannelInfo channel;
#else
    char *port = NULL;
#endif
    int mode = RxMODE, m = 0;
    int op = 0, rf = 0, xf = 0, ef = 0, lo = 0, sh = 0;
    long baudrate = CANBDR_250; int bd = 0;
    int verbose = 0;
    time_t txtime = 0;
    long txframes = 0;
    long can_id = 0x100; int c = 0;
    long can_dlc = 8; int d = 0;
    long delay = 0; int t = 0;
    long number = 0; int n = 0;
    int stop_on_error = 0;
    int num_boards = 0;
    int show_version = 0;
    char *device, *firmware, *software;
    char property[CANPROP_MAX_BUFFER_SIZE] = "";
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
        {"list-boards", no_argument, 0, 'L'},
        {"test-boards", no_argument, 0, 'T'},
        {"help", no_argument, 0, 'h'},
        {"version", no_argument, &show_version, 1},
        {0, 0, 0, 0}
    };
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
    while ((opt = getopt_long(argc, (char * const *)argv, "b:vm:rn:st:f:R:c:u:d:i:lLaTh", long_options, NULL)) != -1) {
        switch (opt) {
        /* option `--baudrate=<baudrate>' (-b) */
        case 'b':
            if (bd++) {
                fprintf(stderr, "%s: duplicated option `--baudrate' (%c)\n", basename(argv[0]), opt);
                return 1;
            }
            if (sscanf(optarg, "%li", &baudrate) != 1) {
                fprintf(stderr, "%s: illegal argument for option `--baudrate' (%c)\n", basename(argv[0]), opt);
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
                fprintf(stderr, "%s: illegal argument for option `--baudrate' (%c)\n", basename(argv[0]), opt);
                return 1;
            }
            if (CCanDevice::MapBitrate2Speed(bitrate, speed) != CCanApi::NoError) {
                fprintf(stderr, "%s: illegal argument for option `--baudrate' (%c)\n", basename(argv[0]), opt);
                return 1;
            }
            break;
        /* option `--bitrate=<bit-rate>' as string */
        case 'B':
            if (bd++) {
                fprintf(stderr, "%s: duplicated option `--bitrate'\n", basename(argv[0]));
                return 1;
            }
            if (CCanDevice::MapString2Bitrate(optarg, bitrate, hasDataPhase, hasNoSamp) != CCanApi::NoError) {
                fprintf(stderr, "%s: illegal argument for option `--bitrate'\n", basename(argv[0]));
                return 1;
            }
            if (CCanDevice::MapBitrate2Speed(bitrate, speed) != CCanApi::NoError) {
                fprintf(stderr, "%s: illegal argument for option `--bitrate'\n", basename(argv[0]));
                return 1;
            }
            break;
        /* option `--verbose' (-v) */
        case 'v':
            if (verbose) {
                fprintf(stderr, "%s: duplicated option `--verbose' (%c)\n", basename(argv[0]), opt);
                return 1;
            }
            verbose = 1;
            break;
        /* option `--mode=(2.0|FDF[+BRS])' (-m) */
        case 'm':
            if (op++) {
                fprintf(stderr, "%s: duplicated option `--mode' (%c)\n", basename(argv[0]), opt);
                return 1;
            }
            if (!strcasecmp(optarg, "default") || !strcasecmp(optarg, "classic") || !strcasecmp(optarg, "classical") ||
                !strcasecmp(optarg, "CAN20") || !strcasecmp(optarg, "CAN2.0") || !strcasecmp(optarg, "2.0"))
                opMode.byte |= CANMODE_DEFAULT;
#if (CAN_FD_SUPPORTED != 0)
            else if (!strcasecmp(optarg, "CANFD") || !strcasecmp(optarg, "FD") || !strcasecmp(optarg, "FDF"))
                opMode.byte |= CANMODE_FDOE;
            else if (!strcasecmp(optarg, "CANFD+BRS") || !strcasecmp(optarg, "FD+BRS") || !strcasecmp(optarg, "FDF+BRS"))
                opMode.byte |= CANMODE_FDOE | CANMODE_BRSE;
#endif
            else {
                fprintf(stderr, "%s: illegal argument for option `--mode' (%c)\n", basename(argv[0]), opt);
                return 1;
            }
            break;
        /* option `--shared' */
        case 'S':
            if (sh++) {
                fprintf(stderr, "%s: duplicated option `--shared'\n", basename(argv[0]));
                return 1;
            }
            opMode.shrd = 1;
            break;
        /* option `--listen-only' */
        case 'M':
            if (lo++) {
                fprintf(stderr, "%s: duplicated option `--listen-only'\n", basename(argv[0]));
                return 1;
            }
            opMode.mon = 1;
            break;
        /* option `--error-frames' */
        case 'E':
            if (ef++) {
                fprintf(stderr, "%s: duplicated option `--error-frames'\n", basename(argv[0]));
                return 1;
            }
            opMode.err = 1;
            break;
        /* option `--no-extended-frames' */
        case 'X':
            if (xf++) {
                fprintf(stderr, "%s: duplicated option `--no-extended-frames'\n", basename(argv[0]));
                return 1;
            }
            opMode.nxtd = 1;
            break;
        /* option `--no-remote-frames' */
        case 'R':
            if (rf++) {
                fprintf(stderr, "%s: duplicated option `--no-remote-frames'\n", basename(argv[0]));
                return 1;
            }
            opMode.nrtr = 1;
            break;
        /* option `--receive' (-r) */
        case 'r':
            if (m++) {
                fprintf(stderr, "%s: duplicated option `--receive' (%c)\n", basename(argv[0]), opt);
                return 1;
            }
            mode = RxMODE;
            break;
        /* option `--number=<offset>' (-n) */
        case 'n':
            if (n++) {
                fprintf(stderr, "%s: duplicated option `--number' (%c)\n", basename(argv[0]), opt);
                return 1;
            }
            if (sscanf(optarg, "%li", &number) != 1) {
                fprintf(stderr, "%s: illegal argument for option `--number' (%c)\n", basename(argv[0]), opt);
                return 1;
            }
            if (number < 0) {
                fprintf(stderr, "%s: illegal argument for option `--number' (%c)\n", basename(argv[0]), opt);
                return 1;
            }
            break;
        /* option `--stop' (-s) */
        case 's':
            if (stop_on_error) {
                fprintf(stderr, "%s: duplicated option `--stop' (%c)\n", basename(argv[0]), opt);
                return 1;
            }
            stop_on_error = 1;
            break;
        /* option `--transmit=<duration>' (-t) in [s] */
        case 't':
            if (m++) {
                fprintf(stderr, "%s: duplicated option `--transmit' (%c)\n", basename(argv[0]), opt);
                return 1;
            }
            if (sscanf(optarg, "%li", &txtime) != 1) {
                fprintf(stderr, "%s: illegal argument for option `--transmit' (%c)\n", basename(argv[0]), opt);
                return 1;
            }
            if (txtime < 0) {
                fprintf(stderr, "%s: illegal argument for option `--transmit' (%c)\n", basename(argv[0]), opt);
                return 1;
            }
            mode = TxMODE;
            break;
        /* option `--frames=<frames>' (-f) */
        case 'f':
            if (m++) {
                fprintf(stderr, "%s: duplicated option `--frames' (%c)\n", basename(argv[0]), opt);
                return 1;
            }
            if (sscanf(optarg, "%li", &txframes) != 1) {
                fprintf(stderr, "%s: illegal argument for option `--frames' (%c)\n", basename(argv[0]), opt);
                return 1;
            }
            if (txframes < 0) {
                fprintf(stderr, "%s: illegal argument for option `--frames' (%c)\n", basename(argv[0]), opt);
                return 1;
            }
            mode = TxFRAMES;
            break;
        /* option `--random=<frames>' */
        case 'F':
            if (m++) {
                fprintf(stderr, "%s: duplicated option `--random'\n", basename(argv[0]));
                return 1;
            }
            if (sscanf(optarg, "%li", &txframes) != 1) {
                fprintf(stderr, "%s: illegal argument for option `--random'\n", basename(argv[0]));
                return 1;
            }
            if (txframes < 0) {
                fprintf(stderr, "%s: illegal argument for option `--random' (%c)\n", basename(argv[0]), opt);
                return 1;
            }
            if (!d) /* let the tester generate messages of arbitrary length */
                can_dlc = 0;
            mode = TxRANDOM;
            break;
        /* option `--cycle=<msec>' (-c) */
        case 'c':
            if (t++) {
                fprintf(stderr, "%s: duplicated option `--cycle' (%c)\n", basename(argv[0]), opt);
                return 1;
            }
            if (sscanf(optarg, "%li", &delay) != 1) {
                fprintf(stderr, "%s: illegal argument for option `--cycle' (%c)\n", basename(argv[0]), opt);
                return 1;
            }
            if ((delay < 0) || (delay > (LONG_MAX / 1000l))) {
                fprintf(stderr, "%s: illegal argument for option `--cycle' (%c)\n", basename(argv[0]), opt);
                return 1;
            }
            delay *= 1000l;
            break;
        /* option `--usec=<usec>' (-u) */
        case 'u':
            if (t++) {
                fprintf(stderr, "%s: duplicated option `--usec' (%c)\n", basename(argv[0]), opt);
                return 1;
            }
            if (sscanf(optarg, "%li", &delay) != 1) {
                fprintf(stderr, "%s: illegal argument for option `--usec' (%c)\n", basename(argv[0]), opt);
                return 1;
            }
            if (delay < 0) {
                fprintf(stderr, "%s: illegal argument for option `--usec' (%c)\n", basename(argv[0]), opt);
                return 1;
            }
            break;
        /* option `--dlc=<length>' (-d) */
        case 'd':
            if (d++) {
                fprintf(stderr, "%s: duplicated option `--dlc' (%c)\n", basename(argv[0]), opt);
                return 1;
            }
            if (sscanf(optarg, "%li", &can_dlc) != 1) {
                fprintf(stderr, "%s: illegal argument for option `--dlc' (%c)\n", basename(argv[0]), opt);
                return 1;
            }
#if (CAN_FD_SUPPORTED != 0)
            if ((can_dlc < 0) || (CANFD_MAX_LEN < can_dlc)) {
#else
            if ((can_dlc < 0) || (CAN_MAX_LEN < can_dlc)) {
#endif
                fprintf(stderr, "%s: illegal argument for option `--dlc' (%c)\n", basename(argv[0]), opt);
                return 1;
            }
            break;
        /* option `--id=<identifier>' (-i) */
        case 'i':
            if (c++) {
                fprintf(stderr, "%s: duplicated option `--id' (%c)\n", basename(argv[0]), opt);
                return 1;
            }
            if (sscanf(optarg, "%li", &can_id) != 1) {
                fprintf(stderr, "%s: illegal argument for option `--id' (%c)\n", basename(argv[0]), opt);
                return 1;
            }
            if ((can_id < 0x000) || (0x1FFFFFFF < can_id)) { // TODO: to be checked with --mode=NXTD
                fprintf(stderr, "%s: illegal argument for option `--id' (%c)\n", basename(argv[0]), opt);
                return 1;
            }
            break;
        /* option `--list-bitrates[=(2.0|FDF[+BRS])]' */
        case 'l':
            if (optarg != NULL) {
                if (!strcasecmp(optarg, "default") || !strcasecmp(optarg, "classic") || !strcasecmp(optarg, "classical") ||
                    !strcasecmp(optarg, "CAN20") || !strcasecmp(optarg, "CAN2.0") || !strcasecmp(optarg, "2.0"))
                    brMode.byte |= CANMODE_DEFAULT;
#if (CAN_FD_SUPPORTED != 0)
                else if (!strcasecmp(optarg, "CANFD") || !strcasecmp(optarg, "FD") || !strcasecmp(optarg, "FDF"))
                    brMode.byte |= CANMODE_FDOE;
                else if (!strcasecmp(optarg, "CANFD+BRS") || !strcasecmp(optarg, "FD+BRS") || !strcasecmp(optarg, "FDF+BRS"))
                    brMode.byte |= CANMODE_FDOE | CANMODE_BRSE;
#endif
                else {
                    fprintf(stderr, "%s: illegal argument for option `--list-bitrates' (%c)\n", basename(argv[0]), opt);
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
        case 'a':  /* (-a, deprecated) */
        case 'L':  
            fprintf(stdout, "%s\n%s\n\n%s\n\n", APPLICATION, COPYRIGHT, WARRANTY);
            /* list all supported interfaces */
            num_boards = CCanDevice::ListCanDevices(/*optarg*/);
            fprintf(stdout, "Number of supported CAN interfaces: %i\n", num_boards);
            return (num_boards >= 0) ? 0 : 1;
        /* option `--test-boards[=<vendor>]' (-T) */
        case 'T':
            fprintf(stdout, "%s\n%s\n\n%s\n\n", APPLICATION, COPYRIGHT, WARRANTY);
            /* list all available interfaces */
            num_boards = CCanDevice::TestCanDevices(opMode/*, optarg*/);
            fprintf(stdout, "Number of present CAN interfaces: %i\n", num_boards);
            return (num_boards >= 0) ? 0 : 1;
        /* option `--help' (-h) */
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
#if (SERIAL_CAN_SUPPORTED == 0)
    /* - search the <interface> by its name in the device list */
    bool result = CCanDevice::GetFirstChannel(channel);
    while (result) {
        if (strcasecmp(argv[optind], channel.m_szDeviceName) == 0) {
            break;
        }
        result = CCanDevice::GetNextChannel(channel);
    }
    if (!result) {
        fprintf(stderr, "%s: illegal argument `%s'\n", basename(argv[0]), argv[optind]);
        return 1;
    }
#else
    /* - take serial device name from command line */
    port = (char*)argv[optind];
#endif
#if (CAN_FD_SUPPORTED != 0)
    /* - check bit-timing index (n/a for CAN FD) */
    if (opMode.fdoe && (bitrate.btr.frequency <= 0)) {
        fprintf(stderr, "%s: illegal combination of options `--mode' (m) and `--bitrate'\n", basename(argv[0]));
        return 1;
    }
    /* - check data length and make CAN FD DLC (0x0..0xF) */
    if (!opMode.fdoe && (can_dlc > CAN_MAX_LEN)) {
        fprintf(stderr, "%s: illegal combination of options `--mode' (m) and `--dlc' (d)\n", basename(argv[0]));
        return 1;
    } else {
        if (can_dlc > 48) can_dlc = 0xF;
        else if (can_dlc > 32) can_dlc = 0xE;
        else if (can_dlc > 24) can_dlc = 0xD;
        else if (can_dlc > 20) can_dlc = 0xC;
        else if (can_dlc > 16) can_dlc = 0xB;
        else if (can_dlc > 12) can_dlc = 0xA;
        else if (can_dlc > 8) can_dlc = 0x9;
    }
#endif
    /* - check operation mode flags */
    if ((mode != RxMODE) && opMode.mon) {
        fprintf(stderr, "%s: illegal option `--listen-only' for transmitter test\n", basename(argv[0]));
        return 1;
    }
    if ((mode != RxMODE) && opMode.err) {
        fprintf(stderr, "%s: illegal option `--error-frames' for transmitter test\n", basename(argv[0]));
        return 1;
    }
    if ((mode != RxMODE) && opMode.nxtd) {
        fprintf(stderr, "%s: illegal option `--no-extended-frames' for transmitter test\n", basename(argv[0]));
        return 1;
    }
    if ((mode != RxMODE) && opMode.nrtr) {
        fprintf(stderr, "%s: illegal option `--no-remote-frames' for transmitter test\n", basename(argv[0]));
        return 1;
    }
    /* CAN Tester for generic CAN interfaces */
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
    /* - do your job well: */
    switch (mode) {
    case TxMODE:    /* transmitter test (duration) */
        (void)canDevice.TransmitterTest((time_t)txtime, opMode, (uint32_t)can_id, (uint8_t)can_dlc, (uint64_t)delay, (uint64_t)number);
        break;
    case TxFRAMES:  /* transmitter test (frames) */
        (void)canDevice.TransmitterTest((uint64_t)txframes, opMode, false, (uint32_t)can_id, (uint8_t)can_dlc, (uint64_t)delay, (uint64_t)number);
        break;
    case TxRANDOM:  /* transmitter test (random) */
        (void)canDevice.TransmitterTest((uint64_t)txframes, opMode, true, (uint32_t)can_id, (uint8_t)can_dlc, (uint64_t)delay, (uint64_t)number);
        break;
    default:        /* receiver test (abort with Ctrl+C) */
        (void)canDevice.ReceiverTest((bool)n, (uint64_t)number, (bool)stop_on_error);
        break;
    }
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

uint64_t CCanDevice::TransmitterTest(time_t duration, CANAPI_OpMode_t opMode, uint32_t id, uint8_t dlc, uint64_t delay, uint64_t offset) {
    CANAPI_Message_t message;
    CANAPI_Return_t retVal;

    time_t start = time(NULL);
    uint64_t frames = 0;
    uint64_t errors = 0;
    uint64_t calls = 0;

    memset(&message, 0, sizeof(CANAPI_Message_t));

    fprintf(stderr, "\nPress ^C to abort.\n");
    message.id  = id;
    message.xtd = 0;
    message.rtr = 0;
#if (CAN_FD_SUPPORTED != 0)
    message.fdf = opMode.fdoe;
    message.brs = opMode.brse;
#else
    (void) opMode;
#endif
    message.dlc = dlc;
    fprintf(stdout, "\nTransmitting message(s)...");
    fflush (stdout);
    while (time(NULL) < (start + duration)) {
        message.data[0] = (uint8_t)((frames + offset) >> 0);
        message.data[1] = (uint8_t)((frames + offset) >> 8);
        message.data[2] = (uint8_t)((frames + offset) >> 16);
        message.data[3] = (uint8_t)((frames + offset) >> 24);
        message.data[4] = (uint8_t)((frames + offset) >> 32);
        message.data[5] = (uint8_t)((frames + offset) >> 40);
        message.data[6] = (uint8_t)((frames + offset) >> 48);
        message.data[7] = (uint8_t)((frames + offset) >> 56);
#if (CAN_FD_SUPPORTED != 0)
        memset(&message.data[8], 0, CANFD_MAX_LEN - 8);
#endif
        /* transmit message (repeat when busy) */
retry_tx_test:
        calls++;
        retVal = WriteMessage(message);
        if (retVal == CCanApi::NoError)
            fprintf(stderr, "%s", prompt[(frames++ % 4)]);
        else if ((retVal == CCanApi::TransmitterBusy) && running)
            goto retry_tx_test;
        else
            errors++;
        /* pause between two messages, as you please */
        CTimer::Delay(delay * CTimer::USEC);
        if (!running) {
            fprintf(stderr, "\b");
            fprintf(stdout, "STOP!\n\n");
            fprintf(stdout, "Message(s)=%" PRIu64 "\n", frames);
            fprintf(stdout, "Error(s)=%" PRIu64 "\n", errors);
            fprintf(stdout, "Call(s)=%" PRIu64 "\n", calls);
            fprintf(stdout, "Time=%lisec\n\n", time(NULL) - start);
            return frames;
        }
    }
    fprintf(stderr, "\b");
    fprintf(stdout, "OK!\n\n");
    fprintf(stdout, "Message(s)=%" PRIu64 "\n", frames);
    fprintf(stdout, "Error(s)=%" PRIu64 "\n", errors);
    fprintf(stdout, "Call(s)=%" PRIu64 "\n", calls);
    fprintf(stdout, "Time=%lisec\n\n", time(NULL) - start);

    CTimer::Delay(1U * CTimer::SEC);  /* afterburner */
    return frames;
}

uint64_t CCanDevice::TransmitterTest(uint64_t count, CANAPI_OpMode_t opMode, bool random, uint32_t id, uint8_t dlc, uint64_t delay, uint64_t offset) {
    CANAPI_Message_t message;
    CANAPI_Return_t retVal;

    time_t start = time(NULL);
    uint64_t frames = 0;
    uint64_t errors = 0;
    uint64_t calls = 0;

    srand((unsigned int)time(NULL));
    memset(&message, 0, sizeof(CANAPI_Message_t));

    fprintf(stderr, "\nPress ^C to abort.\n");
    message.id  = id;
    message.xtd = 0;
    message.rtr = 0;
#if (CAN_FD_SUPPORTED != 0)
    message.fdf = opMode.fdoe;
    message.brs = opMode.brse;
#else
    (void) opMode;
#endif
    message.dlc = dlc;
    fprintf(stdout, "\nTransmitting message(s)...");
    fflush (stdout);
    while (frames < count) {
        message.data[0] = (uint8_t)((frames + offset) >> 0);
        message.data[1] = (uint8_t)((frames + offset) >> 8);
        message.data[2] = (uint8_t)((frames + offset) >> 16);
        message.data[3] = (uint8_t)((frames + offset) >> 24);
        message.data[4] = (uint8_t)((frames + offset) >> 32);
        message.data[5] = (uint8_t)((frames + offset) >> 40);
        message.data[6] = (uint8_t)((frames + offset) >> 48);
        message.data[7] = (uint8_t)((frames + offset) >> 56);
#if (CAN_FD_SUPPORTED != 0)
        memset(&message.data[8], 0, CANFD_MAX_LEN - 8);
        if (random)
            message.dlc = dlc + (uint8_t)(rand() % ((CANFD_MAX_DLC - dlc) + 1));
#else
        if (random)
            message.dlc = dlc + (uint8_t)(rand() % ((CAN_MAX_DLC - dlc) + 1));
#endif
        /* transmit message (repeat when busy) */
retry_tx_test:
        calls++;
        retVal = WriteMessage(message);
        if (retVal == CCanApi::NoError)
            fprintf(stderr, "%s", prompt[(frames++ % 4)]);
        else if ((retVal == CCanApi::TransmitterBusy) && running)
            goto retry_tx_test;
        else
            errors++;
        /* pause between two messages, as you please */
        if (random)
            CTimer::Delay(CTimer::USEC * (delay + (uint64_t)(rand() % 54945)));
        else
            CTimer::Delay(CTimer::USEC * delay);
        if (!running) {
            fprintf(stderr, "\b");
            fprintf(stdout, "STOP!\n\n");
            fprintf(stdout, "Message(s)=%" PRIu64 "\n", frames);
            fprintf(stdout, "Error(s)=%" PRIu64 "\n", errors);
            fprintf(stdout, "Call(s)=%" PRIu64 "\n", calls);
            fprintf(stdout, "Time=%lisec\n\n", time(NULL) - start);
            return frames;
        }
    }
    fprintf(stderr, "\b");
    fprintf(stdout, "OK!\n\n");
    fprintf(stdout, "Message(s)=%" PRIu64 "\n", frames);
    fprintf(stdout, "Error(s)=%" PRIu64 "\n", errors);
    fprintf(stdout, "Call(s)=%" PRIu64 "\n", calls);
    fprintf(stdout, "Time=%lisec\n\n", time(NULL) - start);

    CTimer::Delay(1U * CTimer::SEC);  /* afterburner */
    return frames;}

uint64_t CCanDevice::ReceiverTest(bool checkCounter, uint64_t expectedNumber, bool stopOnError) {
    CANAPI_Message_t message;
    CANAPI_Status_t status;
    CANAPI_Return_t retVal;

    time_t start = time(NULL);
    uint64_t frames = 0U;
    uint64_t errors = 0U;
    uint64_t calls = 0U;
    uint64_t data;

    fprintf(stderr, "\nPress ^C to abort.\n");
    fprintf(stdout, "\nReceiving message(s)...");
    fflush (stdout);
    for (;;) {
        retVal = ReadMessage(message);
        if (retVal == CCanApi::NoError) {
            fprintf(stderr, "%s", prompt[(frames++ % 4)]);
            // checking PCBUSB issue #198 (aka. MACCAN-2)
            if (checkCounter) {
                data = 0;
                if (message.dlc > 0)
                    data |= (uint64_t)message.data[0] << 0;
                if (message.dlc > 1)
                    data |= (uint64_t)message.data[1] << 8;
                if (message.dlc > 2)
                    data |= (uint64_t)message.data[2] << 16;
                if (message.dlc > 3)
                    data |= (uint64_t)message.data[3] << 24;
                if (message.dlc > 4)
                    data |= (uint64_t)message.data[4] << 32;
                if (message.dlc > 5)
                    data |= (uint64_t)message.data[5] << 40;
                if (message.dlc > 6)
                    data |= (uint64_t)message.data[6] << 48;
                if (message.dlc > 7)
                    data |= (uint64_t)message.data[7] << 56;
                if (data != expectedNumber) {
                    fprintf(stderr, "\b");
                    fprintf(stdout, "ISSUE#198!\n");
                    fprintf(stderr, "+++ data inconsistent: %" PRIu64 " received / %" PRIu64 " expected\n", data, expectedNumber);
                    retVal = GetStatus(status);
                    if ((retVal == CCanApi::NoError) && ((status.byte & ~CANSTAT_RESET) != 0x00U)) {
                        fprintf(stderr, "    status register:%s%s%s%s%s%s (%02X)\n",
                            (status.bus_off) ? " BO" : "",
                            (status.warning_level) ? " WL" : "",
                            (status.bus_error) ? " BE" : "",
                            (status.transmitter_busy) ? " TP" : "",
                            (status.message_lost) ? " ML" : "",
                            (status.queue_overrun) ? " QUE" : "", status.byte);
                    }
                    if (stopOnError) {
                        fprintf(stdout, "Message(s)=%" PRIu64 "\n", frames);
                        fprintf(stdout, "Error(s)=%" PRIu64 "\n", errors);
                        fprintf(stdout, "Call(s)=%" PRIu64 "\n", calls);
                        fprintf(stdout, "Time=%lisec\n\n", time(NULL) - start);
                        return frames;
                    }
                    else {
                        fprintf(stderr, "Receiving message(s)... ");
                        expectedNumber = data;
                    }
                }
                expectedNumber++;  // depending on DLC received data may wrap around while number is counting up!
            }
        } else if (retVal != CCanApi::ReceiverEmpty)
            errors++;
        calls++;
        if (!running) {
            fprintf(stderr, "\b");
            fprintf(stdout, "OK!\n\n");
            fprintf(stdout, "Message(s)=%" PRIu64 "\n", frames);
            fprintf(stdout, "Error(s)=%" PRIu64 "\n", errors);
            fprintf(stdout, "Call(s)=%" PRIu64 "\n", calls);
            fprintf(stdout, "Time=%lisec\n\n", time(NULL) - start);
            return frames;
        }
    }
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
    fprintf(stream, "Options for receiver test (default test mode):\n");
    fprintf(stream, " -r, --receive                        count received messages until ^C is pressed\n");
    fprintf(stream, " -n, --number=<number>                check up-counting numbers starting with <number>\n");
    fprintf(stream, " -s, --stop                           stop on error (with option --number)\n");
#if (CAN_FD_SUPPORTED != 0)
    fprintf(stream, " -m, --mode=(2.0|FDF[+BRS])           CAN operation mode: CAN 2.0 or CAN FD mode\n");
#else
    fprintf(stream, " -m, --mode=2.0                       CAN operation mode: CAN 2.0\n");
#endif
    fprintf(stream, "     --shared                         shared CAN controller access (if supported)\n");
    fprintf(stream, "     --listen-only                    monitor mode (listen-only, transmitter is off)\n");
    fprintf(stream, "     --error-frames                   allow reception of error frames\n");
    fprintf(stream, "     --no-remote-frames               suppress remote frames (RTR frames)\n");
    fprintf(stream, "     --no-extended-frames             suppress extended frames (29-bit identifier)\n");
    fprintf(stream, " -b, --baudrate=<baudrate>            CAN bit-timing in kbps (default=250), or\n");
    fprintf(stream, "     --bitrate=<bit-rate>             CAN bit-rate settings (as a string)\n");
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
#if (CAN_FD_SUPPORTED != 0)
    fprintf(stream, " -m, --mode=(2.0|FDF[+BRS])           CAN operation mode: CAN 2.0 or CAN FD mode\n");
#else
    fprintf(stream, " -m, --mode=2.0                       CAN operation mode: CAN 2.0\n");
#endif
    fprintf(stream, "     --shared                         shared CAN controller access (if supported)\n");
    fprintf(stream, " -b, --baudrate=<baudrate>            CAN bit-timing in kbps (default=250), or\n");
    fprintf(stream, "     --bitrate=<bit-rate>             CAN bit-rate settings (as a string)\n");
    fprintf(stream, " -v, --verbose                        show detailed bit-rate settings\n");
    fprintf(stream, "Other options:\n");
#if (CAN_FD_SUPPORTED != 0)
    fprintf(stream, "     --list-bitrates[=<mode>]         list standard bit-rate settings\n");
#else
    fprintf(stream, "     --list-bitrates[=2.0]            list standard bit-rate settings\n");
#endif
#if (OPTION_CANAPI_LIBRARY != 0)
    fprintf(stream, " -L, --list-boards[=<vendor>]         list all supported CAN interfaces and exit\n");
    fprintf(stream, " -T, --test-boards[=<vendor>]         list all available CAN interfaces and exit\n");
#elif (SERIAL_CAN_SUPPORTED == 0)
    fprintf(stream, " -L, --list-boards                    list all supported CAN interfaces and exit\n");
    fprintf(stream, " -T, --test-boards                    list all available CAN interfaces and exit\n");
#endif
    fprintf(stream, " -h, --help                           display this help screen and exit\n");
    fprintf(stream, "     --version                        show version information and exit\n");
#if (0)
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
    fprintf(stream, "Written by Uwe Vogt, UV Software, Berlin <https://www.mac-can.net/>\n");
}
