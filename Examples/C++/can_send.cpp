#ifdef _MSC_VER
//no Microsoft extensions please!
#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS 1
#endif
#endif
#include <iostream>

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
#define OPTION_CANAPI_DRIVER  1
#include "can_api.h"

#include "CANAPI_SerialCAN.h"

#ifndef BAUDRATE
#define BAUDRATE  CANBTR_INDEX_250K
#endif
#define FRAMES  (CAN_MAX_STD_ID+1)

int main(int argc, const char * argv[]) {
    int handle, result;
    can_bitrate_t bitrate;
    can_message_t message;

    can_sio_param_t port;
    port.name = (char*)SERIAL_PORT;
    port.attr.options = CANSIO_SLCAN;
    port.attr.baudrate = 115200U;
    port.attr.bytesize = CANSIO_5DATABITS;
    port.attr.parity = CANSIO_NOPARITY;
    port.attr.stopbits = CANSIO_1STOPBIT;

    std::cout << can_version() << std::endl;
    if ((handle = can_init(CAN_BOARD(CANLIB_SERIALCAN, CANDEV_SERIAL), CANMODE_DEFAULT, (const void*)&port)) < 0) {
        std::cerr << "+++ error: interface could not be initialized" << std::endl;
        return -1;
    }
    bitrate.index = BAUDRATE;
    if ((result = can_start(handle, &bitrate)) < 0) {
        std::cerr << "+++ error: interface could not be started" << std::endl;
        goto end;
    }
    message.xtd = message.rtr = message.sts = 0;
    for (uint64_t i = 0; i < FRAMES; i++) {
        message.id = (uint32_t)i & CAN_MAX_STD_ID;
        message.dlc = 8U;
        message.data[0] = (uint8_t)((uint64_t)i >> 0);
        message.data[1] = (uint8_t)((uint64_t)i >> 8);
        message.data[2] = (uint8_t)((uint64_t)i >> 16);
        message.data[3] = (uint8_t)((uint64_t)i >> 24);
        message.data[4] = (uint8_t)((uint64_t)i >> 32);
        message.data[5] = (uint8_t)((uint64_t)i >> 40);
        message.data[6] = (uint8_t)((uint64_t)i >> 48);
        message.data[7] = (uint8_t)((uint64_t)i >> 56);
        if ((result = can_write(handle, &message, 0U)) < 0) {
            std::cerr << "+++ error: interface could not be stopped" << std::endl;
            goto reset;;
        }
    }
    usleep(1000000);  // afterburner
reset:
    if ((result = can_reset(handle)) < 0)
        std::cerr << "+++ error: interface could not be stopped" << std::endl;
end:
    if ((result = can_exit(handle)) < 0)
        std::cerr << "+++ error: interface could not be shutdown" << std::endl;
    return result;
}
