//  SPDX-License-Identifier: BSD-2-Clause OR GPL-3.0-or-later
//
//  CAN Interface API, Version 3 (Testing)
//
//  Copyright (c) 2004-2024 Uwe Vogt, UV Software, Berlin (info@uv-software.com)
//  All rights reserved.
//
//  This file is part of CAN API V3.
//
//  CAN API V3 is dual-licensed under the BSD 2-Clause "Simplified" License
//  and under the GNU General Public License v3.0 (or any later version). You
//  can choose between one of them if you use CAN API V3 in whole or in part.
//
//  BSD 2-Clause "Simplified" License:
//  Redistribution and use in source and binary forms, with or without
//  modification, are permitted provided that the following conditions are met:
//  1. Redistributions of source code must retain the above copyright notice, this
//     list of conditions and the following disclaimer.
//  2. Redistributions in binary form must reproduce the above copyright notice,
//     this list of conditions and the following disclaimer in the documentation
//     and/or other materials provided with the distribution.
//
//  CAN API V3 IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
//  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
//  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
//  DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
//  FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
//  DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
//  SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
//  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
//  OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
//  OF CAN API V3, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
//  GNU General Public License v3.0 or later:
//  CAN API V3 is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  CAN API V3 is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with CAN API V3.  If not, see <https://www.gnu.org/licenses/>.
//
#import "Settings.h"
#import "can_api.h"
#import <XCTest/XCTest.h>

#ifndef CAN_FD_SUPPORTED
#define CAN_FD_SUPPORTED  FEATURE_SUPPORTED
#warning CAN_FD_SUPPORTED not set, default=FEATURE_SUPPORTED
#endif

#ifndef FEATURE_WRITE_ACKNOWLEDGED
#define FEATURE_WRITE_ACKNOWLEDGED  FEATURE_UNSUPPORTED
#warning FEATURE_WRITE_ACKNOWLEDGED not set, default=FEATURE_UNSUPPORTED
#endif

#ifndef FEATURE_STATUS_BIT_QUE_OVR
#define FEATURE_STATUS_BIT_QUE_OVR  FEATURE_SUPPORTED
#warning FEATURE_STATUS_BIT_QUE_OVR not set, default=FEATURE_SUPPORTED
#endif

#define TIMESTAMP_DELAY_10MS  10U
#define TIMESTAMP_DELAY_7MS   7U
#define TIMESTAMP_DELAY_5MS   5U
#define TIMESTAMP_DELAY_2MS   2U
#define TIMESTAMP_DELAY_1MS   1U
#define TIMESTAMP_DELAY_0MS   0U

@interface test_can_read : XCTestCase

@end

@implementation test_can_read

- (void)setUp {
    // Put setup code here. This method is called before the invocation of each test method in the class.
}

- (void)tearDown {
    (void)can_exit(CANKILL_ALL);
}

// @xctest TC04.0: Read a CAN message (sunnyday scenario)
//
// @expected: CANERR_NOERROR
//
// - (void)testSunnydayScenario {
//     // @test:
//     // @todo: insert coin here
//     // @end.
// }

// @xctest TC04.1: Read a CAN message with invalid channel handle(s)
//
// @expected: CANERR_HANDLE
//
- (void)testWithInvalidHandle {
    can_bitrate_t bitrate = { TEST_BTRINDEX };
    can_status_t status = { CANSTAT_RESET };
    can_message_t message = {};
    int handle = INVALID_HANDLE;
    int rc = CANERR_FATAL;
    // @pre:
    // @- initialize DUT1 with configured settings
    handle = can_init(DUT1, TEST_CANMODE, TEST_PARAM(PAR1));
    XCTAssertLessThanOrEqual(0, handle);
    // @- get status of DUT1 and check to be in INIT state
    rc = can_status(handle, &status.byte);
    XCTAssertEqual(CANERR_NOERROR, rc);
    XCTAssertTrue(status.can_stopped);
    // @test:
    // @- try to read a message from DUT1 with invalid handle -1
    rc = can_read(INVALID_HANDLE, &message, 0U);
    XCTAssertEqual(CANERR_HANDLE, rc);
    // @- get status of DUT1 and check to be in INIT state
    rc = can_status(handle, &status.byte);
    XCTAssertEqual(CANERR_NOERROR, rc);
    XCTAssertTrue(status.can_stopped);
    // @- start DUT1 with configured bit-rate settings
    rc = can_start(handle, &bitrate);
    XCTAssertEqual(CANERR_NOERROR, rc);
    // @- try to read a message from DUT1 with invalid handle INT32_MIN
    rc = can_read(INT32_MIN, &message, 0U);
    XCTAssertEqual(CANERR_HANDLE, rc);
    // @- get status of DUT1 and check to be in RUNNING state
    rc = can_status(handle, &status.byte);
    XCTAssertEqual(CANERR_NOERROR, rc);
    XCTAssertFalse(status.can_stopped);
    // @- send and receive some frames to/from DUT2 (optional)
#if (SEND_TEST_FRAMES != 0)
    CTester tester;
    XCTAssertEqual(TEST_FRAMES, tester.SendSomeFrames(handle, DUT2, TEST_FRAMES));
    XCTAssertEqual(TEST_FRAMES, tester.ReceiveSomeFrames(handle, DUT2, TEST_FRAMES));
    // @- get status of DUT1 and check to be in RUNNING state
    rc = can_status(handle, &status.byte);
    XCTAssertEqual(CANERR_NOERROR, rc);
    XCTAssertFalse(status.can_stopped);
#endif
    // @- stop/reset DUT1
    rc = can_reset(handle);
    XCTAssertEqual(CANERR_NOERROR, rc);
    // @- try to read a message from DUT1 with invalid handle INT32_MAX
    rc = can_read(INT32_MAX, &message, 0U);
    XCTAssertEqual(CANERR_HANDLE, rc);
    // @- get status of DUT1 and check to be in INIT state
    rc = can_status(handle, &status.byte);
    XCTAssertEqual(CANERR_NOERROR, rc);
    XCTAssertTrue(status.can_stopped);
    // @post:
    // @- tear down DUT1
    rc = can_exit(handle);
    XCTAssertEqual(CANERR_NOERROR, rc);
    // @end.
}

// @xctest TC04.2: Give a NULL pointer as argument for parameter 'message'
//
// @expected: CANERR_NULLPTR
//
- (void)testWithNullPointerForMessage {
    can_bitrate_t bitrate = { TEST_BTRINDEX };
    can_status_t status = { CANSTAT_RESET };
    int handle = INVALID_HANDLE;
    int rc = CANERR_FATAL;
    // @pre:
    // @- initialize DUT1 with configured settings
    handle = can_init(DUT1, TEST_CANMODE, TEST_PARAM(PAR1));
    XCTAssertLessThanOrEqual(0, handle);
    // @- get status of DUT1 and check to be in INIT state
    rc = can_status(handle, &status.byte);
    XCTAssertEqual(CANERR_NOERROR, rc);
    XCTAssertTrue(status.can_stopped);
    // @- start DUT1 with configured bit-rate settings
    rc = can_start(handle, &bitrate);
    XCTAssertEqual(CANERR_NOERROR, rc);
    // @test:
    // @- read a message from DUT1 with NULL for parameter 'message'
    rc = can_read(handle, NULL, 0U);
    XCTAssertEqual(CANERR_NULLPTR, rc);
    // @- get status of DUT1 and check to be in RUNNING state
    rc = can_status(handle, &status.byte);
    XCTAssertEqual(CANERR_NOERROR, rc);
    XCTAssertFalse(status.can_stopped);
    // @post:
    // @- send and receive some frames to/from DUT2 (optional)
#if (SEND_TEST_FRAMES != 0)
    CTester tester;
    XCTAssertEqual(TEST_FRAMES, tester.SendSomeFrames(handle, DUT2, TEST_FRAMES));
    XCTAssertEqual(TEST_FRAMES, tester.ReceiveSomeFrames(handle, DUT2, TEST_FRAMES));
    // @- get status of DUT1 and check to be in RUNNING state
    rc = can_status(handle, &status.byte);
    XCTAssertEqual(CANERR_NOERROR, rc);
    XCTAssertFalse(status.can_stopped);
#endif
    // @- stop/reset DUT1
    rc = can_reset(handle);
    XCTAssertEqual(CANERR_NOERROR, rc);
    // @- get status of DUT1 and check to be in INIT state
    rc = can_status(handle, &status.byte);
    XCTAssertEqual(CANERR_NOERROR, rc);
    XCTAssertTrue(status.can_stopped);
    // @- tear down DUT1
    rc = can_exit(handle);
    XCTAssertEqual(CANERR_NOERROR, rc);
    // @end.
}

// @xctest TC04.3: Read a CAN message if CAN channel is not initialized
//
// @expected: CANERR_NOTINIT
//
- (void)testIfChannelNotInitialized {
    can_bitrate_t bitrate = { TEST_BTRINDEX };
    can_status_t status = { CANSTAT_RESET };
    can_message_t message = {};
    int handle = INVALID_HANDLE;
    int rc = CANERR_FATAL;
    // @test:
    // @- try to read a message from DUT1
    rc = can_read(DUT1, &message, 0U);
    XCTAssertEqual(CANERR_NOTINIT, rc);
    // @post:
    // @- initialize DUT1 with configured settings
    handle = can_init(DUT1, TEST_CANMODE, TEST_PARAM(PAR1));
    XCTAssertLessThanOrEqual(0, handle);
    // @- get status of DUT1 and check to be in INIT state
    rc = can_status(handle, &status.byte);
    XCTAssertEqual(CANERR_NOERROR, rc);
    XCTAssertTrue(status.can_stopped);
    // @- start DUT1 with configured bit-rate settings
    rc = can_start(handle, &bitrate);
    XCTAssertEqual(CANERR_NOERROR, rc);
    // @- get status of DUT1 and check to be in RUNNING state
    rc = can_status(handle, &status.byte);
    XCTAssertEqual(CANERR_NOERROR, rc);
    XCTAssertFalse(status.can_stopped);
    // @- send and receive some frames to/from DUT2 (optional)
#if (SEND_TEST_FRAMES != 0)
    CTester tester;
    XCTAssertEqual(TEST_FRAMES, tester.SendSomeFrames(handle, DUT2, TEST_FRAMES));
    XCTAssertEqual(TEST_FRAMES, tester.ReceiveSomeFrames(handle, DUT2, TEST_FRAMES));
    // @- get status of DUT1 and check to be in RUNNING state
    rc = can_status(handle, &status.byte);
    XCTAssertEqual(CANERR_NOERROR, rc);
    XCTAssertFalse(status.can_stopped);
#endif
    // @- stop/reset DUT1
    rc = can_reset(handle);
    XCTAssertEqual(CANERR_NOERROR, rc);
    // @- get status of DUT1 and check to be in INIT state
    rc = can_status(handle, &status.byte);
    XCTAssertEqual(CANERR_NOERROR, rc);
    XCTAssertTrue(status.can_stopped);
    // @- tear down DUT1
    rc = can_exit(handle);
    XCTAssertEqual(CANERR_NOERROR, rc);
    // @end.
}

// @xctest TC04.4: Read a CAN message if CAN controller is not started
//
// @expected: CANERR_OFFLINE
//
- (void)testIfControllerNotStarted {
    can_bitrate_t bitrate = { TEST_BTRINDEX };
    can_status_t status = { CANSTAT_RESET };
    can_message_t message = {};
    int handle = INVALID_HANDLE;
    int rc = CANERR_FATAL;
    // @pre:
    // @- initialize DUT1 with configured settings
    handle = can_init(DUT1, TEST_CANMODE, TEST_PARAM(PAR1));
    XCTAssertLessThanOrEqual(0, handle);
    // @- get status of DUT1 and check to be in INIT state
    rc = can_status(handle, &status.byte);
    XCTAssertEqual(CANERR_NOERROR, rc);
    XCTAssertTrue(status.can_stopped);
    // @test:
    // @- try to read a message from DUT1
    rc = can_read(handle, &message, 0U);
    XCTAssertEqual(CANERR_OFFLINE, rc);
    // @- get status of DUT1 and check to be in INIT state
    rc = can_status(handle, &status.byte);
    XCTAssertEqual(CANERR_NOERROR, rc);
    XCTAssertTrue(status.can_stopped);
    // @post:
    // @- start DUT1 with configured bit-rate settings
    rc = can_start(handle, &bitrate);
    XCTAssertEqual(CANERR_NOERROR, rc);
    // @- get status of DUT1 and check to be in RUNNING state
    rc = can_status(handle, &status.byte);
    XCTAssertEqual(CANERR_NOERROR, rc);
    XCTAssertFalse(status.can_stopped);
    // @- send and receive some frames to/from DUT2 (optional)
#if (SEND_TEST_FRAMES != 0)
    CTester tester;
    XCTAssertEqual(TEST_FRAMES, tester.SendSomeFrames(handle, DUT2, TEST_FRAMES));
    XCTAssertEqual(TEST_FRAMES, tester.ReceiveSomeFrames(handle, DUT2, TEST_FRAMES));
    // @- get status of DUT1 and check to be in RUNNING state
    rc = can_status(handle, &status.byte);
    XCTAssertEqual(CANERR_NOERROR, rc);
    XCTAssertFalse(status.can_stopped);
#endif
    // @- stop/reset DUT1
    rc = can_reset(handle);
    XCTAssertEqual(CANERR_NOERROR, rc);
    // @- get status of DUT1 and check to be in INIT state
    rc = can_status(handle, &status.byte);
    XCTAssertEqual(CANERR_NOERROR, rc);
    XCTAssertTrue(status.can_stopped);
    // @- tear down DUT1
    rc = can_exit(handle);
    XCTAssertEqual(CANERR_NOERROR, rc);
    // @end.
}

// @xctest TC04.5: Read a CAN message if CAN controller was previously stopped
//
// @expected: CANERR_OFFLINE
//
- (void)testIfControllerStopped {
    can_bitrate_t bitrate = { TEST_BTRINDEX };
    can_status_t status = { CANSTAT_RESET };
    can_message_t message = {};
    int handle = INVALID_HANDLE;
    int rc = CANERR_FATAL;
    // @pre:
    // @- initialize DUT1 with configured settings
    handle = can_init(DUT1, TEST_CANMODE, TEST_PARAM(PAR1));
    XCTAssertLessThanOrEqual(0, handle);
    // @- get status of DUT1 and check to be in INIT state
    rc = can_status(handle, &status.byte);
    XCTAssertEqual(CANERR_NOERROR, rc);
    XCTAssertTrue(status.can_stopped);
    // @- start DUT1 with configured bit-rate settings
    rc = can_start(handle, &bitrate);
    XCTAssertEqual(CANERR_NOERROR, rc);
    // @- get status of DUT1 and check to be in RUNNING state
    rc = can_status(handle, &status.byte);
    XCTAssertEqual(CANERR_NOERROR, rc);
    XCTAssertFalse(status.can_stopped);
    // @- send and receive some frames to/from DUT2 (optional)
#if (SEND_TEST_FRAMES != 0)
    CTester tester;
    XCTAssertEqual(TEST_FRAMES, tester.SendSomeFrames(handle, DUT2, TEST_FRAMES));
    XCTAssertEqual(TEST_FRAMES, tester.ReceiveSomeFrames(handle, DUT2, TEST_FRAMES));
    // @- get status of DUT1 and check to be in RUNNING state
    rc = can_status(handle, &status.byte);
    XCTAssertEqual(CANERR_NOERROR, rc);
    XCTAssertFalse(status.can_stopped);
#endif
    // @- stop/reset DUT1
    rc = can_reset(handle);
    XCTAssertEqual(CANERR_NOERROR, rc);
    // @test:
    // @- try to read a message from DUT1
    rc = can_read(handle, &message, 0U);
    XCTAssertEqual(CANERR_OFFLINE, rc);
    // @- get status of DUT1 and check to be in INIT state
    rc = can_status(handle, &status.byte);
    XCTAssertEqual(CANERR_NOERROR, rc);
    XCTAssertTrue(status.can_stopped);
    // @post:
    // @- tear down DUT1
    rc = can_exit(handle);
    XCTAssertEqual(CANERR_NOERROR, rc);
    // @end.
}

// @xctest TC04.6: Read a CAN message if CAN channel was previously torn down
//
// @expected: CANERR_NOTINIT
//
- (void)testIfChannelTornDown {
    can_bitrate_t bitrate = { TEST_BTRINDEX };
    can_status_t status = { CANSTAT_RESET };
    can_message_t message = {};
    int handle = INVALID_HANDLE;
    int rc = CANERR_FATAL;
    // @pre:
    // @- initialize DUT1 with configured settings
    handle = can_init(DUT1, TEST_CANMODE, TEST_PARAM(PAR1));
    XCTAssertLessThanOrEqual(0, handle);
    // @- get status of DUT1 and check to be in INIT state
    rc = can_status(handle, &status.byte);
    XCTAssertEqual(CANERR_NOERROR, rc);
    XCTAssertTrue(status.can_stopped);
    // @- start DUT1 with configured bit-rate settings
    rc = can_start(handle, &bitrate);
    XCTAssertEqual(CANERR_NOERROR, rc);
    // @- get status of DUT1 and check to be in RUNNING state
    rc = can_status(handle, &status.byte);
    XCTAssertEqual(CANERR_NOERROR, rc);
    XCTAssertFalse(status.can_stopped);
    // @- send and receive some frames to/from DUT2 (optional)
#if (SEND_TEST_FRAMES != 0)
    CTester tester;
    XCTAssertEqual(TEST_FRAMES, tester.SendSomeFrames(handle, DUT2, TEST_FRAMES));
    XCTAssertEqual(TEST_FRAMES, tester.ReceiveSomeFrames(handle, DUT2, TEST_FRAMES));
    // @- get status of DUT1 and check to be in RUNNING state
    rc = can_status(handle, &status.byte);
    XCTAssertEqual(CANERR_NOERROR, rc);
    XCTAssertFalse(status.can_stopped);
#endif
    // @- stop/reset DUT1
    rc = can_reset(handle);
    XCTAssertEqual(CANERR_NOERROR, rc);
    // @- get status of DUT1 and check to be in INIT state
    rc = can_status(handle, &status.byte);
    XCTAssertEqual(CANERR_NOERROR, rc);
    XCTAssertTrue(status.can_stopped);
    // @- tear down DUT1
    rc = can_exit(handle);
    XCTAssertEqual(CANERR_NOERROR, rc);
    // @test:
    // @- try to read a message from DUT1
    rc = can_read(handle, &message, 0U);
    XCTAssertEqual(CANERR_NOTINIT, rc);
    // @end.
}

// @xctest TC04.7: Read a CAN message if receive queue is empty
//
// @expected: CANERR_RX_EMPTY
//
- (void)testIfReceiveQueueEmpty {
    can_bitrate_t bitrate = { TEST_BTRINDEX };
    can_status_t status = { CANSTAT_RESET };
    can_message_t message = {};
    int handle = INVALID_HANDLE;
    int rc = CANERR_FATAL;
    // @pre:
    // @- initialize DUT1 with configured settings
    handle = can_init(DUT1, TEST_CANMODE, TEST_PARAM(PAR1));
    XCTAssertLessThanOrEqual(0, handle);
    // @- get status of DUT1 and check to be in INIT state
    rc = can_status(handle, &status.byte);
    XCTAssertEqual(CANERR_NOERROR, rc);
    XCTAssertTrue(status.can_stopped);
    // @- start DUT1 with configured bit-rate settings
    rc = can_start(handle, &bitrate);
    XCTAssertEqual(CANERR_NOERROR, rc);
    // @test:
    // @- try to read a message from DUT1 when there in none
    rc = can_read(handle, &message, 0U);
    XCTAssertEqual(CANERR_RX_EMPTY, rc);
    // @- get status of DUT1 and check to be in RUNNING state
    rc = can_status(handle, &status.byte);
    XCTAssertEqual(CANERR_NOERROR, rc);
    XCTAssertFalse(status.can_stopped);
    // @- check if bit CANSTAT_RX_EMPTY is set in status register
    XCTAssertTrue(status.receiver_empty);
    // @- send and receive some frames to/from DUT2 (optional)
#if (SEND_TEST_FRAMES != 0)
    CTester tester;
    XCTAssertEqual(TEST_FRAMES, tester.SendSomeFrames(handle, DUT2, TEST_FRAMES));
    XCTAssertEqual(TEST_FRAMES, tester.ReceiveSomeFrames(handle, DUT2, TEST_FRAMES));
    // @- try to read a message from DUT1 when there in none
    rc = can_read(handle, &message, 0U);
    XCTAssertEqual(CANERR_RX_EMPTY, rc);
    // @- get status of DUT1 and check to be in RUNNING state
    rc = can_status(handle, &status.byte);
    XCTAssertEqual(CANERR_NOERROR, rc);
    XCTAssertFalse(status.can_stopped);
    // @- check if bit CANSTAT_RX_EMPTY is set in status register
    XCTAssertTrue(status.receiver_empty);
#endif
    // @post:
    // @- stop/reset DUT1
    rc = can_reset(handle);
    XCTAssertEqual(CANERR_NOERROR, rc);
    // @- get status of DUT1 and check to be in INIT state
    rc = can_status(handle, &status.byte);
    XCTAssertEqual(CANERR_NOERROR, rc);
    XCTAssertTrue(status.can_stopped);
    // @- tear down DUT1
    rc = can_exit(handle);
    XCTAssertEqual(CANERR_NOERROR, rc);
    // @end.
}

// @xctest TC04.8: Read a CAN message from receive queue after overrun
//
// @expected: CANERR_NOERROR, but status bit 'queue_overrun' = 1
//
- (void)testIfReceiveQueueFull {
    can_bitrate_t bitrate = { TEST_BTRINDEX };
    can_status_t status = { CANSTAT_RESET };
    can_message_t message1 = {};
    can_message_t message2 = {};
    can_mode_t mode = { TEST_CANMODE };
    int handle1 = INVALID_HANDLE;
    int handle2 = INVALID_HANDLE;
    int rc = CANERR_FATAL;
    int i;
    // transmit message
    message2.id = 0x400U;
    message2.fdf = mode.fdoe ? 1 : 0;
    message2.brs = mode.brse ? 1 : 0;
    message2.xtd = 0;
    message2.rtr = 0;
    message2.esi = 0;
    message2.sts = 0;
    message2.dlc = mode.fdoe ? CANFD_MAX_DLC : CAN_MAX_DLC;
    memset(message2.data, 0, CANFD_MAX_LEN);
    // @pre:
    // @- initialize DUT1 with configured settings
    handle1 = can_init(DUT1, mode.byte, TEST_PARAM(PAR1));
    XCTAssertLessThanOrEqual(0, handle1);
    // @- get status of DUT1 and check to be in INIT state
    rc = can_status(handle1, &status.byte);
    XCTAssertEqual(CANERR_NOERROR, rc);
    XCTAssertTrue(status.can_stopped);
    // @- initialize DUT2 with configured settings
    handle2 = can_init(DUT2, mode.byte, TEST_PARAM(PAR2));
    XCTAssertLessThanOrEqual(0, handle1);
    // @- get status of DUT2 and check to be in INIT state
    rc = can_status(handle2, &status.byte);
    XCTAssertEqual(CANERR_NOERROR, rc);
    XCTAssertTrue(status.can_stopped);
    // @- start DUT1 with configured bit-rate settings
    rc = can_start(handle1, &bitrate);
    XCTAssertEqual(CANERR_NOERROR, rc);
    // @- get status of DUT1 and check to be in RUNNING state
    rc = can_status(handle1, &status.byte);
    XCTAssertEqual(CANERR_NOERROR, rc);
    XCTAssertFalse(status.can_stopped);
    // @- start DUT2 with configured bit-rate settings
    rc = can_start(handle2, &bitrate);
    XCTAssertEqual(CANERR_NOERROR, rc);
    // @- get status of DUT2 and check to be in RUNNING state
    rc = can_status(handle2, &status.byte);
    XCTAssertEqual(CANERR_NOERROR, rc);
    XCTAssertFalse(status.can_stopped);
    // @issue(PeakCAN): a delay of 100ms is required here
    PCBUSB_INIT_DELAY();
    // @test:
    NSLog(@"Be patient...");
    // @- spam the receive queue of DUT1 with one message too much
    for (i = 0; i <= TEST_QUEUE_FULL; i++) {
        message2.data[0] = (uint8_t)((uint64_t)i >> 0);
        message2.data[1] = (uint8_t)((uint64_t)i >> 8);
        message2.data[2] = (uint8_t)((uint64_t)i >> 16);
        message2.data[3] = (uint8_t)((uint64_t)i >> 24);
        message2.data[4] = (uint8_t)((uint64_t)i >> 32);
        message2.data[5] = (uint8_t)((uint64_t)i >> 40);
        message2.data[6] = (uint8_t)((uint64_t)i >> 48);
        message2.data[7] = (uint8_t)((uint64_t)i >> 56);
        do {
            rc = can_write(handle2, &message2, 1000U);
        } while (CANERR_TX_BUSY == rc);
        XCTAssertEqual(CANERR_NOERROR, rc);
        if (CANERR_NOERROR != rc)
            break;
    }
#if (FEATURE_WRITE_ACKNOWLEDGED != FEATURE_SUPPORTED)
    // @note: a delay (after burner) to guarantee that all CAN messages are really sent
    // @      is required when messages are not acknowledged by the CAN controller.
    CTimer::Delay(TEST_AFTER_BURNER*CTimer::MSEC);
    // @note: the delay depends on the bit-rate (set TEST_AFTER_BURNER in "Settings.h").
#else
    CTimer::Delay(100U*CTimer::MSEC);  // [2022-05-28] let the queue finally overflow!
#endif
    NSLog(@"%d frame(s) sent", i);
    // @- read a message from DUT1 (there should be at least one)
    rc = can_read(handle1, &message1, 0U);
    XCTAssertEqual(CANERR_NOERROR, rc);
    // @- get status of DUT1 and check to be in RUNNING state
    rc = can_status(handle1, &status.byte);
    XCTAssertEqual(CANERR_NOERROR, rc);
    XCTAssertFalse(status.can_stopped);
    // @- check if bit CANSTAT_QUE_OVR is set in status register
#if (FEATURE_STATUS_BIT_QUE_OVR == FEATURE_SUPPORTED)
    XCTAssertTrue(status.queue_overrun);
#endif
    // @- stop/reset DUT1 (this should clear the receive queue)
    rc = can_reset(handle1);
    XCTAssertEqual(CANERR_NOERROR, rc);
    // @- start DUT1 with configured bit-rate settings again
    rc = can_start(handle1, &bitrate);
    XCTAssertEqual(CANERR_NOERROR, rc);
    // @- get status of DUT1 and check to be in RUNNING state
    rc = can_status(handle1, &status.byte);
    XCTAssertEqual(CANERR_NOERROR, rc);
    XCTAssertFalse(status.can_stopped);
    // @- try to read a message from DUT1 (now there should none)
    rc = can_read(handle1, &message1, 0U);
    XCTAssertEqual(CANERR_RX_EMPTY, rc);
    // @- get status of DUT1 and check to be in RUNNING state
    rc = can_status(handle1, &status.byte);
    XCTAssertEqual(CANERR_NOERROR, rc);
    XCTAssertFalse(status.can_stopped);
    // @- check if bit CANSTAT_QUE_OVR is cleared in status register
    XCTAssertFalse(status.queue_overrun);
    // @post:
    // @- stop/reset DUT1
    rc = can_reset(handle1);
    XCTAssertEqual(CANERR_NOERROR, rc);
    // @- get status of DUT1 and check to be in INIT state
    rc = can_status(handle1, &status.byte);
    XCTAssertEqual(CANERR_NOERROR, rc);
    XCTAssertTrue(status.can_stopped);
    // @- tear down DUT1
    rc = can_exit(handle1);
    XCTAssertEqual(CANERR_NOERROR, rc);
    // @- tear down DUT2
    rc = can_exit(handle2);
    XCTAssertEqual(CANERR_NOERROR, rc);
    // @end.
}

// @xctest TC04.9: Read a CAN message after message lost
//
// @expected: CANERR_NOERROR but status flag 'message_lost' set
//
// - (void)testIfMessageLost {
//     // @todo: How to loose a message?
//     // @end.
// }

// @xctest TC04.10: Measure the time-stamp accuracy of the device
//
// @expected: CANERR_NOERROR
//
#if (SERIAL_CAN_SUPPORTED == 0)
- (void)testMeasureTimestampAccuracy {
    can_bitrate_t bitrate = { TEST_BTRINDEX };
    can_status_t status = { CANSTAT_RESET };
    can_message_t message1 = {};
    can_message_t message2 = {};
    can_mode_t mode = { TEST_CANMODE };
    int handle1 = INVALID_HANDLE;
    int handle2 = INVALID_HANDLE;
    int rc = CANERR_FATAL;
    int i, n;
    
    CTimer timer = CTimer();
    CTimer delay = CTimer();
    int64_t usec = 0;
    int64_t last = 0;
    int64_t sum = 0;
    int64_t avg = 0;

    message2.id = 0x000U;
    message2.fdf = mode.fdoe ? 1 : 0;
    message2.brs = mode.brse ? 1 : 0;
    message2.xtd = 0;
    message2.rtr = 0;
    message2.esi = 0;
    message2.sts = 0;
    message2.dlc = mode.fdoe ? CANFD_MAX_DLC : CAN_MAX_DLC;
    memset(message2.data, 0, CANFD_MAX_LEN);
    // @pre:
    // @- initialize DUT1 with configured settings
    handle1 = can_init(DUT1, mode.byte, NULL);
    XCTAssertLessThanOrEqual(0, handle1);
    // @- get status of DUT1 and check to be in INIT state
    rc = can_status(handle1, &status.byte);
    XCTAssertEqual(CANERR_NOERROR, rc);
    XCTAssertTrue(status.can_stopped);
    // @- initialize DUT2 with configured settings
    handle2 = can_init(DUT2, mode.byte, NULL);
    XCTAssertLessThanOrEqual(0, handle1);
    // @- get status of DUT2 and check to be in INIT state
    rc = can_status(handle2, &status.byte);
    XCTAssertEqual(CANERR_NOERROR, rc);
    XCTAssertTrue(status.can_stopped);
    // @- start DUT1 with configured bit-rate settings
    rc = can_start(handle1, &bitrate);
    XCTAssertEqual(CANERR_NOERROR, rc);
    // @- get status of DUT1 and check to be in RUNNING state
    rc = can_status(handle1, &status.byte);
    XCTAssertEqual(CANERR_NOERROR, rc);
    XCTAssertFalse(status.can_stopped);
    // @- start DUT2 with configured bit-rate settings
    rc = can_start(handle2, &bitrate);
    XCTAssertEqual(CANERR_NOERROR, rc);
    // @- get status of DUT2 and check to be in RUNNING state
    rc = can_status(handle2, &status.byte);
    XCTAssertEqual(CANERR_NOERROR, rc);
    XCTAssertFalse(status.can_stopped);
    // @issue(PeakCAN): a delay of 100ms is required here
    PCBUSB_INIT_DELAY();

#if (TEST_TIMESTAMP_10MS != 0)
    // @test:
    NSLog(@"Be patient...");
    timer.Restart((uint32_t)TEST_TIMESTAMP * 100U * CTimer::MSEC);
    // @- send n CAN messages with 10ms delay from DUT2 to DUT1:
    message2.id = 0x010U;
    for (i = n = 0; i < TEST_TIMESTAMP; i++) {
        message2.data[0] = (uint8_t)((uint64_t)i >> 0);
        message2.data[1] = (uint8_t)((uint64_t)i >> 8);
        message2.data[2] = (uint8_t)((uint64_t)i >> 16);
        message2.data[3] = (uint8_t)((uint64_t)i >> 24);
        message2.data[4] = (uint8_t)((uint64_t)i >> 32);
        message2.data[5] = (uint8_t)((uint64_t)i >> 40);
        message2.data[6] = (uint8_t)((uint64_t)i >> 48);
        message2.data[7] = (uint8_t)((uint64_t)i >> 56);
        // @-- start a timer and send the message
        delay.Restart(TIMESTAMP_DELAY_10MS * CTimer::MSEC);
        do {
            rc = can_write(handle2, &message2, 0U);
        } while (CANERR_TX_BUSY == rc);
        XCTAssertEqual(CANERR_NOERROR, rc);
        if (CANERR_NOERROR != rc)
            break;
        while (!delay.Timeout());
    }
    NSLog(@"%d frame(s) sent with %u.000ms delay", i, TIMESTAMP_DELAY_10MS);
    sum = last = 0;
    // @- read all CAN messages from DUT1 receive queue:
    while ((n < i) && !timer.Timeout()) {
        memset(&message1, 0, sizeof(can_message_t));
        // @-- read one message and sum up the time differences
        rc = can_read(handle1, &message1, 1000U);
        XCTAssertEqual(CANERR_NOERROR, rc);
        if (!message1.sts) {
            usec = ((int64_t)message1.timestamp.tv_sec * (int64_t)1000000)
                 + ((int64_t)message1.timestamp.tv_nsec / (int64_t)1000);
            if (last)
                sum += (usec - last);
            last = usec;
            n++;
        }
    }
    XCTAssertEqual(i, n);
    // @- calculate the time average and check for tolerance
    avg = (0 < n) ? (sum / (int64_t)(n-1)) : 0;
    XCTAssert((TIMESTAMP_LOWER_10MS <= avg) && (avg <= TIMESTAMP_UPPER_10MS));
    NSLog(@"%d frame(s) read with %.3fms average", n, (float)avg / 1000.0);
    // @- get status of DUT1 and check to be in RUNNING state
    rc = can_status(handle1, &status.byte);
    XCTAssertEqual(CANERR_NOERROR, rc);
    XCTAssertFalse(status.can_stopped);
#endif

#if (TEST_TIMESTAMP_7MS != 0)
    // @test:
    NSLog(@"Be patient...");
    timer.Restart((uint32_t)TEST_TIMESTAMP * 100U * CTimer::MSEC);
    // @- send n CAN messages with 7ms delay from DUT2 to DUT1:
    message2.id = 0x010U;
    for (i = n = 0; i < TEST_TIMESTAMP; i++) {
        message2.data[0] = (uint8_t)((uint64_t)i >> 0);
        message2.data[1] = (uint8_t)((uint64_t)i >> 8);
        message2.data[2] = (uint8_t)((uint64_t)i >> 16);
        message2.data[3] = (uint8_t)((uint64_t)i >> 24);
        message2.data[4] = (uint8_t)((uint64_t)i >> 32);
        message2.data[5] = (uint8_t)((uint64_t)i >> 40);
        message2.data[6] = (uint8_t)((uint64_t)i >> 48);
        message2.data[7] = (uint8_t)((uint64_t)i >> 56);
        // @-- start a timer and send the message
        delay.Restart(TIMESTAMP_DELAY_7MS * CTimer::MSEC);
        do {
            rc = can_write(handle2, &message2, 0U);
        } while (CANERR_TX_BUSY == rc);
        XCTAssertEqual(CANERR_NOERROR, rc);
        if (CANERR_NOERROR != rc)
            break;
        while (!delay.Timeout());
    }
    NSLog(@"%d frame(s) sent with %u.000ms delay", i, TIMESTAMP_DELAY_7MS);
    sum = last = 0;
    // @- read all CAN messages from DUT1 receive queue:
    while ((n < i) && !timer.Timeout()) {
        memset(&message1, 0, sizeof(can_message_t));
        // @-- read one message and sum up the time differences
        rc = can_read(handle1, &message1, 1000U);
        XCTAssertEqual(CANERR_NOERROR, rc);
        if (!message1.sts) {
            usec = ((int64_t)message1.timestamp.tv_sec * (int64_t)1000000)
                 + ((int64_t)message1.timestamp.tv_nsec / (int64_t)1000);
            if (last)
                sum += (usec - last);
            last = usec;
            n++;
        }
    }
    XCTAssertEqual(i, n);
    // @- calculate the time average and check for tolerance
    avg = (0 < n) ? (sum / (int64_t)(n-1)) : 0;
    XCTAssert((TIMESTAMP_LOWER_7MS <= avg) && (avg <= TIMESTAMP_UPPER_7MS));
    NSLog(@"%d frame(s) read with %.3fms average", n, (float)avg / 1000.0);
    // @- get status of DUT1 and check to be in RUNNING state
    rc = can_status(handle1, &status.byte);
    XCTAssertEqual(CANERR_NOERROR, rc);
    XCTAssertFalse(status.can_stopped);
#endif

#if (TEST_TIMESTAMP_5MS != 0)
    // @test:
    NSLog(@"Be patient...");
    timer.Restart((uint32_t)TEST_TIMESTAMP * 100U * CTimer::MSEC);
    // @- send n CAN messages with 5ms delay from DUT2 to DUT1:
    message2.id = 0x010U;
    for (i = n = 0; i < TEST_TIMESTAMP; i++) {
        message2.data[0] = (uint8_t)((uint64_t)i >> 0);
        message2.data[1] = (uint8_t)((uint64_t)i >> 8);
        message2.data[2] = (uint8_t)((uint64_t)i >> 16);
        message2.data[3] = (uint8_t)((uint64_t)i >> 24);
        message2.data[4] = (uint8_t)((uint64_t)i >> 32);
        message2.data[5] = (uint8_t)((uint64_t)i >> 40);
        message2.data[6] = (uint8_t)((uint64_t)i >> 48);
        message2.data[7] = (uint8_t)((uint64_t)i >> 56);
        // @-- start a timer and send the message
        delay.Restart(TIMESTAMP_DELAY_5MS * CTimer::MSEC);
        do {
            rc = can_write(handle2, &message2, 0U);
        } while (CANERR_TX_BUSY == rc);
        XCTAssertEqual(CANERR_NOERROR, rc);
        if (CANERR_NOERROR != rc)
            break;
        while (!delay.Timeout());
    }
    NSLog(@"%d frame(s) sent with %u.000ms delay", i, TIMESTAMP_DELAY_5MS);
    sum = last = 0;
    // @- read all CAN messages from DUT1 receive queue:
    while ((n < i) && !timer.Timeout()) {
        memset(&message1, 0, sizeof(can_message_t));
        // @-- read one message and sum up the time differences
        rc = can_read(handle1, &message1, 1000U);
        XCTAssertEqual(CANERR_NOERROR, rc);
        if (!message1.sts) {
            usec = ((int64_t)message1.timestamp.tv_sec * (int64_t)1000000)
                 + ((int64_t)message1.timestamp.tv_nsec / (int64_t)1000);
            if (last)
                sum += (usec - last);
            last = usec;
            n++;
        }
    }
    XCTAssertEqual(i, n);
    // @- calculate the time average and check for tolerance
    avg = (0 < n) ? (sum / (int64_t)(n-1)) : 0;
    XCTAssert((TIMESTAMP_LOWER_5MS <= avg) && (avg <= TIMESTAMP_UPPER_5MS));
    NSLog(@"%d frame(s) read with %.3fms average", n, (float)avg / 1000.0);
    // @- get status of DUT1 and check to be in RUNNING state
    rc = can_status(handle1, &status.byte);
    XCTAssertEqual(CANERR_NOERROR, rc);
    XCTAssertFalse(status.can_stopped);
#endif

#if (TEST_TIMESTAMP_2MS != 0)
    // @test:
    NSLog(@"Be patient...");
    timer.Restart((uint32_t)TEST_TIMESTAMP * 100U * CTimer::MSEC);
    // @- send n CAN messages with 2ms delay from DUT2 to DUT1:
    message2.id = 0x010U;
    for (i = n = 0; i < TEST_TIMESTAMP; i++) {
        message2.data[0] = (uint8_t)((uint64_t)i >> 0);
        message2.data[1] = (uint8_t)((uint64_t)i >> 8);
        message2.data[2] = (uint8_t)((uint64_t)i >> 16);
        message2.data[3] = (uint8_t)((uint64_t)i >> 24);
        message2.data[4] = (uint8_t)((uint64_t)i >> 32);
        message2.data[5] = (uint8_t)((uint64_t)i >> 40);
        message2.data[6] = (uint8_t)((uint64_t)i >> 48);
        message2.data[7] = (uint8_t)((uint64_t)i >> 56);
        // @-- start a timer and send the message
        delay.Restart(TIMESTAMP_DELAY_2MS * CTimer::MSEC);
        do {
            rc = can_write(handle2, &message2, 0U);
        } while (CANERR_TX_BUSY == rc);
        XCTAssertEqual(CANERR_NOERROR, rc);
        if (CANERR_NOERROR != rc)
            break;
        while (!delay.Timeout());
    }
    NSLog(@"%d frame(s) sent with %u.000ms delay", i, TIMESTAMP_DELAY_2MS);
    sum = last = 0;
    // @- read all CAN messages from DUT1 receive queue:
    while ((n < i) && !timer.Timeout()) {
        memset(&message1, 0, sizeof(can_message_t));
        // @-- read one message and sum up the time differences
        rc = can_read(handle1, &message1, 1000U);
        XCTAssertEqual(CANERR_NOERROR, rc);
        if (!message1.sts) {
            usec = ((int64_t)message1.timestamp.tv_sec * (int64_t)1000000)
                 + ((int64_t)message1.timestamp.tv_nsec / (int64_t)1000);
            if (last)
                sum += (usec - last);
            last = usec;
            n++;
        }
    }
    XCTAssertEqual(i, n);
    // @- calculate the time average and check for tolerance
    avg = (0 < n) ? (sum / (int64_t)(n-1)) : 0;
    XCTAssert((TIMESTAMP_LOWER_2MS <= avg) && (avg <= TIMESTAMP_UPPER_2MS));
    NSLog(@"%d frame(s) read with %.3fms average", n, (float)avg / 1000.0);
    // @- get status of DUT1 and check to be in RUNNING state
    rc = can_status(handle1, &status.byte);
    XCTAssertEqual(CANERR_NOERROR, rc);
    XCTAssertFalse(status.can_stopped);
#endif

#if (TEST_TIMESTAMP_1MS != 0)
    // @test:
    NSLog(@"Be patient...");
    timer.Restart((uint32_t)TEST_TIMESTAMP * 100U * CTimer::MSEC);
    // @- send n CAN messages with 1ms delay from DUT2 to DUT1:
    message2.id = 0x010U;
    for (i = n = 0; i < TEST_TIMESTAMP; i++) {
        message2.data[0] = (uint8_t)((uint64_t)i >> 0);
        message2.data[1] = (uint8_t)((uint64_t)i >> 8);
        message2.data[2] = (uint8_t)((uint64_t)i >> 16);
        message2.data[3] = (uint8_t)((uint64_t)i >> 24);
        message2.data[4] = (uint8_t)((uint64_t)i >> 32);
        message2.data[5] = (uint8_t)((uint64_t)i >> 40);
        message2.data[6] = (uint8_t)((uint64_t)i >> 48);
        message2.data[7] = (uint8_t)((uint64_t)i >> 56);
        // @-- start a timer and send the message
        delay.Restart(TIMESTAMP_DELAY_1MS * CTimer::MSEC);
        do {
            rc = can_write(handle2, &message2, 0U);
        } while (CANERR_TX_BUSY == rc);
        XCTAssertEqual(CANERR_NOERROR, rc);
        if (CANERR_NOERROR != rc)
            break;
        while (!delay.Timeout());
    }
    NSLog(@"%d frame(s) sent with %u.000ms delay", i, TIMESTAMP_DELAY_1MS);
    sum = last = 0;
    // @- read all CAN messages from DUT1 receive queue:
    while ((n < i) && !timer.Timeout()) {
        memset(&message1, 0, sizeof(can_message_t));
        // @-- read one message and sum up the time differences
        rc = can_read(handle1, &message1, 1000U);
        XCTAssertEqual(CANERR_NOERROR, rc);
        if (!message1.sts) {
            usec = ((int64_t)message1.timestamp.tv_sec * (int64_t)1000000)
                 + ((int64_t)message1.timestamp.tv_nsec / (int64_t)1000);
            if (last)
                sum += (usec - last);
            last = usec;
            n++;
        }
    }
    XCTAssertEqual(i, n);
    // @- calculate the time average and check for tolerance
    avg = (0 < n) ? (sum / (int64_t)(n-1)) : 0;
    XCTAssert((TIMESTAMP_LOWER_1MS <= avg) && (avg <= TIMESTAMP_UPPER_1MS));
    NSLog(@"%d frame(s) read with %.3fms average", n, (float)avg / 1000.0);
    // @- get status of DUT1 and check to be in RUNNING state
    rc = can_status(handle1, &status.byte);
    XCTAssertEqual(CANERR_NOERROR, rc);
    XCTAssertFalse(status.can_stopped);
#endif
    // @test:
    NSLog(@"Be patient...");
    timer.Restart((uint32_t)TEST_TIMESTAMP * 100U * CTimer::MSEC);
    // @- send n CAN messages without delay from DUT2 to DUT1:
    message2.id = 0x010U;
    for (i = n = 0; i < TEST_TIMESTAMP; i++) {
        message2.data[0] = (uint8_t)((uint64_t)i >> 0);
        message2.data[1] = (uint8_t)((uint64_t)i >> 8);
        message2.data[2] = (uint8_t)((uint64_t)i >> 16);
        message2.data[3] = (uint8_t)((uint64_t)i >> 24);
        message2.data[4] = (uint8_t)((uint64_t)i >> 32);
        message2.data[5] = (uint8_t)((uint64_t)i >> 40);
        message2.data[6] = (uint8_t)((uint64_t)i >> 48);
        message2.data[7] = (uint8_t)((uint64_t)i >> 56);
        // @-- start a timer and send the message
        delay.Restart(TIMESTAMP_DELAY_0MS * CTimer::MSEC);
        do {
            rc = can_write(handle2, &message2, 0U);
        } while (CANERR_TX_BUSY == rc);
        XCTAssertEqual(CANERR_NOERROR, rc);
        if (CANERR_NOERROR != rc)
            break;
        //while (!delay.Timeout());
    }
    NSLog(@"%d frame(s) sent without delay", i);
    sum = last = 0;
    // @- read all CAN messages from DUT1 receive queue:
    while ((n < i) && !timer.Timeout()) {
        memset(&message1, 0, sizeof(can_message_t));
        // @-- read one message and sum up the time differences
        rc = can_read(handle1, &message1, 1000U);
        XCTAssertEqual(CANERR_NOERROR, rc);
        if (!message1.sts) {
            usec = ((int64_t)message1.timestamp.tv_sec * (int64_t)1000000)
                 + ((int64_t)message1.timestamp.tv_nsec / (int64_t)1000);
            if (last)
                sum += (usec - last);
#if (0)
            NSLog(@"%d %li.%09li %lli %lli %lli %lli",n,message1.timestamp.tv_sec,message1.timestamp.tv_nsec,usec,last,usec-last,sum);
#endif
            last = usec;
            n++;
        }
    }
    XCTAssertEqual(i, n);
    // @- calculate the time average and check for tolerance
    avg = (0 < n) ? (sum / (int64_t)(n-1)) : 0;
    XCTAssert((0 <= avg) && (avg <= TIMESTAMP_UPPER_0MS));
    NSLog(@"%d frame(s) read with %.3fms average", n, (float)avg / 1000.0);
    // @- get status of DUT1 and check to be in RUNNING state
    rc = can_status(handle1, &status.byte);
    XCTAssertEqual(CANERR_NOERROR, rc);
    XCTAssertFalse(status.can_stopped);
    // @post:
    // @- stop/reset DUT1
    rc = can_reset(handle1);
    XCTAssertEqual(CANERR_NOERROR, rc);
    // @- get status of DUT1 and check to be in INIT state
    rc = can_status(handle1, &status.byte);
    XCTAssertEqual(CANERR_NOERROR, rc);
    XCTAssertTrue(status.can_stopped);
    // @- tear down DUT1
    rc = can_exit(handle1);
    XCTAssertEqual(CANERR_NOERROR, rc);
    // @- tear down DUT2
    rc = can_exit(handle2);
    XCTAssertEqual(CANERR_NOERROR, rc);

    // @end.
}
#endif

@end

// $Id: test_can_read.mm 1341 2024-06-15 16:43:48Z makemake $  Copyright (c) UV Software, Berlin //
