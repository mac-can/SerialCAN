//
//  main.cpp
//  SerialCAN
//  Bart Simpson didn´t do it
//
#ifdef _MSC_VER
//no Microsoft extensions please!
#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS 1
#endif
#endif
#include "SerialCAN.h"

#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <errno.h>
#include <time.h>
#if !defined(_WIN32) && !defined(_WIN64)
 #include <unistd.h>
#else
 #include <windows.h>
#endif

#include <inttypes.h>
#include "debug.h"

#if !defined(_WIN32) && !defined(_WIN64)
#if defined(__APPLE__)
#define SERIAL_PORT  "/dev/tty.usbserial-LW4KOZQW"
#elif !defined(__CYGWIN__)
#define SERIAL_PORT  "/dev/ttyUSB0"
#else
#define SERIAL_PORT  "/dev/ttyS3"
#endif
#else
#define SERIAL_PORT  "\\\\.\\COM4"
#endif
#define LOG_FILE  "./slcan.log"

#define OPTION_NO   (0)
#define OPTION_YES  (1)

#define OPTION_TIME_DRIVER  (0)
#define OPTION_TIME_ZERO    (1)
#define OPTION_TIME_ABS     (2)
#define OPTION_TIME_REL     (3)

#if defined(_WIN32) || defined(_WIN64)
 static void usleep(unsigned int usec);
#endif
static void sigterm(int signo);

static void verbose(const can_mode_t &mode, const can_bitrate_t &bitrate, const can_speed_t &speed);

static volatile int running = 1;

static CSerialCAN myDriver = CSerialCAN();

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
    //CCanApi::SChannelInfo info;
    CCanApi::EChannelState state;
    //int32_t clocks[CANPROP_MAX_BUFFER_SIZE/sizeof(int32_t)];
    char szVal[CANPROP_MAX_BUFFER_SIZE];
    can_sio_param_t param;
    uint16_t u16Val;
    uint32_t u32Val;
    uint8_t u8Val;
    int32_t i32Val;
    int frames = 0;
    int option_log = OPTION_NO;
    int option_info = OPTION_NO;
    int option_stat = OPTION_NO;
    int option_test = OPTION_NO;
    int option_exit = OPTION_NO;
    int option_echo = OPTION_YES;
    int option_stop = OPTION_NO;
    int option_check = OPTION_NO;
    int option_retry = OPTION_NO;
    int option_repeat = OPTION_NO;
    int option_transmit = OPTION_NO;
    uint64_t received = 0ULL;
    uint64_t expected = 0ULL;

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
        if (!strcmp(argv[i], "CHECK")) option_check = OPTION_YES;
        if (!strcmp(argv[i], "RETRY")) option_retry = OPTION_YES;
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
        if (!strcmp(argv[i], "STAT")) option_stat = OPTION_YES;
        if (!strcmp(argv[i], "TEST")) option_test = OPTION_YES;
        if (!strcmp(argv[i], "EXIT")) option_exit = OPTION_YES;
        /* additional operation modes (bit field) */
        if (!strcmp(argv[i], "SHARED")) opMode.shrd = 1;
        if (!strcmp(argv[i], "MONITOR")) opMode.mon = 1;
        if (!strcmp(argv[i], "MON:ON")) opMode.mon = 1;
        if (!strcmp(argv[i], "ERR:ON")) opMode.err = 1;
        if (!strcmp(argv[i], "XTD:OFF")) opMode.nxtd = 1;
        if (!strcmp(argv[i], "RTR:OFF")) opMode.nrtr = 1;
    }
    fprintf(stdout, ">>> %s\n", CSerialCAN::GetVersion());
    (void)channel;

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
        retVal = myDriver.GetProperty(CANPROP_GET_SPEC, (void *)&u16Val, sizeof(uint16_t));
        if (retVal == CCanApi::NoError)
            fprintf(stdout, ">>> myDriver.GetProperty(CANPROP_GET_SPEC): value = %u.%u\n", (uint8_t)(u16Val >> 8), (uint8_t)u16Val);
        else
            fprintf(stderr, "+++ error: myDriver.GetProperty(CANPROP_GET_SPEC) returned %i\n", retVal);
        retVal = myDriver.GetProperty(CANPROP_GET_VERSION, (void *)&u16Val, sizeof(uint16_t));
        if (retVal == CCanApi::NoError)
            fprintf(stdout, ">>> myDriver.GetProperty(CANPROP_GET_VERSION): value = %u.%u\n", (uint8_t)(u16Val >> 8), (uint8_t)u16Val);
        else
            fprintf(stderr, "+++ error: myDriver.GetProperty(CANPROP_GET_VERSION) returned %i\n", retVal);
        retVal = myDriver.GetProperty(CANPROP_GET_PATCH_NO, (void *)&u8Val, sizeof(uint8_t));
        if (retVal == CCanApi::NoError)
            fprintf(stdout, ">>> myDriver.GetProperty(CANPROP_GET_PATCH_NO): value = %u\n", u8Val);
        else
            fprintf(stderr, "+++ error: myDriver.GetProperty(CANPROP_GET_PATCH_NO) returned %i\n", retVal);
        retVal = myDriver.GetProperty(CANPROP_GET_BUILD_NO, (void *)&u32Val, sizeof(uint32_t));
        if (retVal == CCanApi::NoError)
            fprintf(stdout, ">>> myDriver.GetProperty(CANPROP_GET_BUILD_NO): value = %" PRIx32 "\n", u32Val);
        else
            fprintf(stderr, "+++ error: myDriver.GetProperty(CANPROP_GET_BUILD_NO) returned %i\n", retVal);
        retVal = myDriver.GetProperty(CANPROP_GET_LIBRARY_ID, (void *)&i32Val, sizeof(int32_t));
        if (retVal == CCanApi::NoError)
            fprintf(stdout, ">>> myDriver.GetProperty(CANPROP_GET_LIBRARY_ID): value = %d\n", i32Val);
        else
            fprintf(stderr, "+++ error: myDriver.GetProperty(CANPROP_GET_LIBRARY_ID) returned %i\n", retVal);
        retVal = myDriver.GetProperty(CANPROP_GET_LIBRARY_DLLNAME, (void *)szVal, CANPROP_MAX_BUFFER_SIZE);
        if (retVal == CCanApi::NoError)
            fprintf(stdout, ">>> myDriver.GetProperty(CANPROP_GET_LIBRARY_DLLNAME): value = '%s'\n", szVal);
        else
            fprintf(stderr, "+++ error: myDriver.GetProperty(CANPROP_GET_LIBRARY_DLLNAME) returned %i\n", retVal);
        retVal = myDriver.GetProperty(CANPROP_GET_LIBRARY_VENDOR, (void *)szVal, CANPROP_MAX_BUFFER_SIZE);
        if (retVal == CCanApi::NoError)
            fprintf(stdout, ">>> myDriver.GetProperty(CANPROP_GET_LIBRARY_VENDOR): value = '%s'\n", szVal);
        else
            fprintf(stderr, "+++ error: myDriver.GetProperty(CANPROP_GET_LIBRARY_VENDOR) returned %i\n", retVal);
        if (option_exit && !option_test)
            return 0;
    }
    if (option_test) {
#if (0)
        bool result = CSerialCAN::GetFirstChannel(info);
        while (result) {
            retVal = CSerialCAN::ProbeChannel(info.m_nChannelNo, opMode, state);
            fprintf(stdout, ">>> CCanAPI::ProbeChannel(%i): state = %s", info.m_nChannelNo,
                            (state == CCanApi::ChannelOccupied) ? "occupied" :
                            (state == CCanApi::ChannelAvailable) ? "available" :
                            (state == CCanApi::ChannelNotAvailable) ? "not available" : "not testable");
            fprintf(stdout, "%s", (retVal == CCanApi::IllegalParameter) ? " (warning: Op.-Mode not supported)\n" : "\n");
            result = CSerialCAN::GetNextChannel(info);
        }
#else
        retVal = myDriver.SetProperty(CANPROP_SET_FIRST_CHANNEL, (void *)NULL, 0U);
        while (retVal == CCanApi::NoError) {
            retVal = myDriver.GetProperty(CANPROP_GET_CHANNEL_NO, (void *)&i32Val, sizeof(int32_t));
            if (retVal == CCanApi::NoError) {
                retVal = CSerialCAN::ProbeChannel(i32Val, opMode, state);
                fprintf(stdout, ">>> CCanApi::ProbeChannel(%i): state = %s", i32Val,
                                (state == CCanApi::ChannelOccupied) ? "occupied" :
                                (state == CCanApi::ChannelAvailable) ? "available" :
                                (state == CCanApi::ChannelNotAvailable) ? "not available" : "not testable");
                fprintf(stdout, "%s", (retVal == CCanApi::IllegalParameter) ? " (warning: Op.-Mode not supported)\n" : "\n");
            }
            retVal = myDriver.SetProperty(CANPROP_SET_NEXT_CHANNEL, (void *)NULL, 0U);
        }
#endif
        if (option_exit)
            return 0;
    }
    retVal = myDriver.InitializeChannel(SERIAL_PORT, opMode);
    if (retVal != CCanApi::NoError) {
        fprintf(stderr, "+++ error: myDriver.InitializeChannel(%s) returned %i\n", SERIAL_PORT, retVal);
        goto end;
    }
    else if (myDriver.GetStatus(status) == CCanApi::NoError) {
        fprintf(stdout, ">>> myDriver.InitializeChannel(%s): status = 0x%02X\n", SERIAL_PORT, status.byte);
    }
    if (option_test) {
        retVal = myDriver.ProbeChannel(SERIAL_PORT, opMode, state);
        fprintf(stdout, ">>> myDriver.ProbeChannel(%s): state = %s", SERIAL_PORT,
                        (state == CCanApi::ChannelOccupied) ? "now occupied" :
                        (state == CCanApi::ChannelAvailable) ? "available" :
                        (state == CCanApi::ChannelNotAvailable) ? "not available" : "not testable");
        fprintf(stdout, "%s", (retVal == CCanApi::IllegalParameter) ? " (warning: Op.-Mode not supported)\n" : "\n");
    }
    if (option_info) {
        retVal = myDriver.GetProperty(CANPROP_GET_NUM_CHANNELS, (void*)&u8Val, sizeof(uint8_t));
        if (retVal == CCanApi::NoError)
            fprintf(stdout, ">>> myDriver.GetProperty(CANPROP_GET_NUM_CHANNELS): value = %d\n", u8Val);
        //else [optional property]
        //    fprintf(stderr, "+++ error: myDriver.GetProperty(CANPROP_GET_NUM_CHANNELS) returned %i\n", retVal);
        retVal = myDriver.GetProperty(CANPROP_GET_CAN_CHANNEL, (void*)&u8Val, sizeof(uint8_t));
        if (retVal == CCanApi::NoError)
            fprintf(stdout, ">>> myDriver.GetProperty(CANPROP_GET_CAN_CHANNEL): value = %u\n", u8Val);
        //else [optional property]
        //    fprintf(stderr, "+++ error: myDriver.GetProperty(CANPROP_GET_CAN_CHANNEL) returned %i\n", retVal);
        retVal = myDriver.GetProperty(CANPROP_GET_DEVICE_TYPE, (void *)&i32Val, sizeof(int32_t));
        if (retVal == CCanApi::NoError)
            fprintf(stdout, ">>> myDriver.GetProperty(CANPROP_GET_DEVICE_TYPE): value = %d\n", i32Val);
        else
            fprintf(stderr, "+++ error: myDriver.GetProperty(CANPROP_GET_DEVICE_TYPE) returned %i\n", retVal);
        retVal = myDriver.GetProperty(CANPROP_GET_DEVICE_NAME, (void *)szVal, CANPROP_MAX_BUFFER_SIZE);
        if (retVal == CCanApi::NoError)
            fprintf(stdout, ">>> myDriver.GetProperty(CANPROP_GET_DEVICE_NAME): value = '%s'\n", szVal);
        else
            fprintf(stderr, "+++ error: myDriver.GetProperty(CANPROP_GET_DEVICE_NAME) returned %i\n", retVal);
        retVal = myDriver.GetProperty(CANPROP_GET_DEVICE_VENDOR, (void *)szVal, CANPROP_MAX_BUFFER_SIZE);
        if (retVal == CCanApi::NoError)
            fprintf(stdout, ">>> myDriver.GetProperty(CANPROP_GET_DEVICE_VENDOR): value = '%s'\n", szVal);
        else
            fprintf(stderr, "+++ error: myDriver.GetProperty(CANPROP_GET_DEVICE_VENDOR) returned %i\n", retVal);
        retVal = myDriver.GetProperty(CANPROP_GET_DEVICE_DLLNAME, (void *)szVal, CANPROP_MAX_BUFFER_SIZE);
        if (retVal == CCanApi::NoError)
            fprintf(stdout, ">>> myDriver.GetProperty(CANPROP_GET_DEVICE_DLLNAME): value = '%s'\n", szVal);
        else
            fprintf(stderr, "+++ error: myDriver.GetProperty(CANPROP_GET_DEVICE_DLLNAME) returned %i\n", retVal);
        retVal = myDriver.GetProperty(CANPROP_GET_DEVICE_PARAM, (void *)&param, sizeof(can_sio_param_t));
        if (retVal == CCanApi::NoError)
            fprintf(stdout, ">>> myDriver.GetProperty(CANPROP_GET_DEVICE_PARAM): value = '%s:%u,%u-%c-%u'\n", param.name,
                    param.attr.baudrate, param.attr.bytesize, param.attr.parity == 0 ? 'N' : 'X', param.attr.stopbits);
        else
            fprintf(stderr, "+++ error: myDriver.GetProperty(CANPROP_GET_DEVICE_PARAM) returned %i\n", retVal);
        // vendor-specific properties
        retVal = myDriver.GetProperty(SERIALCAN_PROPERTY_SERIAL_NUMBER, (void *)&u32Val, sizeof(uint32_t));
        if (retVal == CCanApi::NoError)
            fprintf(stdout, ">>> myDriver.GetProperty(SERIALCAN_PROPERTY_SERIAL_NUMBER): value = '%X'\n", u32Val);
        else
            fprintf(stderr, "+++ error: myDriver.GetProperty(SERIALCAN_PROPERTY_SERIAL_NUMBER) returned %i\n", retVal);
        retVal = myDriver.GetProperty(SERIALCAN_PROPERTY_HARDWARE_VERSION, (void*)&u16Val, sizeof(uint16_t));
        if (retVal == CCanApi::NoError)
            fprintf(stdout, ">>> myDriver.GetProperty(SERIALCAN_PROPERTY_HARDWARE_VERSION): value = %u.%u\n", (uint8_t)(u16Val >> 8), (uint8_t)u16Val);
        else
            fprintf(stderr, "+++ error: myDriver.GetProperty(SERIALCAN_PROPERTY_HARDWARE_VERSION) returned %i\n", retVal);
        retVal = myDriver.GetProperty(SERIALCAN_PROPERTY_FIRMWARE_VERSION, (void*)&u16Val, sizeof(uint16_t));
        if (retVal == CCanApi::NoError)
            fprintf(stdout, ">>> myDriver.GetProperty(SERIALCAN_PROPERTY_FIRMWARE_VERSION): value = %u.%u\n", (uint8_t)(u16Val >> 8), (uint8_t)u16Val);
        else
            fprintf(stderr, "+++ error: myDriver.GetProperty(SERIALCAN_PROPERTY_FIRMWARE_VERSION) returned %i\n", retVal);
        retVal = myDriver.GetProperty(SERIALCAN_PROPERTY_CAN_CLOCK_FREQUENCY, (void *)&i32Val, sizeof(int32_t));
        if (retVal == CCanApi::NoError)
            fprintf(stdout, ">>> myDriver.GetProperty(SERIALCAN_PROPERTY_CAN_CLOCK_FREQUENCY): value = %d\n", i32Val);
        else
            fprintf(stderr, "+++ error: myDriver.GetProperty(SERIALCAN_PROPERTY_CAN_CLOCK_FREQUENCY) returned %i\n", retVal);
#if (0)
        retVal = myDriver.GetProperty(CANPROP_GET_CAN_CLOCKS, (void *)clocks, CANPROP_MAX_BUFFER_SIZE);
        if (retVal == CCanApi::NoError) {
            fprintf(stdout, ">>> myDriver.GetProperty(CANPROP_GET_CAN_CLOCKS): array =");
            for (int i = 0; (i < (int)(CANPROP_MAX_BUFFER_SIZE/sizeof(int32_t))) && (clocks[i] != EOF); i++)
                fprintf(stdout, "%s%.1f", i ? ", " : " [", (float)clocks[i] / (float)1000000);
            fprintf(stdout, "] MHz\n");
        } else
            fprintf(stderr, "+++ error: myDriver.GetProperty(CANPROP_GET_CAN_CLOCKS) returned %i\n", retVal);
#endif
        retVal = myDriver.GetProperty(CANPROP_GET_OP_CAPABILITY, (void *)&u8Val, sizeof(uint8_t));
        if (retVal == CCanApi::NoError)
            fprintf(stdout, ">>> myDriver.GetProperty(CANPROP_GET_OP_CAPABILITY): value = 0x%02X\n", u8Val);
        else
            fprintf(stderr, "+++ error: myDriver.GetProperty(CANPROP_GET_OP_CAPABILITY) returned %i\n", retVal);
        if (myDriver.GetProperty(CANPROP_GET_OP_MODE, (void *)&u8Val, sizeof(uint8_t)) == CCanApi::NoError)
            fprintf(stdout, ">>> myDriver.GetProperty(CANPROP_GET_OP_MODE): value = 0x%02X\n", u8Val);
    }
    retVal = myDriver.StartController(bitrate);
    if (retVal != CCanApi::NoError) {
        fprintf(stderr, "+++ error: myDriver.StartController returned %i\n", retVal);
        goto teardown;
    }
    else if (myDriver.GetStatus(status) == CCanApi::NoError) {
        fprintf(stdout, ">>> myDriver.StartController: status = 0x%02X\n", status.byte);
    }
    if (option_info) {
        CANAPI_BusSpeed_t speed;
        if ((myDriver.GetBitrate(bitrate) == CCanApi::NoError) &&
            (myDriver.GetBusSpeed(speed) == CCanApi::NoError))
            verbose(opMode, bitrate, speed);
    }
    fprintf(stdout, "Press Ctrl+C to abort...\n");
    while (running && (option_transmit-- > 0)) {
retry:
        retVal = myDriver.WriteMessage(message);
        if ((retVal == CCanApi::TransmitterBusy) && option_retry)
            goto retry;
        else if (retVal != CCanApi::NoError) {
            fprintf(stderr, "+++ error: myDriver.WriteMessage returned %i\n", retVal);
            goto teardown;
        }
        if (delay)
            usleep(delay);
    }
    while (running) {
        if ((retVal = myDriver.ReadMessage(message, timeout)) == CCanApi::NoError) {
            if (option_echo) {
                fprintf(stdout, "%i\t%7li.%04li\t%03x\t%c%c [%i]", frames++,
                                 (long)message.timestamp.tv_sec, message.timestamp.tv_nsec / 100000,
                                 message.id, message.xtd? 'X' : 'S', message.rtr? 'R' : ' ', message.dlc);
                if (!message.rtr)  // normal frame
                for (uint8_t i = 0; i < CCanApi::Dlc2Len(message.dlc); i++)
                    fprintf(stdout, " %02x", message.data[i]);
                if (message.sts)
                    fprintf(stdout, " <<< status frame");
                else if (option_repeat) {
                    retVal = myDriver.WriteMessage(message);
                    if (retVal != CCanApi::NoError) {
                        fprintf(stderr, "+++ error: myDriver.WriteMessage returned %i\n", retVal);
                        goto teardown;
                    }
                }
                fprintf(stdout, "\n");
            } else {
                if (!(frames++ % 2048)) {
                    fprintf(stdout, ".");
                    fflush(stdout);
                }
            }
            if (option_check && !message.sts) {
                received = 0;
                if (message.dlc > 0) received |= (uint64_t)message.data[0] << 0;
                if (message.dlc > 1) received |= (uint64_t)message.data[1] << 8;
                if (message.dlc > 2) received |= (uint64_t)message.data[2] << 16;
                if (message.dlc > 3) received |= (uint64_t)message.data[3] << 24;
                if (message.dlc > 4) received |= (uint64_t)message.data[4] << 32;
                if (message.dlc > 5) received |= (uint64_t)message.data[5] << 40;
                if (message.dlc > 6) received |= (uint64_t)message.data[6] << 48;
                if (message.dlc > 7) received |= (uint64_t)message.data[7] << 56;
                if (received != expected) {
                    fprintf(stderr, "+++ error: received data is not equal to expected data (%" PRIu64 " : %" PRIu64 ")\n", received, expected);
                    if (expected > received)
                        fprintf(stderr, "           issue #198: old messages read again (offset -%" PRIu64 ")\a\n", expected - received);
                    if (option_stop)
                        goto teardown;
                }
                expected = received + 1;
            }
        }
        else if (retVal != CCanApi::ReceiverEmpty) {
            goto teardown;
        }
    }
    if (myDriver.GetStatus(status) == CCanApi::NoError) {
        fprintf(stdout, "\n>>> myDriver.ReadMessage: status = 0x%02X\n", status.byte);
    }
    if (option_stat || option_info) {
        uint64_t u64TxCnt, u64RxCnt, u64ErrCnt;
        if ((myDriver.GetProperty(CANPROP_GET_TX_COUNTER, (void *)&u64TxCnt, sizeof(uint64_t)) == CCanApi::NoError) &&
            (myDriver.GetProperty(CANPROP_GET_RX_COUNTER, (void *)&u64RxCnt, sizeof(uint64_t)) == CCanApi::NoError) &&
            (myDriver.GetProperty(CANPROP_GET_ERR_COUNTER, (void *)&u64ErrCnt, sizeof(uint64_t)) == CCanApi::NoError))
            fprintf(stdout, ">>> myDriver.GetProperty(CANPROP_GET_*_COUNTER): TX = %" PRIu64 " RX = %" PRIu64 " ERR = %" PRIu64 "\n", u64TxCnt, u64RxCnt, u64ErrCnt);
    }
    if (option_stat) {
        uint32_t u32QueSize, u32QueHigh; uint64_t u64QueOvfl;
        if ((myDriver.GetProperty(CANPROP_GET_RCV_QUEUE_SIZE, (void *)&u32QueSize, sizeof(uint32_t)) == CCanApi::NoError) &&
            (myDriver.GetProperty(CANPROP_GET_RCV_QUEUE_HIGH, (void *)&u32QueHigh, sizeof(uint32_t)) == CCanApi::NoError) &&
            (myDriver.GetProperty(CANPROP_GET_RCV_QUEUE_OVFL, (void *)&u64QueOvfl, sizeof(uint64_t)) == CCanApi::NoError))
            fprintf(stdout, ">>> myDriver.GetProperty(CANPROP_GET_QUEUE_*): SIZE = %" PRIu32 " HIGH = %" PRIu32 " OVFL = %" PRIu64 "\n", u32QueSize, u32QueHigh, u64QueOvfl);
    }
    if (option_info) {
        char *hardware = myDriver.GetHardwareVersion();
        if (hardware)
            fprintf(stdout, ">>> myDriver.GetHardwareVersion: '%s'\n", hardware);
        char *firmware = myDriver.GetFirmwareVersion();
        if (firmware)
            fprintf(stdout, ">>> myDriver.GetFirmwareVersion: '%s'\n", firmware);
    }
teardown:
    retVal = myDriver.TeardownChannel();
    if (retVal != CCanApi::NoError) {
        fprintf(stderr, "+++ error: myDriver.TeardownChannel returned %i\n", retVal);
        goto end;
    }
    else if (myDriver.GetStatus(status) == CCanApi::NoError) {
        fprintf(stdout, ">>> myDriver.TeardownChannel: status = 0x%02X\n", status.byte);
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

static void verbose(const can_mode_t &mode, const can_bitrate_t &bitrate, const can_speed_t &speed)
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
        if (/*speed.data.brse*/mode.fdoe && mode.brse)
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
        if (mode.fdoe && mode.brse)
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
 /* usleep(3) - Linux man page
  *
  * Notes
  * The type useconds_t is an unsigned integer type capable of holding integers in the range [0,1000000].
  * Programs will be more portable if they never mention this type explicitly. Use
  *
  *    #include <unistd.h>
  *    ...
  *        unsigned int usecs;
  *    ...
  *        usleep(usecs);
  */
 static void usleep(unsigned int usec) {
    HANDLE timer;
    LARGE_INTEGER ft;

    ft.QuadPart = -(10 * (LONGLONG)usec); // Convert to 100 nanosecond interval, negative value indicates relative time
    if (usec >= 100) {
        if ((timer = CreateWaitableTimer(NULL, TRUE, NULL)) != NULL) {
            SetWaitableTimer(timer, &ft, 0, NULL, NULL, 0);
            WaitForSingleObject(timer, INFINITE);
            CloseHandle(timer);
        }
    }
    else {
        Sleep(0);
    }
 }
#endif

static void sigterm(int signo) {
    //fprintf(stderr, "%s: got signal %d\n", __FILE__, signo);
    (void)myDriver.SignalChannel();
    running = 0;
    (void)signo;
}
