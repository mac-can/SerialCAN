//
//  main.c
//  SerialCAN
//  Bart Simpson
//
#ifdef _MSC_VER
//no Microsoft extensions please!
#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS 1
#endif
#endif
#include "serial.h"
#include "slcan.h"
#include "debug.h"

#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <errno.h>
#include <inttypes.h>
#if !defined(_WIN32) && !defined(_WIN64)
#include <unistd.h>
#define SERIAL_PORT  "/dev/ttyUSB0"
#else
#include <Windows.h>
int usleep(unsigned int usec);
#define SERIAL_PORT  "\\\\.\\COM4"
#endif
#define LOG_FILE  "./slcan.log"

static void sigterm(int signo);

static volatile int running = 1;

static int main_serial(int argc, char *argv[]) {
    sio_port_t port = NULL;
    uint8_t cmd_open[2] = {'O','\r'};
    uint8_t cmd_close[2] = {'C','\r'};
    int rc = 0;

    port = sio_create(NULL, NULL);
    if (!port) {
        fprintf(stderr, "+++ error: sio_create returnd NULL (%i)\n", errno);
        return -1;
    }
    rc = sio_connect(port, SERIAL_PORT, NULL);
    if (rc < 0) {
        fprintf(stderr, "+++ error: sio_connect returnd %i (%i)\n", rc, errno);
        goto end;
    }
    rc = sio_transmit(port, cmd_close, 2);
    if (rc < 0) {
        fprintf(stderr, "+++ error: sio_transmit (close) returnd %i (%i)\n", rc, errno);
        goto teardown;
    }
    rc = sio_transmit(port, cmd_open, 2);
    if (rc < 0) {
        fprintf(stderr, "+++ error: sio_transmit (open) returnd %i (%i)\n", rc, errno);
        goto teardown;
    }
    while (running) {
        usleep(10);
    }
    rc = sio_transmit(port, cmd_close, 2);
    if (rc < 0) {
        fprintf(stderr, "+++ error: sio_transmit (close) returnd %i (%i)\n", rc, errno);
        goto teardown;
    }
teardown:
    rc = sio_disconnect(port);
    if (rc < 0) {
        fprintf(stderr, "+++ error: sio_disconnect returnd %i (%i)\n", rc, errno);
        goto end;
    }
end:
    rc = sio_destroy(port);
    if (rc < 0) {
        fprintf(stderr, "+++ error: sio_destroy returnd %i (%i)\n", rc, errno);
    }
    return rc;
}

static int main_slcan(int argc, char *argv[]) {
    slcan_port_t port = NULL;
    int rc = 0;

    port = slcan_create(8U);
    if (!port) {
        fprintf(stderr, "+++ error: slcan_create returnd NULL (%i)\n", errno);
        return -1;
    }
    rc = slcan_connect(port, SERIAL_PORT);
    if (rc < 0) {
        fprintf(stderr, "+++ error: slcan_connect returnd %i (%i)\n", rc, errno);
        goto end;
    }
    rc = slcan_setup_bitrate(port, CAN_250K);
    if (rc < 0) {
        fprintf(stderr, "+++ error: slcan_setup_bitrate returnd %i (%i)\n", rc, errno);
        goto teardown;
    }
    rc = slcan_open_channel(port);
    if (rc < 0) {
        fprintf(stderr, "+++ error: slcan_open_channel returnd %i (%i)\n", rc, errno);
        goto teardown;
    }
    while (running) {
        usleep(10);
    }
    rc = slcan_close_channel(port);
    if (rc < 0) {
        fprintf(stderr, "+++ error: slcan_close_channel returnd %i (%i)\n", rc, errno);
        goto teardown;
    }
teardown:
    rc = slcan_disconnect(port);
    if (rc < 0) {
        fprintf(stderr, "+++ error: slcan_disconnect returnd %i (%i)\n", rc, errno);
        goto end;
    }
end:
    rc = slcan_destroy(port);
    if (rc < 0) {
        fprintf(stderr, "+++ error: slcan_destroy returnd %i (%i)\n", rc, errno);
    }
    return rc;
}

int main(int argc, char *argv[]) {
    int rc = 0;

    if((signal(SIGINT, sigterm) == SIG_ERR) ||
#if !defined(_WIN32) && !defined(_WIN64)
       (signal(SIGHUP, sigterm) == SIG_ERR) ||
#endif
       (signal(SIGTERM, sigterm) == SIG_ERR)) {
        perror("+++ error");
        return errno;
    }
    LOGGER_INIT(NULL);
    fprintf(stdout, "!!! %s %s %s\n",__FILE__,__DATE__,__TIME__);
    rc = main_slcan(argc, argv);
    fprintf(stdout, "!!! Cheers!\n");
    LOGGER_EXIT();
    return rc;
}

static void sigterm(int signo) {
     fprintf(stderr, "%s: got signal %d\n", __FILE__, signo);
     running = 0;
     (void)signo;
}
