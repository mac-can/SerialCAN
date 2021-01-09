//
//  main.cpp
//  SerialCAN
//  Bart Simpson
//
#ifdef _MSC_VER
//no Microsoft extensions please!
#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS 1
#endif
#endif
#include "SerialCAN.h"
#include "debug.h"

#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <errno.h>
#include <inttypes.h>
#if !defined(_WIN32) && !defined(_WIN64)
#include <unistd.h>
#if defined(__APPLE__)
#define SERIAL_PORT  "/dev/tty.usbserial-LW4KOZQW"
#elif !defined(__CYGWIN__)
#define SERIAL_PORT  "/dev/ttyUSB0"
#else
#define SERIAL_PORT  "/dev/ttyS3"
#endif
#else
#include <Windows.h>
#define SERIAL_PORT  "\\\\.\\COM4"
int usleep(unsigned int usec);
#endif
#define LOG_FILE  "./slcan.log"

#define OPTION_NO   (0)
#define OPTION_YES  (1)

#define OPTION_TIME_DRIVER  (0)
#define OPTION_TIME_ZERO    (1)
#define OPTION_TIME_ABS     (2)
#define OPTION_TIME_REL     (3)

static void sigterm(int signo);

static void verbose(const can_mode_t mode, const can_bitrate_t bitrate, const can_speed_t speed);

static volatile int running = 1;

static CSerialCAN mySerialCAN = CSerialCAN();

int main(int argc, const char * argv[]) {
    CANAPI_OpMode_t opMode = {};
    opMode.byte = CANMODE_DEFAULT;
    CANAPI_Status_t status = {};
    status.byte = CANSTAT_RESET;
    CANAPI_Bitrate_t bitrate = {};
    bitrate.index = CANBTR_INDEX_250K;
    CANAPI_Message_t message = {};
    message.id = 0x55AU;
    message.xtd = 0;
    message.rtr = 0;
    message.dlc = CAN_MAX_DLC;
    message.data[0] = 0x11;
    message.data[1] = 0x22;
    message.data[2] = 0x33;
    message.data[3] = 0x44;
    message.data[4] = 0x55;
    message.data[5] = 0x66;
    message.data[6] = 0x77;
    message.data[7] = 0x88;
    message.timestamp.tv_sec = 0;
    message.timestamp.tv_nsec = 0;
    CANAPI_Return_t retVal = 0;
    int32_t channel = 0;
    uint16_t timeout = CANREAD_INFINITE;
    unsigned int delay = 0U;
    CSerialCAN::EChannelState state;
    char szVal[CANPROP_MAX_BUFFER_SIZE];
    can_sio_param_t param;
    uint16_t u16Val;
    uint32_t u32Val;
    uint8_t u8Val;
    int32_t i32Val;
    int frames = 0;
    int option_log = OPTION_NO;
    int option_info = OPTION_NO;
    int option_test = OPTION_NO;
    int option_stop = OPTION_NO;
    int option_echo = OPTION_YES;
    int option_repeat = OPTION_NO;
    int option_transmit = OPTION_NO;

    for (int i = 1, opt = 0; i < argc; i++) {
        /* serial port number */
        if(!strcmp(argv[i], "COM1") || !strcmp(argv[i], "CH:0")) channel = 0;
        if(!strcmp(argv[i], "COM2") || !strcmp(argv[i], "CH:1")) channel = 1;
        if(!strcmp(argv[i], "COM3") || !strcmp(argv[i], "CH:2")) channel = 2;
        if(!strcmp(argv[i], "COM4") || !strcmp(argv[i], "CH:3")) channel = 3;
        if(!strcmp(argv[i], "COM5") || !strcmp(argv[i], "CH:4")) channel = 4;
        if(!strcmp(argv[i], "COM6") || !strcmp(argv[i], "CH:5")) channel = 5;
        if(!strcmp(argv[i], "COM7") || !strcmp(argv[i], "CH:6")) channel = 6;
        if(!strcmp(argv[i], "COM8") || !strcmp(argv[i], "CH:7")) channel = 7;
        /* baud rate (CAN 2.0) */
        if (!strcmp(argv[i], "BD:0") || !strcmp(argv[i], "BD:1000")) bitrate.index = CANBTR_INDEX_1M;
        if (!strcmp(argv[i], "BD:1") || !strcmp(argv[i], "BD:800")) bitrate.index = CANBTR_INDEX_800K;
        if (!strcmp(argv[i], "BD:2") || !strcmp(argv[i], "BD:500")) bitrate.index = CANBTR_INDEX_500K;
        if (!strcmp(argv[i], "BD:3") || !strcmp(argv[i], "BD:250")) bitrate.index = CANBTR_INDEX_250K;
        if (!strcmp(argv[i], "BD:4") || !strcmp(argv[i], "BD:125")) bitrate.index = CANBTR_INDEX_125K;
        if (!strcmp(argv[i], "BD:5") || !strcmp(argv[i], "BD:100")) bitrate.index = CANBTR_INDEX_100K;
        if (!strcmp(argv[i], "BD:6") || !strcmp(argv[i], "BD:50")) bitrate.index = CANBTR_INDEX_50K;
        if (!strcmp(argv[i], "BD:7") || !strcmp(argv[i], "BD:20")) bitrate.index = CANBTR_INDEX_20K;
        if (!strcmp(argv[i], "BD:8") || !strcmp(argv[i], "BD:10")) bitrate.index = CANBTR_INDEX_10K;
        /* asynchronous IO */
        if (!strcmp(argv[i], "POLLING")) timeout = 0U;
        if (!strcmp(argv[i], "BLOCKING")) timeout = CANREAD_INFINITE;
        /* transmit messages */
        if ((sscanf(argv[i], "%i", &opt) == 1) && (opt > 0)) option_transmit = opt;
        if (!strncmp(argv[i], "C:", 2) && sscanf(argv[i], "C:%i", &opt) == 1) delay = (unsigned int)opt * 1000U;
        if (!strncmp(argv[i], "U:", 2) && sscanf(argv[i], "U:%i", &opt) == 1) delay = (unsigned int)opt;
        /* receive messages */
        if (!strcmp(argv[i], "STOP")) option_stop = OPTION_YES;
//        if (!strcmp(argv[i], "IGNORE")) option_check = OPTION_NO;
        if (!strcmp(argv[i], "REPEAT")) option_repeat = OPTION_YES;
        if (!strcmp(argv[i], "SILENT")) option_echo = OPTION_NO;
        /* time-stamps */
//        if (!strcmp(argv[i], "ZERO")) option_time = OPTION_TIME_ZERO;
//        if (!strcmp(argv[i], "ABS") || !strcmp(argv[i], "ABSOLUTE")) option_time = OPTION_TIME_ABS;
//        if (!strcmp(argv[i], "REL") || !strcmp(argv[i], "RELATIVE")) option_time = OPTION_TIME_REL;
        /* logging and debugging */
//        if (!strcmp(argv[i], "TRACE")) option_trace = OPTION_YES;
        if (!strcmp(argv[i], "LOG")) option_log = OPTION_YES;
        /* query some informations: hw, sw, etc. */
        if (!strcmp(argv[i], "INFO")) option_info = OPTION_YES;
        if (!strcmp(argv[i], "TEST")) option_test = OPTION_YES;
        /* additional operation modes */
        if (!strcmp(argv[i], "SHARED")) opMode.shrd = 1;
        if (!strcmp(argv[i], "MONITOR")) opMode.mon = 1;
        if (!strcmp(argv[i], "MON:ON")) opMode.mon = 1;
        if (!strcmp(argv[i], "ERR:ON")) opMode.err = 1;
        if (!strcmp(argv[i], "XTD:OFF")) opMode.nxtd = 1;
        if (!strcmp(argv[i], "RTR:OFF")) opMode.nrtr = 1;
    }
    (void)channel;
    (void)option_stop;
    fprintf(stdout, ">>> %s\n", CSerialCAN::GetVersion());

    if((signal(SIGINT, sigterm) == SIG_ERR) ||
#if !defined(_WIN32) && !defined(_WIN64)
       (signal(SIGHUP, sigterm) == SIG_ERR) ||
#endif
       (signal(SIGTERM, sigterm) == SIG_ERR)) {
        perror("+++ error");
        return errno;
    }
    if (option_log)
        LOGGER_INIT(LOG_FILE);
    if (option_info) {
        retVal = mySerialCAN.GetProperty(SERIALCAN_PROPERTY_CANAPI, (void *)&u16Val, sizeof(uint16_t));
        if (retVal == CSerialCAN::NoError)
            fprintf(stdout, ">>> mySerialCAN.GetProperty(SERIALCAN_PROPERTY_CANAPI): value = %u.%u\n", (uint8_t)(u16Val >> 8), (uint8_t)u16Val);
        else
            fprintf(stderr, "+++ error: mySerialCAN.GetProperty(SERIALCAN_PROPERTY_CANAPI) returned %i\n", retVal);
        retVal = mySerialCAN.GetProperty(SERIALCAN_PROPERTY_VERSION, (void *)&u16Val, sizeof(uint16_t));
        if (retVal == CSerialCAN::NoError)
            fprintf(stdout, ">>> mySerialCAN.GetProperty(SERIALCAN_PROPERTY_VERSION): value = %u.%u\n", (uint8_t)(u16Val >> 8), (uint8_t)u16Val);
        else
            fprintf(stderr, "+++ error: mySerialCAN.GetProperty(SERIALCAN_PROPERTY_VERSION) returned %i\n", retVal);
        retVal = mySerialCAN.GetProperty(SERIALCAN_PROPERTY_PATCH_NO, (void *)&u8Val, sizeof(uint8_t));
        if (retVal == CSerialCAN::NoError)
            fprintf(stdout, ">>> mySerialCAN.GetProperty(SERIALCAN_PROPERTY_PATCH_NO): value = %u\n", (uint8_t)u8Val);
        else
            fprintf(stderr, "+++ error: mySerialCAN.GetProperty(SERIALCAN_PROPERTY_PATCH_NO) returned %i\n", retVal);
        retVal = mySerialCAN.GetProperty(SERIALCAN_PROPERTY_BUILD_NO, (void *)&u32Val, sizeof(uint32_t));
        if (retVal == CSerialCAN::NoError)
            fprintf(stdout, ">>> mySerialCAN.GetProperty(SERIALCAN_PROPERTY_BUILD_NO): value = %" PRIx32 "\n", (uint32_t)u32Val);
        else
            fprintf(stderr, "+++ error: mySerialCAN.GetProperty(SERIALCAN_PROPERTY_BUILD_NO) returned %i\n", retVal);
        retVal = mySerialCAN.GetProperty(SERIALCAN_PROPERTY_LIBRARY_ID, (void *)&i32Val, sizeof(int32_t));
        if (retVal == CSerialCAN::NoError)
            fprintf(stdout, ">>> mySerialCAN.GetProperty(SERIALCAN_PROPERTY_LIBRARY_ID): value = %d\n", i32Val);
        else
            fprintf(stderr, "+++ error: mySerialCAN.GetProperty(SERIALCAN_PROPERTY_LIBRARY_ID) returned %i\n", retVal);
        retVal = mySerialCAN.GetProperty(SERIALCAN_PROPERTY_LIBRARY_NAME, (void *)szVal, CANPROP_MAX_BUFFER_SIZE);
        if (retVal == CSerialCAN::NoError)
            fprintf(stdout, ">>> mySerialCAN.GetProperty(SERIALCAN_PROPERTY_LIBRARY_NAME): value = '%s'\n", szVal);
        else
            fprintf(stderr, "+++ error: mySerialCAN.GetProperty(SERIALCAN_PROPERTY_LIBRARY_NAME) returned %i\n", retVal);
        retVal = mySerialCAN.GetProperty(SERIALCAN_PROPERTY_LIBRARY_VENDOR, (void *)szVal, CANPROP_MAX_BUFFER_SIZE);
        if (retVal == CSerialCAN::NoError)
            fprintf(stdout, ">>> mySerialCAN.GetProperty(SERIALCAN_PROPERTY_LIBRARY_VENDOR): value = '%s'\n", szVal);
        else
            fprintf(stderr, "+++ error: mySerialCAN.GetProperty(SERIALCAN_PROPERTY_LIBRARY_VENDOR) returned %i\n", retVal);
    }
    if (option_test) {
        for (int32_t ch = 0; ch < 8; ch++) {
            retVal = CSerialCAN::ProbeChannel(ch, opMode, state);
            fprintf(stdout, ">>> mySerialCAN.ProbeChannel(%i): state = %s", ch,
                            (state == CSerialCAN::ChannelOccupied) ? "occupied" :
                            (state == CSerialCAN::ChannelAvailable) ? "available" :
                            (state == CSerialCAN::ChannelNotAvailable) ? "not available" : "not testable");
            fprintf(stdout, "%s", (retVal == CSerialCAN::IllegalParameter) ? " (waring: Op.-Mode not supported)\n" : "\n");
        }
    }
    retVal = mySerialCAN.InitializeChannel(SERIAL_PORT, opMode);
    if (retVal != CSerialCAN::NoError) {
        fprintf(stderr, "+++ error: mySerialCAN.InitializeChannel(%s) returned %i\n", SERIAL_PORT, retVal);
        goto end;
    }
    else if (mySerialCAN.GetStatus(status) == CSerialCAN::NoError) {
        fprintf(stdout, ">>> mySerialCAN.InitializeChannel(%s): status = 0x%02X\n", SERIAL_PORT, status.byte);
    }
    if (option_test) {
        retVal = mySerialCAN.ProbeChannel(SERIAL_PORT, opMode, state);
        fprintf(stdout, ">>> mySerialCAN.ProbeChannel(%s): state = %s", SERIAL_PORT,
                        (state == CSerialCAN::ChannelOccupied) ? "now occupied" :
                        (state == CSerialCAN::ChannelAvailable) ? "available" :
                        (state == CSerialCAN::ChannelNotAvailable) ? "not available" : "not testable");
        fprintf(stdout, "%s", (retVal == CSerialCAN::IllegalParameter) ? " (waring: Op.-Mode not supported)\n" : "\n");
    }
    if (option_info) {
        retVal = mySerialCAN.GetProperty(SERIALCAN_PROPERTY_DEVICE_TYPE, (void *)&i32Val, sizeof(int32_t));
        if (retVal == CSerialCAN::NoError)
            fprintf(stdout, ">>> mySerialCAN.GetProperty(SERIALCAN_PROPERTY_DEVICE_TYPE): value = %d\n", i32Val);
        else
            fprintf(stderr, "+++ error: mySerialCAN.GetProperty(SERIALCAN_PROPERTY_DEVICE_TYPE) returned %i\n", retVal);
        retVal = mySerialCAN.GetProperty(SERIALCAN_PROPERTY_DEVICE_NAME, (void *)szVal, CANPROP_MAX_BUFFER_SIZE);
        if (retVal == CSerialCAN::NoError)
            fprintf(stdout, ">>> mySerialCAN.GetProperty(SERIALCAN_PROPERTY_DEVICE_NAME): value = '%s'\n", szVal);
        else
            fprintf(stderr, "+++ error: mySerialCAN.GetProperty(SERIALCAN_PROPERTY_DEVICE_NAME) returned %i\n", retVal);
        retVal = mySerialCAN.GetProperty(SERIALCAN_PROPERTY_DEVICE_VENDOR, (void *)szVal, CANPROP_MAX_BUFFER_SIZE);
        if (retVal == CSerialCAN::NoError)
            fprintf(stdout, ">>> mySerialCAN.GetProperty(SERIALCAN_PROPERTY_DEVICE_VENDOR): value = '%s'\n", szVal);
        else
            fprintf(stderr, "+++ error: mySerialCAN.GetProperty(SERIALCAN_PROPERTY_DEVICE_VENDOR) returned %i\n", retVal);
        retVal = mySerialCAN.GetProperty(SERIALCAN_PROPERTY_DEVICE_DLLNAME, (void *)szVal, CANPROP_MAX_BUFFER_SIZE);
        if (retVal == CSerialCAN::NoError)
            fprintf(stdout, ">>> mySerialCAN.GetProperty(SERIALCAN_PROPERTY_DEVICE_DLLNAME): value = '%s'\n", szVal);
        else
            fprintf(stderr, "+++ error: mySerialCAN.GetProperty(SERIALCAN_PROPERTY_DEVICE_DLLNAME) returned %i\n", retVal);
        retVal = mySerialCAN.GetProperty(SERIALCAN_PROPERTY_DEVICE_PARAM, (void *)&param, sizeof(can_sio_param_t));
        if (retVal == CSerialCAN::NoError)
            fprintf(stdout, ">>> mySerialCAN.GetProperty(SERIALCAN_PROPERTY_DEVICE_PARAM): value = '%s:%u,%u-%c-%u'\n", param.name,
                    param.attr.baudrate, param.attr.bytesize, param.attr.parity == 0 ? 'N' : 'X', param.attr.stopbits);
        else
            fprintf(stderr, "+++ error: mySerialCAN.GetProperty(SERIALCAN_PROPERTY_DEVICE_PARAM) returned %i\n", retVal);
        retVal = mySerialCAN.GetProperty(SERIALCAN_PROPERTY_SERIAL_NUMBER, (void *)&u32Val, sizeof(uint32_t));
        if (retVal == CSerialCAN::NoError)
            fprintf(stdout, ">>> mySerialCAN.GetProperty(SERIALCAN_PROPERTY_SERIAL_NUMBER): value = '%X'\n", u32Val);
        else
            fprintf(stderr, "+++ error: mySerialCAN.GetProperty(SERIALCAN_PROPERTY_SERIAL_NUMBER) returned %i\n", retVal);
        retVal = mySerialCAN.GetProperty(SERIALCAN_PROPERTY_CLOCK_DOMAIN, (void *)&i32Val, sizeof(int32_t));
        if (retVal == CSerialCAN::NoError)
            fprintf(stdout, ">>> mySerialCAN.GetProperty(SERIALCAN_PROPERTY_CLOCK_DOMAIN): value = %d\n", i32Val);
        else
            fprintf(stderr, "+++ error: mySerialCAN.GetProperty(SERIALCAN_PROPERTY_CLOCK_DOMAIN) returned %i\n", retVal);
        retVal = mySerialCAN.GetProperty(SERIALCAN_PROPERTY_OP_CAPABILITY, (void *)&u8Val, sizeof(uint8_t));
        if (retVal == CSerialCAN::NoError)
            fprintf(stdout, ">>> mySerialCAN.GetProperty(SERIALCAN_PROPERTY_OP_CAPABILITY): value = 0x%02X\n", (uint8_t)u8Val);
        else
            fprintf(stderr, "+++ error: mySerialCAN.GetProperty(SERIALCAN_PROPERTY_OP_CAPABILITY) returned %i\n", retVal);
        if (mySerialCAN.GetProperty(SERIALCAN_PROPERTY_OP_MODE, (void *)&opMode.byte, sizeof(uint8_t)) == CSerialCAN::NoError)
            fprintf(stdout, ">>> Op.-Mode: 0x%02X\n", (uint8_t)opMode.byte);
    }
    retVal = mySerialCAN.StartController(bitrate);
    if (retVal != CSerialCAN::NoError) {
        fprintf(stderr, "+++ error: mySerialCAN.StartController returned %i\n", retVal);
        goto teardown;
    }
    else if (mySerialCAN.GetStatus(status) == CSerialCAN::NoError) {
        fprintf(stdout, ">>> mySerialCAN.StartController: status = 0x%02X\n", status.byte);
    }
    if (option_info) {
        CANAPI_BusSpeed_t speed;
        if ((mySerialCAN.GetBitrate(bitrate) == CSerialCAN::NoError) &&
            (mySerialCAN.GetBusSpeed(speed) == CSerialCAN::NoError))
            verbose(opMode, bitrate, speed);
    }
    fprintf(stdout, "Press Ctrl+C to abort...\n");
    while (running && (option_transmit-- > 0)) {
        retVal = mySerialCAN.WriteMessage(message);
        if (retVal != CSerialCAN::NoError) {
            fprintf(stderr, "+++ error: mySerialCAN.WriteMessage returned %i\n", retVal);
            goto teardown;
        }
        if (delay)
            usleep(delay);
    }
    while (running) {
        if ((retVal = mySerialCAN.ReadMessage(message, timeout)) == CSerialCAN::NoError) {
            if (option_echo) {
                fprintf(stdout, "%i\t%7li.%04li\t%03x\t%c%c [%i]", frames++,
                                 (long)message.timestamp.tv_sec, message.timestamp.tv_nsec / 100000,
                                 message.id, message.xtd? 'X' : 'S', message.rtr? 'R' : ' ', message.dlc);
                if (!message.rtr)  // normal frame
                    for (int i = 0; i < message.dlc; i++)
                        fprintf(stdout, " %02x", message.data[i]);
                if (message.sts)   // status frame
                    fprintf(stdout, " <<< status frame");
                else if (option_repeat) {
                    retVal = mySerialCAN.WriteMessage(message);
                    if (retVal != CSerialCAN::NoError) {
                        fprintf(stderr, "+++ error: mySerialCAN.WriteMessage returned %i\n", retVal);
                        goto teardown;
                    }
                }
                fprintf(stdout, "\n");
            } else {
                if(!(frames++ % 2048)) {
                    fprintf(stdout, ".");
                    fflush(stdout);
                }
            }
        }
        else if (retVal != CSerialCAN::ReceiverEmpty) {
            running = 0;
        }
    }
    if (mySerialCAN.GetStatus(status) == CSerialCAN::NoError) {
        fprintf(stdout, "\n>>> mySerialCAN.ReadMessage: status = 0x%02X\n", status.byte);
    }
    if (option_info) {
        uint64_t u64TxCnt, u64RxCnt, u64ErrCnt;
        if ((mySerialCAN.GetProperty(SERIALCAN_PROPERTY_TX_COUNTER, (void *)&u64TxCnt, sizeof(uint64_t)) == CSerialCAN::NoError) &&
            (mySerialCAN.GetProperty(SERIALCAN_PROPERTY_RX_COUNTER, (void *)&u64RxCnt, sizeof(uint64_t)) == CSerialCAN::NoError) &&
            (mySerialCAN.GetProperty(SERIALCAN_PROPERTY_ERR_COUNTER, (void *)&u64ErrCnt, sizeof(uint64_t)) == CSerialCAN::NoError))
            fprintf(stdout, ">>> mySerialCAN.GetProperty(SERIALCAN_PROPERTY_*_COUNTER): TX = %" PRIi64 " RX = %" PRIi64 " ERR = %" PRIi64 "\n", u64TxCnt, u64RxCnt, u64ErrCnt);
        char *hardware = mySerialCAN.GetHardwareVersion();
        if (hardware)
            fprintf(stdout, ">>> mySerialCAN.GetHardwareVersion: '%s'\n", hardware);
        char *firmware = mySerialCAN.GetFirmwareVersion();
        if (firmware)
            fprintf(stdout, ">>> mySerialCAN.GetFirmwareVersion: '%s'\n", firmware);
    }
teardown:
    retVal = mySerialCAN.TeardownChannel();
    if (retVal != CSerialCAN::NoError) {
        fprintf(stderr, "+++ error: mySerialCAN.TeardownChannel returned %i\n", retVal);
        goto end;
    }
    else if (mySerialCAN.GetStatus(status) == CSerialCAN::NoError) {
        fprintf(stdout, ">>> mySerialCAN.TeardownChannel: status = 0x%02X\n", status.byte);
    }
    else {
        fprintf(stdout, "@@@ Resistance is futile!\n");
    }
end:
    if (option_log)
        LOGGER_EXIT();
    fprintf(stdout, ">>> Cheers!\n");
    return retVal;
}

static void verbose(const can_mode_t mode, const can_bitrate_t bitrate, const can_speed_t speed)
{
#if (OPTION_CAN_2_0_ONLY == 0)
    fprintf(stdout, "Op.-Mode: 0x%02X (fdoe=%u,brse=%u,niso=%u,shrd=%u,nxtd=%u,nrtr=%u,err=%u,mon=%u)\n",
            mode.byte, mode.fdoe, mode.brse, mode.niso, mode.shrd, mode.nxtd, mode.nrtr, mode.err, mode.mon);
#else
    fprintf(stdout, "Op.-Mode: 0x%02X (shrd=%u,nxtd=%u,nrtr=%u,err=%u,mon=%u)\n",
            mode.byte, mode.shrd, mode.nxtd, mode.nrtr, mode.err, mode.mon);
#endif
   if (bitrate.btr.frequency > 0) {
        fprintf(stdout, "Baudrate: %.0fkbps@%.1f%%",
           speed.nominal.speed / 1000., speed.nominal.samplepoint * 100.);
#if (OPTION_CAN_2_0_ONLY == 0)
        if(/*speed.data.brse*/mode.fdoe && mode.brse)
           fprintf(stdout, ":%.0fkbps@%.1f%%",
               speed.data.speed / 1000., speed.data.samplepoint * 100.);
#endif
       fprintf(stdout, " (f_clock=%i,nom_brp=%u,nom_tseg1=%u,nom_tseg2=%u,nom_sjw=%u,nom_sam=%u",
           bitrate.btr.frequency,
           bitrate.btr.nominal.brp,
           bitrate.btr.nominal.tseg1,
           bitrate.btr.nominal.tseg2,
           bitrate.btr.nominal.sjw,
           bitrate.btr.nominal.sam);
#if (OPTION_CAN_2_0_ONLY == 0)
        if(mode.fdoe && mode.brse)
           fprintf(stdout, ",data_brp=%u,data_tseg1=%u,data_tseg2=%u,data_sjw=%u",
               bitrate.btr.data.brp,
               bitrate.btr.data.tseg1,
               bitrate.btr.data.tseg2,
               bitrate.btr.data.sjw);
#endif
       fprintf(stdout, ")\n");
    }
    else {
        fprintf(stdout, "Baudrate: %skbps (CiA index %i)\n",
           bitrate.index == CANBDR_1000 ? "1000" :
           bitrate.index == -CANBDR_800 ? "800" :
           bitrate.index == -CANBDR_500 ? "500" :
           bitrate.index == -CANBDR_250 ? "250" :
           bitrate.index == -CANBDR_125 ? "125" :
           bitrate.index == -CANBDR_100 ? "100" :
           bitrate.index == -CANBDR_50 ? "50" :
           bitrate.index == -CANBDR_20 ? "20" :
           bitrate.index == -CANBDR_10 ? "10" : "?", -bitrate.index);
   }
}

#if defined(_WIN32) || defined(_WIN64)
int usleep(unsigned int usec)
{
    HANDLE timer;
    LARGE_INTEGER ft;

    ft.QuadPart = -(10 * (LONGLONG)usec); // Convert to 100 nanosecond interval, negative value indicates relative time
    if (usec >= 100) {
        timer = CreateWaitableTimer(NULL, TRUE, NULL);
        SetWaitableTimer(timer, &ft, 0, NULL, NULL, 0);
        WaitForSingleObject(timer, INFINITE);
        CloseHandle(timer);
    }
    return 1;
}
#endif

static void sigterm(int signo) {
     //fprintf(stderr, "%s: got signal %d\n", __FILE__, signo);
     mySerialCAN.SignalChannel();
     running = 0;
     (void)signo;
}
