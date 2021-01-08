#ifdef _MSC_VER
//no Microsoft extensions please!
#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS 1
#endif
#endif
#include <iostream>
#include <signal.h>
#include <errno.h>

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

#include "SerialCAN.h"

#ifndef BAUDRATE
#define BAUDRATE  CANBTR_INDEX_250K
#endif

static void sigterm(int signo);
static volatile int running = 1;

static CSerialCAN mySerialCAN = CSerialCAN();

int main(int argc, const char * argv[]) {
    CANAPI_OpMode_t opMode = {};
    opMode.byte = CANMODE_DEFAULT;
    CANAPI_Bitrate_t bitrate = {};
    bitrate.index = BAUDRATE;
    CANAPI_Message_t message;
    CANAPI_Return_t retVal = 0;
    int frames = 0;

    std::cout << CSerialCAN::GetVersion() << std::endl;
    if((signal(SIGINT, sigterm) == SIG_ERR) ||
#if !defined(_WIN32) && !defined(_WIN64)
       (signal(SIGHUP, sigterm) == SIG_ERR) ||
#endif
       (signal(SIGTERM, sigterm) == SIG_ERR)) {
        perror("+++ error");
        return errno;
    }
    if ((retVal = mySerialCAN.InitializeChannel(SERIAL_PORT, opMode)) != CCANAPI::NoError) {
        std::cerr << "+++ error: interface could not be initialized" << std::endl;
        return retVal;
    }
    if ((retVal = mySerialCAN.StartController(bitrate)) != CCANAPI::NoError) {
        std::cerr << "+++ error: interface could not be started" << std::endl;
        goto teardown;
    }
    std::cout << "Press Ctrl+C to abort..." << std::endl;
    while (running) {
        if ((retVal = mySerialCAN.ReadMessage(message, CANREAD_INFINITE)) == CCANAPI::NoError) {
            fprintf(stdout, "%i\t%7li.%04li\t%03X\t%c%c [%i]", frames++,
                             message.timestamp.tv_sec, message.timestamp.tv_nsec / 100000,
                             message.id, message.xtd? 'X' : 'S', message.rtr? 'R' : ' ', message.dlc);
            for (int i = 0; i < message.dlc; i++)
                fprintf(stdout, " %02X", message.data[i]);
            if (message.sts)
                fprintf(stdout, " <<< status frame");
            fprintf(stdout, "\n");
        }
        else if (retVal != CCANAPI::ReceiverEmpty) {
            fprintf(stderr, "+++ error: read message returned %i", retVal);
            running = 0;
        }
    }
    std::cout << std::endl;
teardown:
    if ((retVal = mySerialCAN.TeardownChannel()) != CCANAPI::NoError)
        std::cerr << "+++ error: interface could not be shutdown" << std::endl;
    return retVal;
}

static void sigterm(int signo) {
     //fprintf(stderr, "%s: got signal %d\n", __FILE__, signo);
     (void)mySerialCAN.SignalChannel();
     running = 0;
     (void)signo;
}
