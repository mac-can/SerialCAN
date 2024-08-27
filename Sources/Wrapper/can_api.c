/*  SPDX-License-Identifier: BSD-2-Clause OR GPL-3.0-or-later */
/*
 *  CAN Interface API, Version 3 (for CAN-over-Serial-Line Interfaces)
 *
 *  Copyright (c) 2005-2010 Uwe Vogt, UV Software, Friedrichshafen
 *  Copyright (c) 2016-2024 Uwe Vogt, UV Software, Berlin (info@uv-software.com)
 *  All rights reserved.
 *
 *  This file is part of SerialCAN.
 *
 *  SerialCAN is dual-licensed under the BSD 2-Clause "Simplified" License
 *  and under the GNU General Public License v3.0 (or any later version). You can
 *  choose between one of them if you use SerialCAN in whole or in part.
 *
 *  BSD 2-Clause "Simplified" License:
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are met:
 *  1. Redistributions of source code must retain the above copyright notice, this
 *     list of conditions and the following disclaimer.
 *  2. Redistributions in binary form must reproduce the above copyright notice,
 *     this list of conditions and the following disclaimer in the documentation
 *     and/or other materials provided with the distribution.
 *
 *  SerialCAN IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 *  DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 *  FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 *  DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 *  SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 *  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 *  OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *  OF SerialCAN, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *  GNU General Public License v3.0 or later:
 *  SerialCAN is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  SerialCAN is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with SerialCAN.  If not, see <https://www.gnu.org/licenses/>.
 */
/** @addtogroup  can_api
 *  @{
 */
#ifdef _MSC_VER
//no Microsoft extensions please!
#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS 1
#endif
#endif
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

/*  -----------  includes  -----------------------------------------------
 */
#include "can_defs.h"
#include "can_api.h"
#include "can_btr.h"

#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>
#include "slcan.h"
#else
#include <unistd.h>
#include "slcan.h"
#endif
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <errno.h>

/*  -----------  options  ------------------------------------------------
 */
#if (OPTION_CAN_2_0_ONLY != 0)
#ifdef _MSC_VER
#pragma message ( "Compilation with legacy CAN 2.0 frame format!" )
#else
#warning Compilation with legacy CAN 2.0 frame format!
#endif
#endif
#if ((OPTION_CANAPI_SERIALCAN_DYLIB != 0) || (OPTION_CANAPI_SERIALCAN_SO != 0))
__attribute__((constructor))
static void _initializer() {
    // default initializer
}
__attribute__((destructor))
static void _finalizer() {
    // default finalizer
}
#define EXPORT  __attribute__((visibility("default")))
#else
#define EXPORT
#endif

/*  -----------  defines  ------------------------------------------------
 */
#ifndef CAN_MAX_HANDLES
#define CAN_MAX_HANDLES         (16)    // maximum number of open handles
#endif
#define INVALID_HANDLE          (-1)
#define IS_HANDLE_VALID(hnd)    ((0 <= (hnd)) && ((hnd) < CAN_MAX_HANDLES))
#define IS_HANDLE_OPENED(hnd)   (can[hnd].port != NULL)

#define SERIAL_BAUDRATE         57600U
#define SERIAL_BYTESIZE         CANSIO_8DATABITS
#define SERIAL_PARITY           CANSIO_NOPARITY
#define SERIAL_STOPBITS         CANSIO_1STOPBIT
#define SERIAL_PROTOCOL         CANSIO_LAWICEL

#define SUPPORTED_OP_MODE       (CANMODE_DEFAULT)
#define CAN_CLOCK_FREQUENCY     CANBTR_FREQ_SJA1000
#define CAN_BTR_DEFAULT         0x011CU
#define SLCAN_QUEUE_SIZE        65536U
#define FILTER_STD_CODE         (uint32_t)(0x000)
#define FILTER_STD_MASK         (uint32_t)(0x000)
#define FILTER_XTD_CODE         (uint32_t)(0x00000000)
#define FILTER_XTD_MASK         (uint32_t)(0x00000000)
#define FILTER_STD_XOR_MASK     (uint64_t)(0x00000000000007FF)
#define FILTER_XTD_XOR_MASK     (uint64_t)(0x000000001FFFFFFF)
#define FILTER_STD_VALID_MASK   (uint64_t)(0x000007FF000007FF)
#define FILTER_XTD_VALID_MASK   (uint64_t)(0x1FFFFFFF1FFFFFFF)
#define FILTER_RESET_VALUE      (uint64_t)(0x0000000000000000)
#define FILTER_SJA1000_CODE     (uint32_t)(0x00000000)
#define FILTER_SJA1000_MASK     (uint32_t)(0xFFFFFFFF)
#ifndef SYSERR_OFFSET
#define SYSERR_OFFSET           (-10000)
#endif
#define LIB_ID                  SLCAN_LIB_ID
#define LIB_DLLNAME             SLCAN_LIB_WRAPPER
#define DEV_VENDOR              SLCAN_LIB_VENDOR
#define DEV_DLLNAME             SLCAN_LIB_DRIVER
#define NUM_CHANNELS            CANSIO_BOARDS

/*  -----------  types  --------------------------------------------------
 */
typedef struct {                        // message filtering:
    struct {                            //   acceptance filter:
        uint32_t code;                  //     acceptance code
        uint32_t mask;                  //     acceptance mask
    } std, xtd,                         //   for standard and extended frames
      sja1000;                          //   and SJA1000 ACn and AMn register
}   can_filter_t;

typedef struct {                        // frame counters:
    uint64_t tx;                        //   number of transmitted CAN frames
    uint64_t rx;                        //   number of received CAN frames
    uint64_t err;                       //   number of receiced error frames
}   can_counter_t;

typedef struct {                        // SLCAN interface:
    slcan_port_t port;                  //   serial communication port
    can_sio_attr_t attr;                //   serial communication attributes
    can_mode_t mode;                    //   operation mode of the CAN channel
    can_filter_t filter;                //   message filter settings
    can_status_t status;                //   8-bit status register
    can_counter_t counters;             //   statistical counters
    uint16_t btr0btr1;                  //   bit-rate settings
    char name[CANPROP_MAX_BUFFER_SIZE]; //   TTY device name
}   can_interface_t;

/*  -----------  prototypes  ---------------------------------------------
 */
static void var_init(void);             // initialize all variables
static int all_closed(void);            // check if all handles closed

static int exit_channel(int handle);    // teardown a single channel
static int kill_channel(int handle);    // signal a single channel

static slcan_attr_t* slcan_attr(const can_sio_attr_t* attr);
static int slcan_error(int code);       // SLCAN specific errors
static int get_sio_attr(slcan_port_t port, can_sio_attr_t *attr);
static int set_filter(int handle, uint64_t filter, bool xtd);
static int reset_filter(int handle);

static int lib_parameter(uint16_t param, void *value, size_t nbyte);
static int drv_parameter(int handle, uint16_t param, void *value, size_t nbyte);

/*  -----------  variables  ----------------------------------------------
 */
static const char version[] = "CAN API V3 for CAN-over-Serial-Line Interfaces, Version " VERSION_STRING;

EXPORT
can_board_t can_boards[NUM_CHANNELS+1] = {  // list of supported CAN channels
    // note: check the device manager of '/dev/tty*' to find compatibe interfaces
    {EOF, NULL}
};
//static const uint8_t dlc_table[16] = {  // DLC to length
//    0,1,2,3,4,5,6,7,8,12,16,20,24,32,48,64
//};
static can_interface_t can[CAN_MAX_HANDLES];  // interface handles
static int init = 0;                    // initialization flag

/*  -----------  functions  ----------------------------------------------
 */
EXPORT
int can_test(int32_t channel, uint8_t mode, const void *param, int *result)
{
    int rc = CANERR_NOERROR;            // return value
    int i;                              // loop variable

    if (result)                         // serial device not testtable
        *result = CANBRD_NOT_TESTABLE;

#if (OPTION_SERIAL_CHANNEL != 0)
    if (channel != CANDEV_SERIAL)       // must be serial port device!
#ifndef OPTION_CANAPI_RETVALS
        return CANERR_HANDLE;
#else
        // note: can_test shall return vendor-specific error code or
        //       CANERR_NOTINIT in this case
        return CANERR_NOTINIT;
#endif
#else
    // note: channel no. is not used in this implementation
    //       because the serial port is selected by its name
    (void)channel;                      // to avoid compiler warning
#endif
    if (param == NULL)                  // must have serial port parameter
        return CANERR_NULLPTR;

    char* name = ((can_sio_param_t*)param)->name;
    if (name == NULL)                   // must have at least a TTY name
        return CANERR_NULLPTR;

    if (!init) {                        // if not initialized:
        var_init();                     //   initialize all variables
        init = 1;                       //   set initialization flag
    }
    // check requested SLCAN protocol option
    switch (((can_sio_param_t*)param)->attr.protocol) {
    case CANSIO_LAWICEL: break;         //   Lawicel SLCAN protocol
    case CANSIO_CANABLE: break;         //   CANable SLCAN protocol
    default:                            //   sorry, not supported
        rc = CANERR_ILLPARA;
        goto end_test;
    }
    // check if requested operation mode is supported
    if ((mode & (uint8_t)(~SUPPORTED_OP_MODE)) != 0) {
        rc = CANERR_ILLPARA;
        //goto end_test;
    }
    /* check if the SLCAN device is occupied by own process */
    for (i = 0; i < CAN_MAX_HANDLES; i++) {
        if (can[i].port && !strcmp(can[i].name, name)) {
            if (result)
                *result = CANBRD_OCCUPIED;
            break;
        }
    }
end_test:
    // when the music is over, turn out the lights
#if (1)
    if (all_closed()) {                 // if no open handle then
        init = 0;                       //   clear initialization flag
    }
#endif
    return rc;
}

EXPORT
int can_init(int32_t channel, uint8_t mode, const void *param)
{
    int rc = CANERR_FATAL;              // return value
    int handle = (-1);                  // handle index
    int fd = (-1);                      // file descriptor

#if (OPTION_SERIAL_CHANNEL != 0)
    if (channel != CANDEV_SERIAL)       // must be serial port device!
#ifndef OPTION_CANAPI_RETVALS
        return CANERR_HANDLE;
#else
        // note: can_init shall return vendor-specific error code or
        //       CANERR_NOTINIT in this case
        return CANERR_NOTINIT;
#endif
#else
    // note: channel no. is not used in this implementation
    //       because the serial port is selected by its name
    (void)channel;                      // to avoid compiler warning
#endif
    if (param == NULL)                  // must have serial port parameter
        return CANERR_NULLPTR;

    char* name = ((can_sio_param_t*)param)->name;
    if (name == NULL)                   // must have at least a TTY name
        return CANERR_NULLPTR;

    if (!init) {                        // if not initialized:
        var_init();                     //   initialize all variables
        init = 1;                       //   set initialization flag
    }
    for (handle = 0; handle < CAN_MAX_HANDLES; handle++) {
        if ((can[handle].port != NULL) &&  // channel already in use
            !strcmp(can[handle].name, name))
            return CANERR_YETINIT;
    }
    for (handle = 0; handle < CAN_MAX_HANDLES; handle++) {
        if (can[handle].port == NULL)   // get an unused handle, if any
            break;
    }
    if (!IS_HANDLE_VALID(handle)) {     // no free handle found
        return CANERR_NOTINIT;
    }
    // check requested SLCAN protocol option
    switch (((can_sio_param_t*)param)->attr.protocol) {
    case CANSIO_LAWICEL: break;         //   Lawicel SLCAN protocol
    case CANSIO_CANABLE: break;         //   CANable SLCAN protocol
    default:                            //   sorry, not supported
        rc = CANERR_ILLPARA;
        goto err_init;
    }
    // check if requested operation mode is supported
    if ((mode & (uint8_t)(~SUPPORTED_OP_MODE)) != 0) {
        rc = CANERR_ILLPARA;
        goto err_init;
    }
    // create an SLCAN port (w/ message queue)
    can[handle].port = slcan_create(SLCAN_QUEUE_SIZE);
    if (can[handle].port == NULL) {
        rc = slcan_error(-1);
        goto err_init;
    }
    // connect serial interface (returns a file descriptor)
    fd = slcan_connect(can[handle].port, name, slcan_attr(&((can_sio_param_t*)param)->attr));
    rc = slcan_error(fd);
    if (fd < 0) {                       // errno is set in this case
        (void)slcan_destroy(can[handle].port);
        goto err_init;
    }
    // check for SLCAN protocol (Lawicel or CANable protocol)
    if (((can_sio_param_t*)param)->attr.protocol != CANSIO_CANABLE) {
        // dummy read to check the protocol (w/ ACK/NACK feedback)
        rc = slcan_version_number(can[handle].port, NULL, NULL);
        if ((rc < 0) && (errno == EBADMSG)) {   // wrong protocol (errno is set)
            rc = CANERR_VENDOR;
            errno = 0;                  //   clear errno to return CAN API error
        }
    }
    else {
        // disable ACK/NAK feedback for serial commands
        rc = slcan_set_ack(can[handle].port, false);
    }
    rc = slcan_error(rc);
    if (rc != CANERR_NOERROR) {         // errno is set in this case
        (void)slcan_disconnect(can[handle].port);
        (void)slcan_destroy(can[handle].port);
        goto err_init;
    }
    // reset CAN controller (it's possibly running)
    (void)slcan_close_channel(can[handle].port);

    // store the tty name and the operation mode
    strncpy(can[handle].name, &name[0], CANPROP_MAX_BUFFER_SIZE);
    can[handle].name[CANPROP_MAX_BUFFER_SIZE - 1] = '\0';
    can[handle].attr.protocol = ((can_sio_param_t*)param)->attr.protocol;
    (void)get_sio_attr(can[handle].port, &can[handle].attr);
    can[handle].mode.byte = mode;       // store selected operation mode
    can[handle].status.byte = CANSTAT_RESET; // CAN controller not started yet
    return handle;                      // return the handle

err_init:                               // otherwise:
    can[handle].port = NULL;
    return rc;                          // return error code
}

static int exit_channel(int handle)
{
    int rc;                             // return value

    if (!IS_HANDLE_VALID(handle))       // must be a valid handle
        return CANERR_HANDLE;
    if (!IS_HANDLE_OPENED(handle))      // must be an open handle
        return CANERR_HANDLE;
    if (!can[handle].status.can_stopped) { // if running then go bus off
        (void)can_reset(handle);
    }
    rc = slcan_disconnect(can[handle].port);  // disconnect serial interface
    rc = slcan_error(rc);
    if (rc != CANERR_NOERROR) {         // errno is set in this case
        return rc;
    }
    (void)slcan_destroy(can[handle].port);  // destroy SLCAN port

    can[handle].status.byte |= CANSTAT_RESET;  // CAN controller in INIT state
    can[handle].port = NULL;            // handle can be used again
    return CANERR_NOERROR;
}

EXPORT
int can_exit(int handle)
{
    int rc;                             // return value
    int i;                              // loop variable

    if (!init)                          // must be initialized
        return CANERR_NOTINIT;
    if (handle != CANEXIT_ALL) {        // close a single handle
        if ((rc = exit_channel(handle)) != CANERR_NOERROR)
            return rc;
    }
    else {                              // close all open handles
        for (i = 0; i < CAN_MAX_HANDLES; i++) {
            (void)exit_channel(i);      //   don't care about the result
        }
    }
    // when the music is over, turn out the lights
    if (all_closed()) {                 // if no open handle then
        init = 0;                       //   clear initialization flag
    }
    return CANERR_NOERROR;
}

static int kill_channel(int handle)
{
    int rc;                             // return value

    if (!IS_HANDLE_VALID(handle))       // must be a valid handle
        return CANERR_HANDLE;
    if (!IS_HANDLE_OPENED(handle))      // must be an open handle
        return CANERR_HANDLE;
    rc = slcan_signal(can[handle].port);// wake up the SLCAN thread
    rc = slcan_error(rc);
    if (rc != CANERR_NOERROR) {         // errno is set in this case
        return rc;
    }
    return CANERR_NOERROR;
}

EXPORT
int can_kill(int handle)
{
    int rc;                             // return value
    int i;                              // loop variable

    if (!init)                          // must be initialized
        return CANERR_NOTINIT;
    if (handle != CANKILL_ALL) {        // signal a single handle
        if ((rc = kill_channel(handle)) != CANERR_NOERROR)
            return rc;
    }
    else {                              // signal all open handles
        for (i = 0; i < CAN_MAX_HANDLES; i++) {
            (void)kill_channel(i);      //   don't care about the result
        }
    }
    return CANERR_NOERROR;
}

EXPORT
int can_start(int handle, const can_bitrate_t *bitrate)
{
    int rc = CANERR_FATAL;              // return value

    uint16_t btr0btr1 = CAN_BTR_DEFAULT;// btr0btr1 value
    can_bitrate_t temporary;            // bit-rate settings

    if (!init)                          // must be initialized
        return CANERR_NOTINIT;
    if (!IS_HANDLE_VALID(handle))       // must be a valid handle
        return CANERR_HANDLE;
    if (!IS_HANDLE_OPENED(handle))      // must be an open handle
        return CANERR_HANDLE;
    if (bitrate == NULL)                // check for null-pointer
        return CANERR_NULLPTR;
    if (!can[handle].status.can_stopped) // must be stopped
        return CANERR_ONLINE;

    // note: CANable devices do not support SJA1000 bit-rate settings
    //
    if ((bitrate->index > 0) && (can[handle].attr.protocol == CANSIO_CANABLE)) {
        // convert bit-rate settings to index (SJA1000)
        if(btr_bitrate2index(bitrate, &temporary.index) != CANERR_NOERROR)
            return CANERR_BAUDRATE;
        // indexes are defined as negative numbers or zero:  -8 = 10kbps, ..., 0 = 1Mbps
        if ((temporary.index < CANBTR_INDEX_10K) || (temporary.index > CANBTR_INDEX_1M))
            return CANERR_BAUDRATE;
    }
    else {
        // accept both: bit-rate settings or index
        memcpy(&temporary, bitrate, sizeof(can_bitrate_t));
    }
    // set bit-rate (from index or BTR0BTR1 register)
    if (temporary.index <= 0) {
        // convert index to SJA1000 BTR0/BTR1 register
        if (btr_index2sja1000(temporary.index, &btr0btr1) != CANERR_NOERROR)
            return CANERR_BAUDRATE;
        // set the bit-rate (with reverse index numbering)
        rc = slcan_setup_bitrate(can[handle].port, (uint8_t)(CANBDR_10 + temporary.index));
    }
    else {
        // convert bit-rate to SJA1000 BTR0/BTR1 register
        if (btr_bitrate2sja1000(&temporary, &btr0btr1) != CANERR_NOERROR)
            return CANERR_BAUDRATE;
        // set the bit-timing register
        rc = slcan_setup_btr(can[handle].port, btr0btr1);
    }
    if (rc < 0)
        return slcan_error(rc);
    // set acceptance filter (code and mask)
    if (can[handle].attr.protocol != CANSIO_CANABLE) {
        rc = slcan_acceptance_code(can[handle].port, can[handle].filter.sja1000.code);
        if (rc < 0)
            return slcan_error(rc);
        rc = slcan_acceptance_mask(can[handle].port, can[handle].filter.sja1000.mask);
        if (rc < 0)
            return slcan_error(rc);
    }
    // start the CAN controller
    rc = slcan_open_channel(can[handle].port);
    if (rc < 0)
        return slcan_error(rc);
    // store the bit-rate settings
    can[handle].btr0btr1 = btr0btr1;
    // clear old status and counters
    can[handle].status.byte = 0x00u;
    can[handle].counters.tx = 0ull;
    can[handle].counters.rx = 0ull;
    can[handle].counters.err = 0ull;
    // CAN controller started!
    can[handle].status.can_stopped = 0;
    return CANERR_NOERROR;
}

EXPORT
int can_reset(int handle)
{
    int rc = CANERR_FATAL;              // return value

    if (!init)                          // must be initialized
        return CANERR_NOTINIT;
    if (!IS_HANDLE_VALID(handle))       // must be a valid handle
        return CANERR_HANDLE;
    if (!IS_HANDLE_OPENED(handle))      // must be an open handle
        return CANERR_HANDLE;
    if (can[handle].status.can_stopped) // must be running
#if (OPTION_CANAPI_RETVALS == OPTION_DISABLED)
        return CANERR_OFFLINE;
#else
        // note: can_reset shall return CANERR_NOERROR even if
        //       the CAN controller has not been started
        return CANERR_NOERROR;
#endif
    // stop the CAN controller (INIT state)
    rc = slcan_close_channel(can[handle].port);
    rc = slcan_error(rc);
    can[handle].status.can_stopped = (rc == CANERR_NOERROR) ? 1 : 0;
    return rc;
}

EXPORT
int can_write(int handle, const can_message_t *msg, uint16_t timeout)
{
    slcan_message_t slcan;              // SLCAN message
    int rc = CANERR_FATAL;              // return value

    if (!init)                          // must be initialized
        return CANERR_NOTINIT;
    if (!IS_HANDLE_VALID(handle))       // must be a valid handle
        return CANERR_HANDLE;
    if (!IS_HANDLE_OPENED(handle))      // must be an open handle
        return CANERR_HANDLE;
    if (msg == NULL)                    // check for null-pointer
        return CANERR_NULLPTR;
    if (can[handle].status.can_stopped) // must be running
        return CANERR_OFFLINE;

    if (msg->id > (uint32_t)(msg->xtd ? CAN_MAX_XTD_ID : CAN_MAX_STD_ID))
        return CANERR_ILLPARA;          // invalid identifier
    if (msg->dlc > CAN_MAX_DLC)
        return CANERR_ILLPARA;          // invalid data length code
    if (msg->xtd && can[handle].mode.nxtd)
        return CANERR_ILLPARA;          // suppress extended frames
    if (msg->rtr && can[handle].mode.nrtr)
        return CANERR_ILLPARA;          // suppress remote frames
    if (msg->sts)
        return CANERR_ILLPARA;          // error frames cannot be sent

    // map message layout
    memset(&slcan, 0x00, sizeof(slcan_message_t));
    slcan.can_id = msg->id & (msg->xtd ? CAN_XTD_MASK : CAN_STD_MASK);
    slcan.can_id |= (msg->xtd ? CAN_XTD_FRAME : 0x00000000U);
    slcan.can_id |= (msg->rtr ? CAN_RTR_FRAME : 0x00000000U);
    slcan.can_dlc = msg->dlc;
    memcpy(slcan.data, msg->data, slcan.can_dlc);
    // transmit the CAN message
    rc = slcan_write_message(can[handle].port, &slcan, timeout);
    rc = slcan_error(rc);
    // update status and tx counter
    can[handle].status.transmitter_busy = (rc != CANERR_NOERROR) ? 1 : 0;
    can[handle].counters.tx += (rc == CANERR_NOERROR) ? 1U : 0U;

    return rc;
}

EXPORT
int can_read(int handle, can_message_t *msg, uint16_t timeout)
{
    slcan_message_t slcan;              // SLCAN message
    int rc = CANERR_FATAL;              // return value

    if (!init)                          // must be initialized
        return CANERR_NOTINIT;
    if (!IS_HANDLE_VALID(handle))       // must be a valid handle
        return CANERR_HANDLE;
    if (!IS_HANDLE_OPENED(handle))      // must be an open handle
        return CANERR_HANDLE;
    if (msg == NULL)                    // check for null-pointer
        return CANERR_NULLPTR;
    if (can[handle].status.can_stopped) // must be running
        return CANERR_OFFLINE;

    memset(msg, 0x00, sizeof(can_message_t));
    msg->id = 0xFFFFFFFFu;
    msg->sts = 1;

    // read one CAN message from message queue, if any
    rc = slcan_read_message(can[handle].port, &slcan, timeout);
    if (rc == CANERR_NOERROR) {
        // map message layout
        msg->xtd = (slcan.can_id & CAN_XTD_FRAME) ? 1 : 0;
        msg->sts = (slcan.can_id & CAN_ERR_FRAME) ? 1 : 0;
        msg->rtr = (slcan.can_id & CAN_RTR_FRAME) ? 1 : 0;
        msg->id = slcan.can_id & (msg->xtd ? CAN_XTD_MASK : CAN_STD_MASK);
        msg->dlc = (slcan.can_dlc < CAN_DLC_MAX) ? slcan.can_dlc : CAN_LEN_MAX;
        memcpy(msg->data, slcan.data, msg->dlc);
        // update receive counter
        can[handle].counters.rx += !msg->sts ? 1U : 0U;
        can[handle].counters.err += msg->sts ? 1U : 0U;
    }
    else if (rc != CANERR_RX_EMPTY) {
        rc = slcan_error(rc);
    }
    else {
        rc = CANERR_RX_EMPTY;
    }
    // update status register
    can[handle].status.receiver_empty = (rc != CANERR_NOERROR) ? 1 : 0;
    can[handle].status.queue_overrun |= (errno == ENOSPC) ? 1 : 0;

    return rc;
}

EXPORT
int can_status(int handle, uint8_t *status)
{
    int rc = CANERR_FATAL;              // return value

    slcan_flags_t flags;                // SLCAN flags

    if (!init)                          // must be initialized
        return CANERR_NOTINIT;
    if (!IS_HANDLE_VALID(handle))       // must be a valid handle
        return CANERR_HANDLE;
    if (!IS_HANDLE_OPENED(handle))      // must be an open handle
        return CANERR_HANDLE;

    if (!can[handle].status.can_stopped) { // if running get bus status
        // get status-register from device (CAN API V1 compatible)
        if ((rc = slcan_status_flags(can[handle].port, &flags)) < 0)
            return slcan_error(rc);
        // TODO: SJA1000 datasheet, rtfm!
        can[handle].status.message_lost = (flags.DOI | flags.RxFIFO | flags.TxFIFO) ? 1 : 0;
        can[handle].status.bus_error = flags.BEI ? 1 : 0;
        can[handle].status.warning_level = (flags.EI | flags.EPI);
        can[handle].status.bus_off = flags.ALI;
    }
    if (status)                         // status-register
        *status = can[handle].status.byte;
    return CANERR_NOERROR;
}

EXPORT
int can_busload(int handle, uint8_t *load, uint8_t *status)
{
    int rc = CANERR_FATAL;              // return value
    float busLoad = 0.0;                // bus-load (in [percent])

    if (!init)                          // must be initialized
        return CANERR_NOTINIT;
    if (!IS_HANDLE_VALID(handle))       // must be a valid handle
        return CANERR_HANDLE;
    if (!IS_HANDLE_OPENED(handle))      // must be an open handle
        return CANERR_HANDLE;

    if (!can[handle].status.can_stopped) { // if running get bus load
        (void)busLoad; //  TODO: measure bus load
    }
    if (load)                           // bus-load (in [percent])
        *load = (uint8_t)busLoad;
    // get status-register from device
    rc = can_status(handle, status);
#if (OPTION_CANAPI_RETVALS == OPTION_DISABLED)
    if (rc == CANERR_NOERROR)
        rc = !can[handle].status.can_stopped ? CANERR_NOERROR : CANERR_OFFLINE;
#else
    // note: can_busload shall return CANERR_NOERROR if
    //       the CAN controller has not been started
#endif
    return rc;
}

EXPORT
int can_bitrate(int handle, can_bitrate_t *bitrate, can_speed_t *speed)
{
    int rc = CANERR_FATAL;              // return value
    can_bitrate_t tmpBitrate;           // bit-rate settings
    can_speed_t tmpSpeed;               // transmission speed

    memset(&tmpBitrate, 0, sizeof(can_bitrate_t));
    memset(&tmpSpeed, 0, sizeof(can_speed_t));

    if (!init)                          // must be initialized
        return CANERR_NOTINIT;
    if (!IS_HANDLE_VALID(handle))       // must be a valid handle
        return CANERR_HANDLE;
    if (!IS_HANDLE_OPENED(handle))      // must be an open handle
        return CANERR_HANDLE;

    // get bit-rate settings from SJA1000 registers
    if ((rc = btr_sja10002bitrate(can[handle].btr0btr1, &tmpBitrate)) == CANERR_NOERROR)
        rc = btr_bitrate2speed(&tmpBitrate, &tmpSpeed);
    /* note: 'bitrate' as well as 'speed' are optional */
    if (bitrate)
        memcpy(bitrate, &tmpBitrate, sizeof(can_bitrate_t));
    if (speed)
        memcpy(speed, &tmpSpeed, sizeof(can_speed_t));
#if (OPTION_CANAPI_RETVALS == OPTION_DISABLED)
    if (rc == CANERR_NOERROR)
        rc = !can[handle].status.can_stopped ? CANERR_NOERROR : CANERR_OFFLINE;
#else
    // note: can_bitrate shall return CANERR_NOERROR if
    //       the CAN controller has not been started
#endif
    return rc;
}

EXPORT
int can_property(int handle, uint16_t param, void *value, uint32_t nbyte)
{
    if (!init || !IS_HANDLE_VALID(handle)) {
        // note: library properties can be queried w/o a handle
        return lib_parameter(param, value, (size_t)nbyte);
    }
    // note: library is initialized and handle is valid

    if (!IS_HANDLE_OPENED(handle))      // must be an open handle
        return CANERR_HANDLE;
    // note: device properties must be queried with a valid handle
    return drv_parameter(handle, param, value, (size_t)nbyte);
}

EXPORT
char *can_hardware(int handle)
{
    static char hardware[(2 * CANPROP_MAX_BUFFER_SIZE) + 1] = "";
    uint8_t hw_version = 0x00U;

    if (!init)                          // must be initialized
        return NULL;
    if (!IS_HANDLE_VALID(handle))       // must be a valid handle
        return NULL;
    if (!IS_HANDLE_OPENED(handle))      // must be an open handle
        return NULL;

    // get version number: HW and SW
    if (slcan_version_number(can[handle].port, &hw_version, NULL) < 0)
        return NULL;

    // note: TTY name has at worst 255 characters plus terminating zero
    snprintf(hardware, (2 * CANPROP_MAX_BUFFER_SIZE), "Hardware %u.%u (%s:%u,%u-%c-%u)",
        (uint8_t)(hw_version >> 4), (uint8_t)(hw_version & 0xFU),
        can[handle].name, can[handle].attr.baudrate, can[handle].attr.bytesize,
        can[handle].attr.parity == CANSIO_EVENPARITY ? 'E' : (can[handle].attr.parity == CANSIO_ODDPARITY ? 'O' : 'N'),
        can[handle].attr.stopbits);
    hardware[(2 * CANPROP_MAX_BUFFER_SIZE)] = '\0';
    return (char*)hardware;
}

EXPORT
char *can_firmware(int handle)
{
    static char firmware[CANPROP_MAX_BUFFER_SIZE+1] = "";
    uint8_t sw_version = 0x00U;

    if (!init)                          // must be initialized
        return NULL;
    if (!IS_HANDLE_VALID(handle))       // must be a valid handle
        return NULL;
    if (!IS_HANDLE_OPENED(handle))      // must be an open handle
        return NULL;

    // get version number: HW and SW
    if (slcan_version_number(can[handle].port, NULL, &sw_version) < 0)
        return NULL;

    snprintf(firmware, CANPROP_MAX_BUFFER_SIZE, "Firmware %u.%u (%s SLCAN protocol)",
        (uint8_t)(sw_version >> 4), (uint8_t)(sw_version & 0xFU),
        can[handle].attr.protocol == CANSIO_LAWICEL ? "Lawicel" :
        can[handle].attr.protocol == CANSIO_CANABLE ? "CANable" : "?");
    firmware[CANPROP_MAX_BUFFER_SIZE] = '\0';
    return (char*)firmware;
}

/*  -----------  local functions  ----------------------------------------
 */
static void var_init(void)
{
    int i;

    for (i = 0; i < CAN_MAX_HANDLES; i++) {
        memset(&can[i], 0, sizeof(can_interface_t));
        can[i].port = NULL;
        can[i].name[0] = '\0';
        can[i].attr.baudrate = SERIAL_BAUDRATE;
        can[i].attr.bytesize = SERIAL_BYTESIZE;
        can[i].attr.parity = SERIAL_PARITY;
        can[i].attr.stopbits = SERIAL_STOPBITS;
        can[i].attr.protocol = SERIAL_PROTOCOL;
        can[i].btr0btr1 = CAN_BTR_DEFAULT;
        can[i].mode.byte = CANMODE_DEFAULT;
        can[i].status.byte = CANSTAT_RESET;
        can[i].filter.sja1000.code = FILTER_SJA1000_CODE;
        can[i].filter.sja1000.mask = FILTER_SJA1000_MASK;
        can[i].filter.std.code = FILTER_STD_CODE;
        can[i].filter.std.mask = FILTER_STD_MASK;
        can[i].filter.xtd.code = FILTER_XTD_CODE;
        can[i].filter.xtd.mask = FILTER_XTD_MASK;
        can[i].counters.tx = 0ull;
        can[i].counters.rx = 0ull;
        can[i].counters.err = 0ull;
    }
}

static int all_closed(void)
{
    int handle;

    if (!init)
        return 1;
    for (handle = 0; handle < CAN_MAX_HANDLES; handle++) {
        if (IS_HANDLE_OPENED(handle))
            return 0;
    }
    return 1;
}

static int slcan_error(int code)
{
    int rc = CANERR_NOERROR;

    // note: in case that a slcan function returns -1
    //       the system error variable 'errno' is set
    if (code < 0) {
        switch (errno) {
        case EINVAL:   rc = CANERR_ILLPARA; break;
        case ENODEV:   rc = CANERR_HANDLE; break;
        case EBADF:    rc = CANERR_NOTINIT; break;
        case EALREADY: rc = CANERR_YETINIT; break;
        default:       rc = CANERR_VENDOR - errno; break;
        }
    }
    return rc;
}

static slcan_attr_t* slcan_attr(const can_sio_attr_t *attr)
{
    static slcan_attr_t slcan;

    assert(attr);

    slcan.baudrate = attr->baudrate;    // in bits per second
    switch (attr->bytesize) {
    case CANSIO_5DATABITS: slcan.bytesize = BYTESIZE5; break;
    case CANSIO_6DATABITS: slcan.bytesize = BYTESIZE6; break;
    case CANSIO_7DATABITS: slcan.bytesize = BYTESIZE7; break;
    default: slcan.bytesize = BYTESIZE8; break;
    }
    switch (attr->stopbits) {
    case CANSIO_2STOPBITS: slcan.stopbits = STOPBITS2; break;
    default: slcan.stopbits = STOPBITS1; break;
    }
    switch (attr->parity) {
    case CANSIO_ODDPARITY: slcan.parity = PARITYODD; break;
    case CANSIO_EVENPARITY: slcan.parity = PARITYEVEN; break;
    default: slcan.parity = PARITYNONE; break;
    }
    return &slcan;
}

static int get_sio_attr(slcan_port_t port, can_sio_attr_t *attr)
{
    slcan_attr_t slcan;
    int rc;

    assert(attr);

    if ((rc = slcan_get_attr(port, &slcan)) == 0) {
        switch (slcan.parity) {
        case PARITYNONE: attr->parity = CANSIO_NOPARITY; break;
        case PARITYODD: attr->parity = CANSIO_ODDPARITY; break;
        case PARITYEVEN: attr->parity = CANSIO_EVENPARITY; break;
        default: attr->parity = 0; break;
        }
        switch (slcan.stopbits) {
        case STOPBITS1: attr->stopbits = CANSIO_1STOPBIT; break;
        case STOPBITS2: attr->stopbits = CANSIO_2STOPBITS; break;
        default: attr->stopbits = 0; break;
        }
        switch (slcan.bytesize) {
        case BYTESIZE5: attr->bytesize = CANSIO_5DATABITS; break;
        case BYTESIZE6: attr->bytesize = CANSIO_6DATABITS; break;
        case BYTESIZE7: attr->bytesize = CANSIO_7DATABITS; break;
        case BYTESIZE8: attr->bytesize = CANSIO_8DATABITS; break;
        default: attr->bytesize = 0; break;
        }
        attr->baudrate = slcan.baudrate;// in bits per second
    } else {
        attr->baudrate = 0;
        attr->bytesize = 0;
        attr->stopbits = 0;
        attr->parity = 0;
    }
    return rc;
}

static int set_filter(int handle, uint64_t filter, bool xtd)
{
    assert(IS_HANDLE_VALID(handle));    // just to make sure

    /* the acceptance code and mask are coded together in a 64-bit value, each of them using 4 bytes
     * the acceptance code is in the most significant bytes, the mask in the least significant bytes
     */
    uint32_t code = (uint32_t)((filter >> 32)) & (uint32_t)(xtd ? CAN_MAX_XTD_ID : CAN_MAX_STD_ID);
    uint32_t mask = (uint32_t)((filter >>  0)) & (uint32_t)(xtd ? CAN_MAX_XTD_ID : CAN_MAX_STD_ID);

    /* store the acceptance filter values:
     * a) they can not be set when the controller has been started
     * b) they cannot be read directly from the controller
     * c) SJA1000 has only one pair of code and mask registers
     */
    if (!xtd) {
        can[handle].filter.std.code = code;
        can[handle].filter.std.mask = mask;
        can[handle].filter.xtd.code = FILTER_XTD_CODE;
        can[handle].filter.xtd.mask = FILTER_XTD_MASK;
        // determine the ACn and AMn register
        can[handle].filter.sja1000.code = (uint32_t)(code << 5);
        can[handle].filter.sja1000.mask = (uint32_t)((~mask & CAN_MAX_STD_ID) << 5) | (uint32_t)0x1FU;
    } else {
        can[handle].filter.std.code = FILTER_STD_CODE;
        can[handle].filter.std.mask = FILTER_STD_MASK;
        can[handle].filter.xtd.code = code;
        can[handle].filter.xtd.mask = mask;
        // determine the ACn and AMn register
        can[handle].filter.sja1000.code = (uint32_t)(code << 3);
        can[handle].filter.sja1000.mask = (uint32_t)((~mask & CAN_MAX_XTD_ID) << 3) | (uint32_t)0x7U;
    }
    return CANERR_NOERROR;
}

static int reset_filter(int handle)
{
    assert(IS_HANDLE_VALID(handle));    // just to make sure

    /* reset the acceptance filter for standard and extended identifier */
    can[handle].filter.std.code = FILTER_STD_CODE;
    can[handle].filter.std.mask = FILTER_STD_MASK;
    can[handle].filter.xtd.code = FILTER_XTD_CODE;
    can[handle].filter.xtd.mask = FILTER_XTD_MASK;
    can[handle].filter.sja1000.code = FILTER_SJA1000_CODE;
    can[handle].filter.sja1000.mask = FILTER_SJA1000_MASK;

    return CANERR_NOERROR;
}

/*  - - - - - -  CAN API V3 properties  - - - - - - - - - - - - - - - - -
 */
static int lib_parameter(uint16_t param, void *value, size_t nbyte)
{
    int rc = CANERR_ILLPARA;            // suppose an invalid parameter
    static int idx_board = EOF;         // actual index in the interface list

    if (value == NULL) {                // check for null-pointer
        if ((param != CANPROP_SET_FIRST_CHANNEL) &&
            (param != CANPROP_SET_NEXT_CHANNEL) &&
            (param != CANPROP_SET_FILTER_RESET))
            return CANERR_NULLPTR;
    }
    // query or modify a CAN library property
    switch (param) {
    case CANPROP_GET_SPEC:              // version of the wrapper specification (uint16_t)
        if (nbyte >= sizeof(uint16_t)) {
            *(uint16_t*)value = (uint16_t)CAN_API_SPEC;
            rc = CANERR_NOERROR;
        }
        break;
    case CANPROP_GET_VERSION:           // version number of the library (uint16_t)
        if (nbyte >= sizeof(uint16_t)) {
            *(uint16_t*)value = ((uint16_t)VERSION_MAJOR << 8)
                              | ((uint16_t)VERSION_MINOR & 0xFu);
            rc = CANERR_NOERROR;
        }
        break;
    case CANPROP_GET_PATCH_NO:          // patch number of the library (uint8_t)
        if (nbyte >= sizeof(uint8_t)) {
            *(uint8_t*)value = (uint8_t)VERSION_PATCH;
            rc = CANERR_NOERROR;
        }
        break;
    case CANPROP_GET_BUILD_NO:          // build number of the library (uint32_t)
        if (nbyte >= sizeof(uint32_t)) {
            *(uint32_t*)value = (uint32_t)VERSION_BUILD;
            rc = CANERR_NOERROR;
        }
        break;
    case CANPROP_GET_LIBRARY_ID:        // library id of the library (int32_t)
        if (nbyte >= sizeof(int32_t)) {
            *(int32_t*)value = (int32_t)LIB_ID;
            rc = CANERR_NOERROR;
        }
        break;
    case CANPROP_GET_LIBRARY_VENDOR:    // vendor name of the library (char[])
        if (nbyte >= 1u) {
            strncpy((char*)value, CAN_API_VENDOR, nbyte);
            ((char*)value)[(nbyte - 1)] = '\0';
            rc = CANERR_NOERROR;
        }
        break;
    case CANPROP_GET_LIBRARY_DLLNAME:   // file name of the library (char[])
        if (nbyte >= 1u) {
            strncpy((char*)value, LIB_DLLNAME, nbyte);
            ((char*)value)[(nbyte - 1)] = '\0';
            rc = CANERR_NOERROR;
        }
        break;
    case CANPROP_GET_DEVICE_VENDOR:     // vendor name of the CAN interface (char[])
        if (nbyte >= 1u) {
            strncpy((char*)value, DEV_VENDOR, nbyte);
            ((char*)value)[(nbyte - 1)] = '\0';
            rc = CANERR_NOERROR;
        }
        break;
    case CANPROP_GET_DEVICE_DLLNAME:    // file name of the CAN interface DLL (char[])
        if (nbyte >= 1u) {
            strncpy((char*)value, DEV_DLLNAME, nbyte);
            ((char*)value)[(nbyte - 1)] = '\0';
            rc = CANERR_NOERROR;
        }
        break;
    case CANPROP_SET_FIRST_CHANNEL:     // set index to the first entry in the interface list (NULL)
        idx_board = 0;
        rc = (can_boards[idx_board].type != EOF) ? CANERR_NOERROR : CANERR_RESOURCE;
        break;
    case CANPROP_SET_NEXT_CHANNEL:      // set index to the next entry in the interface list (NULL)
        if ((0 <= idx_board) && (idx_board < /*CANSIO_BOARDS*/1)) {  // warning: statement with no effect
            if (can_boards[idx_board].type != EOF)
                idx_board++;
            rc = (can_boards[idx_board].type != EOF) ? CANERR_NOERROR : CANERR_RESOURCE;
        }
        else
            rc = CANERR_RESOURCE;
        break;
    case CANPROP_GET_CHANNEL_NO:        // get channel no. at actual index in the interface list (int32_t)
        if (nbyte >= sizeof(int32_t)) {
            if ((0 <= idx_board) && (idx_board < /*CANSIO_BOARDS*/1) &&  // warning: statement with no effect
                (can_boards[idx_board].type != EOF)) {
                *(int32_t*)value = (int32_t)can_boards[idx_board].type;
                rc = CANERR_NOERROR;
            }
            else
                rc = CANERR_RESOURCE;
        }
        break;
    case CANPROP_GET_CHANNEL_NAME:      // get channel name at actual index in the interface list (char[])
        if (nbyte >= 1u) {
            if ((0 <= idx_board) && (idx_board < /*CANSIO_BOARDS*/1) &&  // warning: statement with no effect
                (can_boards[idx_board].type != EOF)) {
                strncpy((char*)value, can_boards[idx_board].name, nbyte);
                ((char*)value)[(nbyte - 1)] = '\0';
                rc = CANERR_NOERROR;
            }
            else
                rc = CANERR_RESOURCE;
        }
        break;
    case CANPROP_GET_CHANNEL_DLLNAME:   // get file name of the DLL at actual index in the interface list (char[])
        if (nbyte >= 1u) {
            if ((0 <= idx_board) && (idx_board < /*CANSIO_BOARDS*/1) &&  // warning: statement with no effect
                (can_boards[idx_board].type != EOF)) {
                strncpy((char*)value, DEV_DLLNAME, nbyte);
                ((char*)value)[(nbyte - 1)] = '\0';
                rc = CANERR_NOERROR;
            }
            else
                rc = CANERR_RESOURCE;
        }
        break;
    case CANPROP_GET_CHANNEL_VENDOR_ID: // get library id at actual index in the interface list (int32_t)
        if (nbyte >= sizeof(int32_t)) {
            if ((0 <= idx_board) && (idx_board < /*CANSIO_BOARDS*/1) &&  // warning: statement with no effect
                (can_boards[idx_board].type != EOF)) {
                *(int32_t*)value = (int32_t)LIB_ID;
                rc = CANERR_NOERROR;
            }
            else
                rc = CANERR_RESOURCE;
        }
        break;
    case CANPROP_GET_CHANNEL_VENDOR_NAME: // get vendor name at actual index in the interface list (char[])
        if (nbyte >= 1u) {
            if ((0 <= idx_board) && (idx_board < /*CANSIO_BOARDS*/1) &&  // warning: statement with no effect
                (can_boards[idx_board].type != EOF)) {
                strncpy((char*)value, DEV_VENDOR, nbyte);
                ((char*)value)[(nbyte - 1)] = '\0';
                rc = CANERR_NOERROR;
            }
            else
                rc = CANERR_RESOURCE;
        }
        break;
    case CANPROP_GET_DEVICE_TYPE:       // device type of the CAN interface (int32_t)
    case CANPROP_GET_DEVICE_NAME:       // device name of the CAN interface (char[])
    case CANPROP_GET_DEVICE_PARAM:      // device parameter of the CAN interface (char[])
    case CANPROP_GET_OP_CAPABILITY:     // supported operation modes of the CAN controller (uint8_t)
    case CANPROP_GET_OP_MODE:           // active operation mode of the CAN controller (uint8_t)
    case CANPROP_GET_BITRATE:           // active bit-rate of the CAN controller (can_bitrate_t)
    case CANPROP_GET_SPEED:             // active bus speed of the CAN controller (can_speed_t)
    case CANPROP_GET_STATUS:            // current status register of the CAN controller (uint8_t)
    case CANPROP_GET_BUSLOAD:           // current bus load of the CAN controller (uint16_t)
    case CANPROP_GET_NUM_CHANNELS:      // numbers of CAN channels on the CAN interface (uint8_t)
    case CANPROP_GET_CAN_CHANNEL:       // active CAN channel on the CAN interface (uint8_t)
    case CANPROP_GET_CAN_CLOCK:         // frequency of the CAN controller clock in [Hz] (int32_t)
    case CANPROP_GET_TX_COUNTER:        // total number of sent messages (uint64_t)
    case CANPROP_GET_RX_COUNTER:        // total number of reveiced messages (uint64_t)
    case CANPROP_GET_ERR_COUNTER:       // total number of reveiced error frames (uint64_t)
    case CANPROP_GET_RCV_QUEUE_SIZE:    // maximum number of message the receive queue can hold (uint32_t)
    case CANPROP_GET_RCV_QUEUE_HIGH:    // maximum number of message the receive queue has hold (uint32_t)
    case CANPROP_GET_RCV_QUEUE_OVFL:    // overflow counter of the receive queue (uint64_t)
    case CANPROP_GET_FILTER_11BIT:      // acceptance filter code and mask for 11-bit identifier (uint64_t)
    case CANPROP_GET_FILTER_29BIT:      // acceptance filter code and mask for 29-bit identifier (uint64_t)
    case CANPROP_SET_FILTER_11BIT:      // set value for acceptance filter code and mask for 11-bit identifier (uint64_t)
    case CANPROP_SET_FILTER_29BIT:      // set value for acceptance filter code and mask for 29-bit identifier (uint64_t)
    case CANPROP_SET_FILTER_RESET:      // reset acceptance filter code and mask to default values (NULL)
        // note: a device parameter requires a valid handle.
        if (!init)
            rc = CANERR_NOTINIT;
        else
            rc = CANERR_HANDLE;
        break;
    default:
        rc = CANERR_NOTSUPP;
        break;
    }
    return rc;
}

static int drv_parameter(int handle, uint16_t param, void *value, size_t nbyte)
{
    int rc = CANERR_ILLPARA;            // suppose an invalid parameter
    can_bitrate_t bitrate;              // bit-rate settings
    can_speed_t speed;                  // current bus speed
    uint8_t status = 0u;                // status register
    uint8_t load = 0u;                  // bus load
    uint8_t version_no = 0x00u;         // version number (8-bit)
    uint32_t serial_no = 0x00000000u;   // serial number (32-bit)

    assert(IS_HANDLE_VALID(handle));    // just to make sure

    if (value == NULL) {                // check for null-pointer
        if ((param != CANPROP_SET_FIRST_CHANNEL) &&
            (param != CANPROP_SET_NEXT_CHANNEL) &&
            (param != CANPROP_SET_FILTER_RESET))
            return CANERR_NULLPTR;
    }
    // query or modify a CAN interface property
    switch (param) {
    case CANPROP_GET_DEVICE_TYPE:       // device type of the CAN interface (int32_t)
        if (nbyte >= sizeof(int32_t)) {
            *(int32_t*)value = (int32_t)can[handle].attr.protocol;
            rc = CANERR_NOERROR;
        }
        break;
    case CANPROP_GET_DEVICE_NAME:       // device name of the CAN interface (char[])
        if (nbyte >= 1u) {
            strncpy((char*)value, can[handle].name, nbyte);
            ((char*)value)[(nbyte - 1)] = '\0';
            rc = CANERR_NOERROR;
        }
        break;
    case CANPROP_GET_DEVICE_VENDOR:     // vendor name of the CAN interface (char[])
        if (nbyte >= 1u) {
            strncpy((char*)value, DEV_VENDOR, nbyte);
            ((char*)value)[(nbyte - 1)] = '\0';
            rc = CANERR_NOERROR;
        }
        break;
    case CANPROP_GET_DEVICE_DLLNAME:    // file name of the CAN interface DLL (char[])
        if (nbyte >= 1u) {
            strncpy((char*)value, DEV_DLLNAME, nbyte);
            ((char*)value)[(nbyte - 1)] = '\0';
            rc = CANERR_NOERROR;
        }
        break;
    case CANPROP_GET_DEVICE_PARAM:      // device parameter of the CAN interface (can_sio_param_t)
        if (nbyte >= sizeof(can_sio_param_t)) {
            ((can_sio_param_t*)value)->name = (char*)can[handle].name;
            ((can_sio_param_t*)value)->attr.baudrate = can[handle].attr.baudrate;
            ((can_sio_param_t*)value)->attr.bytesize = can[handle].attr.bytesize;
            ((can_sio_param_t*)value)->attr.parity = can[handle].attr.parity;
            ((can_sio_param_t*)value)->attr.stopbits = can[handle].attr.stopbits;
            ((can_sio_param_t*)value)->attr.protocol = can[handle].attr.protocol;
            rc = CANERR_NOERROR;
        }
        break;
    case CANPROP_GET_OP_CAPABILITY:     // supported operation modes of the CAN controller (uint8_t)
        if (nbyte >= sizeof(uint8_t)) {
            *(uint8_t*)value = (uint8_t)SUPPORTED_OP_MODE;
            rc = CANERR_NOERROR;
        }
        break;
    case CANPROP_GET_OP_MODE:           // active operation mode of the CAN controller (uint8_t)
        if (nbyte >= sizeof(uint8_t)) {
            *(uint8_t*)value = (uint8_t)can[handle].mode.byte;
            rc = CANERR_NOERROR;
        }
        break;
    case CANPROP_GET_BITRATE:           // active bit-rate of the CAN controller (can_bitrate_t)
        if (nbyte >= sizeof(can_bitrate_t)) {
            if (((rc = can_bitrate(handle, &bitrate, NULL)) == CANERR_NOERROR) || (rc == CANERR_OFFLINE)) {
                memcpy(value, &bitrate, sizeof(can_bitrate_t));
                rc = CANERR_NOERROR;
            }
        }
        break;
    case CANPROP_GET_SPEED:             // active bus speed of the CAN controller (can_speed_t)
        if (nbyte >= sizeof(can_speed_t)) {
            if (((rc = can_bitrate(handle, NULL, &speed)) == CANERR_NOERROR) || (rc == CANERR_OFFLINE)) {
                memcpy(value, &speed, sizeof(can_speed_t));
                rc = CANERR_NOERROR;
            }
        }
        break;
    case CANPROP_GET_STATUS:            // current status register of the CAN controller (uint8_t)
        if (nbyte >= sizeof(uint8_t)) {
            if ((rc = can_status(handle, &status)) == CANERR_NOERROR) {
                *(uint8_t*)value = (uint8_t)status;
                rc = CANERR_NOERROR;
            }
        }
        break;
    case CANPROP_GET_BUSLOAD:           // current bus load of the CAN controller (uint16_t)
        if (nbyte >= sizeof(uint8_t)) {
            if (((rc = can_busload(handle, &load, NULL)) == CANERR_NOERROR) || (rc == CANERR_OFFLINE)) {
                if (nbyte > sizeof(uint8_t))
                    *(uint16_t*)value = (uint16_t)load * 100u;  // 0..10000 ==> 0.00%..100.00%
                else
                    *(uint8_t*)value = (uint8_t)load;           // 0..100% (note: legacy resolution)
                rc = CANERR_NOERROR;
            }
        }
        break;
    case CANPROP_GET_NUM_CHANNELS:      // numbers of CAN channels on the CAN interface (uint8_t)
        if (nbyte >= sizeof(uint8_t)) {
            *(uint8_t*)value = (uint8_t)1;  // FIXME: replace magic number
            rc = CANERR_NOERROR;
        }
        break;
    case CANPROP_GET_CAN_CHANNEL:       // active CAN channel on the CAN interface (uint8_t)
        if (nbyte >= sizeof(uint8_t)) {
            *(uint8_t*)value = (uint8_t)0;  // FIXME: replace magic number
            rc = CANERR_NOERROR;
        }
        break;
    case CANPROP_GET_CAN_CLOCK:         // frequency of the CAN controller clock in [Hz] (int32_t)
        if (nbyte >= sizeof(int32_t)) {
            *(int32_t*)value = (int32_t)CAN_CLOCK_FREQUENCY;
            rc = CANERR_NOERROR;
        }
        break;
    case CANPROP_GET_TX_COUNTER:        // total number of sent messages (uint64_t)
        if (nbyte >= sizeof(uint64_t)) {
            *(uint64_t*)value = (uint64_t)can[handle].counters.tx;
            rc = CANERR_NOERROR;
        }
        break;
    case CANPROP_GET_RX_COUNTER:        // total number of reveiced messages (uint64_t)
        if (nbyte >= sizeof(uint64_t)) {
            *(uint64_t*)value = (uint64_t)can[handle].counters.rx;
            rc = CANERR_NOERROR;
        }
        break;
    case CANPROP_GET_ERR_COUNTER:       // total number of reveiced error frames (uint64_t)
        if (nbyte >= sizeof(uint64_t)) {
            *(uint64_t*)value = (uint64_t)can[handle].counters.err;
            rc = CANERR_NOERROR;
        }
        break;
    case CANPROP_GET_RCV_QUEUE_SIZE:    // maximum number of message the receive queue can hold (uint32_t)
    case CANPROP_GET_RCV_QUEUE_HIGH:    // maximum number of message the receive queue has hold (uint32_t)
    case CANPROP_GET_RCV_QUEUE_OVFL:    // overflow counter of the receive queue (uint64_t)
        // note: cannot be determined
        rc = CANERR_NOTSUPP;
        break;
    case CANPROP_GET_FILTER_11BIT:      // acceptance filter code and mask for 11-bit identifier (uint64_t)
        if (nbyte >= sizeof(uint64_t)) {
            *(uint64_t*)value = ((uint64_t)can[handle].filter.std.code << 32)
                              | ((uint64_t)can[handle].filter.std.mask);
            rc = CANERR_NOERROR;
        }
        break;
    case CANPROP_GET_FILTER_29BIT:      // acceptance filter code and mask for 29-bit identifier (uint64_t)
        if (nbyte >= sizeof(uint64_t)) {
            *(uint64_t*)value = ((uint64_t)can[handle].filter.xtd.code << 32)
                              | ((uint64_t)can[handle].filter.xtd.mask);
            rc = CANERR_NOERROR;
        }
        break;
    case CANPROP_SET_FILTER_11BIT:      // set value for acceptance filter code and mask for 11-bit identifier (uint64_t)
        if (nbyte >= sizeof(uint64_t)) {
            if (!(*(uint64_t*)value & ~FILTER_STD_VALID_MASK)) {
                // note: code and mask must not exceed 11-bit identifier
                if (can[handle].status.can_stopped) {
                    // note: set filter only if the CAN controller is in INIT mode
                    rc= set_filter(handle, *(uint64_t*)value, false);
                }
                else
                    rc = CANERR_ONLINE;
            }
            else
                rc = CANERR_ILLPARA;
        }
        break;
    case CANPROP_SET_FILTER_29BIT:      // set value for acceptance filter code and mask for 29-bit identifier (uint64_t)
        if (nbyte >= sizeof(uint64_t)) {
            if (!(*(uint64_t*)value & ~FILTER_XTD_VALID_MASK) && !can[handle].mode.nxtd) {
                // note: code and mask must not exceed 29-bit identifier and
                //       extended frame format mode must not be suppressed
                if (can[handle].status.can_stopped) {
                    // note: set filter only if the CAN controller is in INIT mode
                    rc = set_filter(handle, *(uint64_t*)value, true);
                }
                else
                    rc = CANERR_ONLINE;
            }
            else
                rc = CANERR_ILLPARA;
        }
        break;
    case CANPROP_SET_FILTER_RESET:      // reset acceptance filter code and mask to default values (NULL)
        if (can[handle].status.can_stopped) {
            // note: reset filter only if the CAN controller is in INIT mode
            rc = reset_filter(handle);
        }
        else
            rc = CANERR_ONLINE;
        break;
    /* vendor-specific properties */
    case (CANPROP_GET_VENDOR_PROP + SLCAN_SERIAL_NUMBER):       // serial no (uint32_t)
        if (nbyte >= sizeof(uint32_t)) {
            if ((rc = slcan_serial_number(can[handle].port, &serial_no)) == 0) {
                *(uint32_t*)value = (uint32_t)serial_no;
                rc = CANERR_NOERROR;
            }
            else if (can[handle].attr.protocol == CANSIO_CANABLE) {
                *(uint32_t*)value = (uint32_t)0x99999999;
                rc = CANERR_NOERROR;
            }
            else {
                rc = slcan_error(rc);
            }
        }
        break;
    case (CANPROP_GET_VENDOR_PROP + SLCAN_HARDWARE_VERSION):    // hardware version (uint16_t)
        if (nbyte >= sizeof(uint16_t)) {
            if ((rc = slcan_version_number(can[handle].port, &version_no, NULL)) == 0) {
                *(uint16_t*)value = ((uint16_t)(version_no & 0xF0U) << 4)
                                  | ((uint16_t)version_no & 0xFU);
                rc = CANERR_NOERROR;
            }
            else {
                rc = slcan_error(rc);
            }
        }
        break;
    case (CANPROP_GET_VENDOR_PROP + SLCAN_FIRMWARE_VERSION):    // firmware version (uint16_t)
        if (nbyte >= sizeof(uint16_t)) {
            if ((rc = slcan_version_number(can[handle].port, NULL, &version_no)) == 0) {
                *(uint16_t*)value = ((uint16_t)(version_no & 0xF0U) << 4)
                                  | ((uint16_t)version_no & 0xFU);
                rc = CANERR_NOERROR;
            }
            else {
                rc = slcan_error(rc);
            }
        }
        break;
    default:
        rc = lib_parameter(param, value, nbyte);   // library properties (see lib_parameter)
        break;
    }
    return rc;
}

/*  -----------  revision control  ---------------------------------------
 */
EXPORT
char *can_version(void)
{
    return (char*)version;
}
/** @}
 */
/*  ----------------------------------------------------------------------
 *  Uwe Vogt,  UV Software,  Chausseestrasse 33 A,  10115 Berlin,  Germany
 *  Tel.: +49-30-46799872,  Fax: +49-30-46799873,  Mobile: +49-170-3801903
 *  E-Mail: uwe.vogt@uv-software.de, Homepage: https://www.uv-software.de/
 */
