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

@interface test_can_write : XCTestCase

@end

@implementation test_can_write

- (void)setUp {
    // Put setup code here. This method is called before the invocation of each test method in the class.
}

- (void)tearDown {
    // Put teardown code here. This method is called after the invocation of each test method in the class.
    (void)can_exit(CANKILL_ALL);
}

// @xctest TC05.0: Send a CAN message (sunnyday scenario)
//
// @expected: CANERR_NOERROR
//
// - (void)testSunnydayScenario {
//     // @test:
//     // @todo: insert coin here
//     // @end.
// }

// @xctest TC05.1: Send a CAN message with invalid channel handle(s)
//
// @expected: CANERR_HANDLE
//
- (void)testWithInvalidHandle {
    can_bitrate_t bitrate = { TEST_BTRINDEX };
    can_status_t status = { CANSTAT_RESET };
    can_mode_t mode = { TEST_CANMODE };
    can_message_t message = {};
    int handle = INVALID_HANDLE;
    int rc = CANERR_FATAL;
    // transmit message
    message.id = 0x300U;
    message.fdf = mode.fdoe ? 1 : 0;
    message.brs = mode.brse ? 1 : 0;
    message.xtd = 0;
    message.rtr = 0;
    message.esi = 0;
    message.sts = 0;
    message.dlc = mode.fdoe ? CANFD_MAX_DLC : CAN_MAX_DLC;
    memset(message.data, 0, CANFD_MAX_LEN);
    // @pre:
    // @- initialize DUT1 with configured settings
    handle = can_init(DUT1, mode.byte, TEST_PARAM(PAR1));
    XCTAssertLessThanOrEqual(0, handle);
    // @- get status of DUT1 and check to be in INIT state
    rc = can_status(handle, &status.byte);
    XCTAssertEqual(CANERR_NOERROR, rc);
    XCTAssertTrue(status.can_stopped);
    // @test:
    // @- try to send a message from DUT1 with invalid handle -1
    rc = can_write(INVALID_HANDLE, &message, 0U);
    XCTAssertEqual(CANERR_HANDLE, rc);
    // @- get status of DUT1 and check to be in INIT state
    rc = can_status(handle, &status.byte);
    XCTAssertEqual(CANERR_NOERROR, rc);
    XCTAssertTrue(status.can_stopped);
    // @- start DUT1 with configured bit-rate settings
    rc = can_start(handle, &bitrate);
    XCTAssertEqual(CANERR_NOERROR, rc);
    // @- try to send a message from DUT1 with invalid handle INT32_MIN
    rc = can_write(INT32_MIN, &message, 0U);
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
    // @- try to send a message from DUT1 with invalid handle INT32_MAX
    rc = can_write(INT32_MAX, &message, 0U);
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

// @xctest TC05.2: Give a NULL pointer as argument for parameter 'message'
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
    // @- send a message from DUT1 with NULL for parameter 'message'
    rc = can_write(handle, NULL, 0U);
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

// @xctest TC05.3: Send a CAN message if CAN channel is not initialized
//
// @expected: CANERR_NOTINIT
//
- (void)testIfChannelNotInitialized {
    can_bitrate_t bitrate = { TEST_BTRINDEX };
    can_status_t status = { CANSTAT_RESET };
    can_mode_t mode = { TEST_CANMODE };
    can_message_t message = {};
    int handle = INVALID_HANDLE;
    int rc = CANERR_FATAL;
    // transmit message
    message.id = 0x302U;
    message.fdf = mode.fdoe ? 1 : 0;
    message.brs = mode.brse ? 1 : 0;
    message.xtd = 0;
    message.rtr = 0;
    message.esi = 0;
    message.sts = 0;
    message.dlc = mode.fdoe ? CANFD_MAX_DLC : CAN_MAX_DLC;
    memset(message.data, 0, CANFD_MAX_LEN);
    // @test:
    // @- try to send a message from DUT1
    rc = can_write(DUT1, &message, 0U);
    XCTAssertEqual(CANERR_NOTINIT, rc);
    // @post:
    // @- initialize DUT1 with configured settings
    handle = can_init(DUT1, mode.byte, TEST_PARAM(PAR1));
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

// @xctest TC05.4: Send a CAN message if CAN controller is not started
//
// @expected: CANERR_OFFLINE
//
- (void)testControllerNotNotStarted {
    can_bitrate_t bitrate = { TEST_BTRINDEX };
    can_status_t status = { CANSTAT_RESET };
    can_mode_t mode = { TEST_CANMODE };
    can_message_t message = {};
    int handle = INVALID_HANDLE;
    int rc = CANERR_FATAL;
    // transmit message
    message.id = 0x303U;
    message.fdf = mode.fdoe ? 1 : 0;
    message.brs = mode.brse ? 1 : 0;
    message.xtd = 0;
    message.rtr = 0;
    message.esi = 0;
    message.sts = 0;
    message.dlc = mode.fdoe ? CANFD_MAX_DLC : CAN_MAX_DLC;
    memset(message.data, 0, CANFD_MAX_LEN);
    // @pre:
    // @- initialize DUT1 with configured settings
    handle = can_init(DUT1, mode.byte, TEST_PARAM(PAR1));
    XCTAssertLessThanOrEqual(0, handle);
    // @test:
    // @- try to send a message from DUT1
    rc = can_write(handle, &message, 0U);
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

// @xctest TC05.5: Send a CAN message if CAN controller was previously stopped
//
// @expected: CANERR_OFFLINE
//
- (void)testIfControllerStopped {
    can_bitrate_t bitrate = { TEST_BTRINDEX };
    can_status_t status = { CANSTAT_RESET };
    can_mode_t mode = { TEST_CANMODE };
    can_message_t message = {};
    int handle = INVALID_HANDLE;
    int rc = CANERR_FATAL;
    // transmit message
    message.id = 0x304U;
    message.fdf = mode.fdoe ? 1 : 0;
    message.brs = mode.brse ? 1 : 0;
    message.xtd = 0;
    message.rtr = 0;
    message.esi = 0;
    message.sts = 0;
    message.dlc = mode.fdoe ? CANFD_MAX_DLC : CAN_MAX_DLC;
    memset(message.data, 0, CANFD_MAX_LEN);
    // @pre:
    // @- initialize DUT1 with configured settings
    handle = can_init(DUT1, mode.byte, TEST_PARAM(PAR1));
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
    // @- try to send a message from DUT1
    rc = can_write(handle, &message, 0U);
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

// @xctest TC05.6: Send a CAN message if CAN channel was previously torn down
//
// @expected: CANERR_NOTINIT
//
- (void)testIfChannelTornDown {
    can_bitrate_t bitrate = { TEST_BTRINDEX };
    can_status_t status = { CANSTAT_RESET };
    can_mode_t mode = { TEST_CANMODE };
    can_message_t message = {};
    int handle = INVALID_HANDLE;
    int rc = CANERR_FATAL;
    // transmit message
    message.id = 0x305U;
    message.fdf = mode.fdoe ? 1 : 0;
    message.brs = mode.brse ? 1 : 0;
    message.xtd = 0;
    message.rtr = 0;
    message.esi = 0;
    message.sts = 0;
    message.dlc = mode.fdoe ? CANFD_MAX_DLC : CAN_MAX_DLC;
    memset(message.data, 0, CANFD_MAX_LEN);
    // @pre:
    // @- initialize DUT1 with configured settings
    handle = can_init(DUT1, mode.byte, TEST_PARAM(PAR1));
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
    // @- try to send a message from DUT1
    rc = can_write(handle, &message, 0U);
    XCTAssertEqual(CANERR_NOTINIT, rc);
    // @end.
}

// @xctest TC05.7: Send CAN messages with valid 11-bit identifier(s) and check its correct transmission on receiver side
//
// @expected: CANERR_NOERROR
//
- (void)testWithValid11bitIdentifier {
    can_bitrate_t bitrate = { TEST_BTRINDEX };
    can_status_t status = { CANSTAT_RESET };
    can_message_t message1 = {};
    can_message_t message2 = {};
    can_mode_t mode = { TEST_CANMODE };
    int handle1 = INVALID_HANDLE;
    int handle2 = INVALID_HANDLE;
    int rc = CANERR_FATAL;

    message1.id = 0x306U;
    message1.fdf = mode.fdoe ? 1 : 0;
    message1.brs = mode.brse ? 1 : 0;
    message1.xtd = 0;
    message1.rtr = 0;
    message1.esi = 0;
    message1.sts = 0;
    message1.dlc = mode.fdoe ? CANFD_MAX_DLC : CAN_MAX_DLC;
    memset(message1.data, 0, CANFD_MAX_LEN);
    // @pre:
    mode.nxtd = 0;
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
    // @- loop over all 11-bit CAN identifier (0x000 to 0x7FF with +1)
    for (uint32_t canId = 0x000U; canId <= CAN_MAX_STD_ID; canId++) {
        for (uint8_t i = 0; i < (mode.fdoe ? CANFD_MAX_LEN : CAN_MAX_LEN); i++)
            message1.data[i] = (uint8_t)canId + i;
        // @-- send one message with valid STD id. from DUT1
        message1.id = canId;
        rc = can_write(handle1, &message1, 0U);
        XCTAssertEqual(CANERR_NOERROR, rc);
        // @-- read one message from DUT2 receive queue (to <= 100ms)
        memset(&message2, 0, sizeof(can_message_t));
        rc = can_read(handle2, &message2, 100U);
        XCTAssertEqual(CANERR_NOERROR, rc);
        // @-- compare sent and received message (ignore status message)
        if (!message2.sts) {
            XCTAssertEqual(message1.id, message2.id);
            XCTAssertEqual(message1.fdf, message2.fdf);
            XCTAssertEqual(message1.brs, message2.brs);
            XCTAssertEqual(message1.xtd, message2.xtd);
            XCTAssertEqual(message1.rtr, message2.rtr);
            XCTAssertEqual(message1.esi, message2.esi);
            XCTAssertEqual(message1.sts, message2.sts);
            XCTAssertEqual(message1.dlc, message2.dlc);
            XCTAssertEqual(0, memcmp(message1.data, message2.data, CTester::Dlc2Len(message1.dlc)));
        }
    }
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

// @xctest TC05.8: Send CAN messages with invalid 11-bit identifier(s)
//
// @expected: CANERR_ILLPARA
//
- (void)testWithInvalid11bitIdentifier {
    can_bitrate_t bitrate = { TEST_BTRINDEX };
    can_status_t status = { CANSTAT_RESET };
    can_mode_t mode = { TEST_CANMODE };
    can_message_t message = {};
    int handle = INVALID_HANDLE;
    int rc = CANERR_FATAL;
    // transmit message
    message.id = 0x307U;
    message.fdf = mode.fdoe ? 1 : 0;
    message.brs = mode.brse ? 1 : 0;
    message.xtd = 0;
    message.rtr = 0;
    message.esi = 0;
    message.sts = 0;
    message.dlc = mode.fdoe ? CANFD_MAX_DLC : CAN_MAX_DLC;
    memset(message.data, 0, CANFD_MAX_LEN);
    // @pre:
    mode.nxtd = 0;
    // @- initialize DUT1 with configured settings but w/o bit NXTD
    handle = can_init(DUT1, mode.byte, TEST_PARAM(PAR1));
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
    // @test:
    // @- try to send a message with invalid STD 0x800 from DUT1
    message.id = 0x800U;
    rc = can_write(handle, &message, 0U);
    XCTAssertEqual(CANERR_ILLPARA, rc);
    // @- try to send a message with invalid STD 0xFFF from DUT1
    message.id = 0xFFFU;
    rc = can_write(handle, &message, 0U);
    XCTAssertEqual(CANERR_ILLPARA, rc);
    // @- try to send a message with invalid STD 0x1000 from DUT1
    message.id = 0x1000U;
    rc = can_write(handle, &message, 0U);
    XCTAssertEqual(CANERR_ILLPARA, rc);
    // @- try to send a message with invalid STD 0xFFFF from DUT1
    message.id = 0xFFFFU;
    rc = can_write(handle, &message, 0U);
    XCTAssertEqual(CANERR_ILLPARA, rc);
    // @- try to send a message with invalid STD 0x10000 from DUT1
    message.id = 0x10000U;
    rc = can_write(handle, &message, 0U);
    XCTAssertEqual(CANERR_ILLPARA, rc);
    // @- try to send a message with invalid STD 0xFFFFF from DUT1
    message.id = 0xFFFFFU;
    rc = can_write(handle, &message, 0U);
    XCTAssertEqual(CANERR_ILLPARA, rc);
    // @- try to send a message with invalid STD 0x100000 from DUT1
    message.id = 0x100000U;
    rc = can_write(handle, &message, 0U);
    XCTAssertEqual(CANERR_ILLPARA, rc);
    // @- try to send a message with invalid STD 0xFFFFFF from DUT1
    message.id = 0xFFFFFFU;
    rc = can_write(handle, &message, 0U);
    XCTAssertEqual(CANERR_ILLPARA, rc);
    // @- try to send a message with invalid STD 0x1000000 from DUT1
    message.id = 0x1000000U;
    rc = can_write(handle, &message, 0U);
    XCTAssertEqual(CANERR_ILLPARA, rc);
    // @- try to send a message with invalid STD 0xFFFFFFF from DUT1
    message.id = 0xFFFFFFFU;
    rc = can_write(handle, &message, 0U);
    XCTAssertEqual(CANERR_ILLPARA, rc);
    // @- try to send a message with invalid STD 0x10000000 from DUT1
    message.id = 0x10000000U;
    rc = can_write(handle, &message, 0U);
    XCTAssertEqual(CANERR_ILLPARA, rc);
    // @- try to send a message with invalid STD 0x1FFFFFFF from DUT1
    message.id = 0x1FFFFFFFU;
    rc = can_write(handle, &message, 0U);
    XCTAssertEqual(CANERR_ILLPARA, rc);
    // @- try to send a message with invalid STD 0x20000000 from DUT1
    message.id = 0x20000000U;
    rc = can_write(handle, &message, 0U);
    XCTAssertEqual(CANERR_ILLPARA, rc);
    // @- try to send a message with invalid STD 0xFFFFFFFF from DUT1
    message.id = 0xFFFFFFFFU;
    rc = can_write(handle, &message, 0U);
    XCTAssertEqual(CANERR_ILLPARA, rc);
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

// @xctest TC05.9: Send CAN messages with valid 29-bit identifier(s) and check its correct transmission on receiver side
//
// @expected: CANERR_NOERROR
//
- (void)testWithValid29bitIdentifier {
    can_bitrate_t bitrate = { TEST_BTRINDEX };
    can_status_t status = { CANSTAT_RESET };
    can_message_t message1 = {};
    can_message_t message2 = {};
    can_mode_t mode = { TEST_CANMODE };
    int handle1 = INVALID_HANDLE;
    int handle2 = INVALID_HANDLE;
    int rc = CANERR_FATAL;

    message1.id = 0x308U;
    message1.fdf = mode.fdoe ? 1 : 0;
    message1.brs = mode.brse ? 1 : 0;
    message1.xtd = 1;
    message1.rtr = 0;
    message1.esi = 0;
    message1.sts = 0;
    message1.dlc = mode.fdoe ? CANFD_MAX_DLC : CAN_MAX_DLC;
    memset(message1.data, 0, CANFD_MAX_LEN);
    // @pre:
    mode.nxtd = 0;
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
    // @- loop over all 29-bit CAN identifier (0x000 to 0x1FFFFFFF with (<<1)+1)
    for (uint32_t canId = 0x000U; canId <= CAN_MAX_XTD_ID; canId = ((canId << 1) + 1U))  {
        for (uint8_t i = 0; i < (mode.fdoe ? CANFD_MAX_LEN : CAN_MAX_LEN); i++)
            message1.data[i] = (uint8_t)canId + i;
        // @-- send one message with valid XTD id. from DUT1
        message1.id = canId;
        rc = can_write(handle1, &message1, 0U);
        XCTAssertEqual(CANERR_NOERROR, rc);
        // @-- read one message from DUT2 receive queue (to <= 100ms)
        memset(&message2, 0, sizeof(can_message_t));
        rc = can_read(handle2, &message2, 100U);
        XCTAssertEqual(CANERR_NOERROR, rc);
        // @-- compare sent and received message (ignore status message)
        if (!message2.sts) {
            XCTAssertEqual(message1.id, message2.id);
            XCTAssertEqual(message1.fdf, message2.fdf);
            XCTAssertEqual(message1.brs, message2.brs);
            XCTAssertEqual(message1.xtd, message2.xtd);
            XCTAssertEqual(message1.rtr, message2.rtr);
            XCTAssertEqual(message1.esi, message2.esi);
            XCTAssertEqual(message1.sts, message2.sts);
            XCTAssertEqual(message1.dlc, message2.dlc);
            XCTAssertEqual(0, memcmp(message1.data, message2.data, CTester::Dlc2Len(message1.dlc)));
        }
    }
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

// @xctest TC05.10: Send CAN messages with invalid 29-bit identifier(s)
//
// @expected: CANERR_ILLPARA
//
- (void)testWithInvalid29bitIdentifier  {
    can_bitrate_t bitrate = { TEST_BTRINDEX };
    can_status_t status = { CANSTAT_RESET };
    can_mode_t mode = { TEST_CANMODE };
    can_message_t message = {};
    int handle = INVALID_HANDLE;
    int rc = CANERR_FATAL;
    // transmit message
    message.id = 0x309U;
    message.fdf = mode.fdoe ? 1 : 0;
    message.brs = mode.brse ? 1 : 0;
    message.xtd = 1;
    message.rtr = 0;
    message.esi = 0;
    message.sts = 0;
    message.dlc = mode.fdoe ? CANFD_MAX_DLC : CAN_MAX_DLC;
    memset(message.data, 0, CANFD_MAX_LEN);
    // @pre:
    mode.nxtd = 0;
    // @- initialize DUT1 with configured settings but w/o bit NXTD
    handle = can_init(DUT1, mode.byte, TEST_PARAM(PAR1));
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
    // @test:
    // @- try to send a message with invalid XTD 0x20000000 from DUT1
    message.id = 0x20000000U;
    rc = can_write(handle, &message, 0U);
    XCTAssertEqual(CANERR_ILLPARA, rc);
    // @- try to send a message with invalid XTD 0xFFFFFFFF from DUT1
    message.id = 0xFFFFFFFFU;
    rc = can_write(handle, &message, 0U);
    XCTAssertEqual(CANERR_ILLPARA, rc);
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

// @xctest TC05.11: Send CAN messages with valid Data Length Code(s) and check its correct transmission on receiver side
//
// @expected: CANERR_NOERROR
//
- (void)testWithValidDataLengthCode {
    can_bitrate_t bitrate = { TEST_BTRINDEX };
    can_status_t status = { CANSTAT_RESET };
    can_message_t message1 = {};
    can_message_t message2 = {};
    can_mode_t mode = { TEST_CANMODE };
    int handle1 = INVALID_HANDLE;
    int handle2 = INVALID_HANDLE;
    int rc = CANERR_FATAL;

    message1.id = 0x30AU;
    message1.fdf = mode.fdoe ? 1 : 0;
    message1.brs = mode.brse ? 1 : 0;
    message1.xtd = 0;
    message1.rtr = 0;
    message1.esi = 0;
    message1.sts = 0;
    message1.dlc = mode.fdoe ? CANFD_MAX_DLC : CAN_MAX_DLC;
    memset(message1.data, 0, CANFD_MAX_LEN);
    // @pre:
    mode.nxtd = 0;
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
    // @- loop over all Data Length codes (0 to 8 or 15 with +1)
    for (uint8_t canDlc = 0x0U; canDlc <= (mode.fdoe ? CANFD_MAX_DLC : CAN_MAX_DLC); canDlc++)  {
        for (uint8_t i = 0; i < (mode.fdoe ? CANFD_MAX_LEN : CAN_MAX_LEN); i++)
            message1.data[i] = canDlc + i + '0';
        // @-- send one message with valid DLC from DUT1
        message1.dlc = canDlc;
        rc = can_write(handle1, &message1, 0U);
        XCTAssertEqual(CANERR_NOERROR, rc);
        // @-- read one message from DUT2 receive queue (to <= 100ms)
        memset(&message2, 0, sizeof(can_message_t));
        rc = can_read(handle2, &message2, 100U);
        XCTAssertEqual(CANERR_NOERROR, rc);
        // @-- compare sent and received message (ignore status message)
        if (!message2.sts) {
            XCTAssertEqual(message1.id, message2.id);
            XCTAssertEqual(message1.fdf, message2.fdf);
            XCTAssertEqual(message1.brs, message2.brs);
            XCTAssertEqual(message1.xtd, message2.xtd);
            XCTAssertEqual(message1.rtr, message2.rtr);
            XCTAssertEqual(message1.esi, message2.esi);
            XCTAssertEqual(message1.sts, message2.sts);
            XCTAssertEqual(message1.dlc, message2.dlc);
            XCTAssertEqual(0, memcmp(message1.data, message2.data, CTester::Dlc2Len(message1.dlc)));
        }
    }
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

// @xctest TC05.12: Send CAN messages with invalid Data Length Code(s)
//
// @expected: CANERR_ILLPARA
//
- (void)testWithInvalidDataLengthCode {
    can_bitrate_t bitrate = { TEST_BTRINDEX };
    can_status_t status = { CANSTAT_RESET };
    can_mode_t mode = { TEST_CANMODE };
    can_message_t message = {};
    int handle = INVALID_HANDLE;
    int rc = CANERR_FATAL;
    // transmit message
    message.id = 0x30BU;
    message.fdf = mode.fdoe ? 1 : 0;
    message.brs = mode.brse ? 1 : 0;
    message.xtd = 0;
    message.rtr = 0;
    message.esi = 0;
    message.sts = 0;
    message.dlc = mode.fdoe ? CANFD_MAX_DLC : CAN_MAX_DLC;
    memset(message.data, 0, CANFD_MAX_LEN);
    // @pre:
    // @- initialize DUT1 with configured settings
    handle = can_init(DUT1, mode.byte, TEST_PARAM(PAR1));
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
    // @test:
    // @- CAN 2.0:
    if (!mode.fdoe) {
        // @-- try to send a message with invalid DLC 0x9 from DUT1
        message.dlc = 0x9U;
        rc = can_write(handle, &message, 0U);
        XCTAssertEqual(CANERR_ILLPARA, rc);
        // @-- try to send a message with invalid DLC 0xA from DUT1
        message.dlc = 0xAU;
        rc = can_write(handle, &message, 0U);
        XCTAssertEqual(CANERR_ILLPARA, rc);
        // @-- try to send a message with invalid DLC 0xB from DUT1
        message.dlc = 0xBU;
        rc = can_write(handle, &message, 0U);
        XCTAssertEqual(CANERR_ILLPARA, rc);
        // @-- try to send a message with invalid DLC 0xC from DUT1
        message.dlc = 0xCU;
        rc = can_write(handle, &message, 0U);
        XCTAssertEqual(CANERR_ILLPARA, rc);
        // @-- try to send a message with invalid DLC 0xD from DUT1
        message.dlc = 0xDU;
        rc = can_write(handle, &message, 0U);
        XCTAssertEqual(CANERR_ILLPARA, rc);
        // @-- try to send a message with invalid DLC 0xE from DUT1
        message.dlc = 0xEU;
        rc = can_write(handle, &message, 0U);
        XCTAssertEqual(CANERR_ILLPARA, rc);
        // @-- try to send a message with invalid DLC 0xF from DUT1
        message.dlc = 0xFU;
        rc = can_write(handle, &message, 0U);
        XCTAssertEqual(CANERR_ILLPARA, rc);
    }
    // @- try to send a message with invalid DLC 0x10 from DUT1
    message.dlc = 0x10U;
    rc = can_write(handle, &message, 0U);
    XCTAssertEqual(CANERR_ILLPARA, rc);
    // @- try to send a message with invalid DLC 0xFF from DUT1
    message.dlc = 0xFFU;
    rc = can_write(handle, &message, 0U);
    XCTAssertEqual(CANERR_ILLPARA, rc);
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

// @xctest TC05.13: Send a CAN message with flag XTD set but operation mode NXTD is selected (suppress extended frames)
//
// @expected: CANERR_ILLPARA
//
- (void)testCheckFlagXtdWhenOperationModeNoXtd {
    can_bitrate_t bitrate = { TEST_BTRINDEX };
    can_status_t status = { CANSTAT_RESET };
    can_mode_t capa = { CANMODE_DEFAULT };
    can_mode_t mode = { CANMODE_DEFAULT };
    can_message_t message = {};
    int handle = INVALID_HANDLE;
    int rc = CANERR_FATAL;
    // transmit message
    message.id = 0x30CU;
    message.fdf = mode.fdoe ? 1 : 0;
    message.brs = mode.brse ? 1 : 0;
    message.xtd = 0;
    message.rtr = 0;
    message.esi = 0;
    message.sts = 0;
    message.dlc = mode.fdoe ? CANFD_MAX_DLC : CAN_MAX_DLC;
    memset(message.data, 0, CANFD_MAX_LEN);
    // @pre:
    // @- initialize DUT1 with configured settings
    handle = can_init(DUT1, TEST_CANMODE, TEST_PARAM(PAR1));
    XCTAssertLessThanOrEqual(0, handle);
    // @- get operation capability from DUT1
    rc = can_property(handle, CANPROP_GET_OP_CAPABILITY, (void*)&capa.byte, sizeof(UInt8));
    XCTAssertEqual(CANERR_NOERROR, rc);
    // @- tear down DUT1
    rc = can_exit(handle);
    XCTAssertEqual(CANERR_NOERROR, rc);
    // @- when suppress XTD frames supported:
    if (capa.nxtd) {
        mode.nxtd = 1;
        // @-- initialize DUT1 with operation mode bit NXTD set
        handle = can_init(DUT1, mode.byte, TEST_PARAM(PAR1));
        XCTAssertLessThanOrEqual(0, handle);
        // @-- get status of DUT1 and check to be in INIT state
        rc = can_status(handle, &status.byte);
        XCTAssertEqual(CANERR_NOERROR, rc);
        XCTAssertTrue(status.can_stopped);
        // @-- start DUT1 with configured bit-rate settings
        rc = can_start(handle, &bitrate);
        XCTAssertEqual(CANERR_NOERROR, rc);
        // @-- get status of DUT1 and check to be in RUNNING state
        rc = can_status(handle, &status.byte);
        XCTAssertEqual(CANERR_NOERROR, rc);
        XCTAssertFalse(status.can_stopped);

        // @test:
        // @-- try to send a message with bit XTD set
        message.xtd = 1;
        rc = can_write(handle, &message, 0U);
        XCTAssertEqual(CANERR_ILLPARA, rc);
        // @-- get status of DUT1 and check to be in RUNNING state
        rc = can_status(handle, &status.byte);
        XCTAssertEqual(CANERR_NOERROR, rc);
        XCTAssertFalse(status.can_stopped);

        // @post:
        // @-- send and receive some frames to/from DUT2 (optional)
#if (SEND_TEST_FRAMES != 0)
        CTester tester;
        XCTAssertEqual(TEST_FRAMES, tester.SendSomeFrames(handle, DUT2, TEST_FRAMES));
        XCTAssertEqual(TEST_FRAMES, tester.ReceiveSomeFrames(handle, DUT2, TEST_FRAMES));
        // @-- get status of DUT1 and check to be in RUNNING state
        rc = can_status(handle, &status.byte);
        XCTAssertEqual(CANERR_NOERROR, rc);
        XCTAssertFalse(status.can_stopped);
#endif
        // @-- stop/reset DUT1
        rc = can_reset(handle);
        XCTAssertEqual(CANERR_NOERROR, rc);
        // @-- get status of DUT1 and check to be in INIT state
        rc = can_status(handle, &status.byte);
        XCTAssertEqual(CANERR_NOERROR, rc);
        XCTAssertTrue(status.can_stopped);
        // @-- tear down DUT1
        rc = can_exit(handle);
        XCTAssertEqual(CANERR_NOERROR, rc);
    }
    // @end.
}

// @xctest TC05.14: Send a CAN message with flag RTR set but operation mode NRTR is selected (suppress remote frames)
//
// @expected: CANERR_ILLPARA
//
- (void)testCheckFlagRtrWhenOperationModeNoRtr {
    can_bitrate_t bitrate = { TEST_BTRINDEX };
    can_status_t status = { CANSTAT_RESET };
    can_mode_t capa = { CANMODE_DEFAULT };
    can_mode_t mode = { CANMODE_DEFAULT };
    can_message_t message = {};
    int handle = INVALID_HANDLE;
    int rc = CANERR_FATAL;
    // transmit message
    message.id = 0x30DU;
    message.fdf = mode.fdoe ? 1 : 0;
    message.brs = mode.brse ? 1 : 0;
    message.xtd = 0;
    message.rtr = 0;
    message.esi = 0;
    message.sts = 0;
    message.dlc = mode.fdoe ? CANFD_MAX_DLC : CAN_MAX_DLC;
    memset(message.data, 0, CANFD_MAX_LEN);
    // @pre:
    // @- initialize DUT1 with configured settings
    handle = can_init(DUT1, TEST_CANMODE, TEST_PARAM(PAR1));
    XCTAssertLessThanOrEqual(0, handle);
    // @- get operation capability from DUT1
    rc = can_property(handle, CANPROP_GET_OP_CAPABILITY, (void*)&capa.byte, sizeof(UInt8));
    XCTAssertEqual(CANERR_NOERROR, rc);
    // @- tear down DUT1
    rc = can_exit(handle);
    XCTAssertEqual(CANERR_NOERROR, rc);
    // @- when suppress RTR frames supported:
    if (capa.nrtr) {
        mode.nrtr = 1;
        // @-- initialize DUT1 with operation mode bit NRTR set
        handle = can_init(DUT1, mode.byte, TEST_PARAM(PAR1));
        XCTAssertLessThanOrEqual(0, handle);
        // @-- get status of DUT1 and check to be in INIT state
        rc = can_status(handle, &status.byte);
        XCTAssertEqual(CANERR_NOERROR, rc);
        XCTAssertTrue(status.can_stopped);
        // @-- start DUT1 with configured bit-rate settings
        rc = can_start(handle, &bitrate);
        XCTAssertEqual(CANERR_NOERROR, rc);
        // @-- get status of DUT1 and check to be in RUNNING state
        rc = can_status(handle, &status.byte);
        XCTAssertEqual(CANERR_NOERROR, rc);
        XCTAssertFalse(status.can_stopped);

        // @test:
        // @-- try to send a message with bit RTR set
        message.rtr = 1;
        rc = can_write(handle, &message, 0U);
        XCTAssertEqual(CANERR_ILLPARA, rc);
        // @-- get status of DUT1 and check to be in RUNNING state
        rc = can_status(handle, &status.byte);
        XCTAssertEqual(CANERR_NOERROR, rc);
        XCTAssertFalse(status.can_stopped);

        // @post:
        // @-- send and receive some frames to/from DUT2 (optional)
#if (SEND_TEST_FRAMES != 0)
        CTester tester;
        XCTAssertEqual(TEST_FRAMES, tester.SendSomeFrames(handle, DUT2, TEST_FRAMES));
        XCTAssertEqual(TEST_FRAMES, tester.ReceiveSomeFrames(handle, DUT2, TEST_FRAMES));
        // @-- get status of DUT1 and check to be in RUNNING state
        rc = can_status(handle, &status.byte);
        XCTAssertEqual(CANERR_NOERROR, rc);
        XCTAssertFalse(status.can_stopped);
#endif
        // @-- stop/reset DUT1
        rc = can_reset(handle);
        XCTAssertEqual(CANERR_NOERROR, rc);
        // @-- get status of DUT1 and check to be in INIT state
        rc = can_status(handle, &status.byte);
        XCTAssertEqual(CANERR_NOERROR, rc);
        XCTAssertTrue(status.can_stopped);
        // @-- tear down DUT1
        rc = can_exit(handle);
        XCTAssertEqual(CANERR_NOERROR, rc);
    }
    // @end.
}

// @xctest TC05.15: Send a CAN FD message with flag FDF set in CAN 2.0 operation mode (FDOE = 0)
//
// @expected: CANERR_ILLPARA
//
- (void)testCheckFlagFdfInCan20OperationMode {
    can_bitrate_t bitrate = { TEST_BTRINDEX };
    can_status_t status = { CANSTAT_RESET };
    can_mode_t capa = { CANMODE_DEFAULT };
    can_mode_t mode = { CANMODE_DEFAULT };
    can_message_t message = {};
    int handle = INVALID_HANDLE;
    int rc = CANERR_FATAL;
    // transmit message
    message.id = 0x30EU;
    message.fdf = mode.fdoe ? 1 : 0;
    message.brs = mode.brse ? 1 : 0;
    message.xtd = 0;
    message.rtr = 0;
    message.esi = 0;
    message.sts = 0;
    message.dlc = mode.fdoe ? CANFD_MAX_DLC : CAN_MAX_DLC;
    memset(message.data, 0, CANFD_MAX_LEN);
    // @pre:
    // @- initialize DUT1 with configured settings
    handle = can_init(DUT1, TEST_CANMODE, TEST_PARAM(PAR1));
    XCTAssertLessThanOrEqual(0, handle);
    // @- get operation capability from DUT1
    rc = can_property(handle, CANPROP_GET_OP_CAPABILITY, (void*)&capa.byte, sizeof(UInt8));
    XCTAssertEqual(CANERR_NOERROR, rc);
    // @- tear down DUT1
    rc = can_exit(handle);
    XCTAssertEqual(CANERR_NOERROR, rc);
    // @- when CAN FD long frames supported:
    if (capa.fdoe) {
        mode.fdoe = mode.brse = 0;
        // @--- initialize DUT1 with operation mode bit FDOE and BRSE cleared
        handle = can_init(DUT1, mode.byte, TEST_PARAM(PAR1));
        XCTAssertLessThanOrEqual(0, handle);
        // @-- get status of DUT1 and check to be in INIT state
        rc = can_status(handle, &status.byte);
        XCTAssertEqual(CANERR_NOERROR, rc);
        XCTAssertTrue(status.can_stopped);
        // @-- start DUT1 with configured bit-rate settings
        rc = can_start(handle, &bitrate);
        XCTAssertEqual(CANERR_NOERROR, rc);
        // @-- get status of DUT1 and check to be in RUNNING state
        rc = can_status(handle, &status.byte);
        XCTAssertEqual(CANERR_NOERROR, rc);
        XCTAssertFalse(status.can_stopped);

        // @test:
        // @-- try to send a message with bit FDF set
        message.fdf = 1;
        message.brs = 0;
        rc = can_write(handle, &message, 0U);
        XCTAssertEqual(CANERR_ILLPARA, rc);
        // @-- get status of DUT1 and check to be in RUNNING state
        rc = can_status(handle, &status.byte);
        XCTAssertEqual(CANERR_NOERROR, rc);
        XCTAssertFalse(status.can_stopped);

        // @post:
        // @-- send and receive some frames to/from DUT2 (optional)
#if (SEND_TEST_FRAMES != 0)
        CTester tester;
        XCTAssertEqual(TEST_FRAMES, tester.SendSomeFrames(handle, DUT2, TEST_FRAMES));
        XCTAssertEqual(TEST_FRAMES, tester.ReceiveSomeFrames(handle, DUT2, TEST_FRAMES));
        // @-- get status of DUT1 and check to be in RUNNING state
        rc = can_status(handle, &status.byte);
        XCTAssertEqual(CANERR_NOERROR, rc);
        XCTAssertFalse(status.can_stopped);
#endif
        // @-- stop/reset DUT1
        rc = can_reset(handle);
        XCTAssertEqual(CANERR_NOERROR, rc);
        // @-- get status of DUT1 and check to be in INIT state
        rc = can_status(handle, &status.byte);
        XCTAssertEqual(CANERR_NOERROR, rc);
        XCTAssertTrue(status.can_stopped);
        // @-- tear down DUT1
        rc = can_exit(handle);
        XCTAssertEqual(CANERR_NOERROR, rc);
    }
    // @end.
}

// @xctest TC05.16: Send a CAN FD message with flag BRS set in CAN 2.0 operation mode (FDOE = 0 and BRSE = 0)
//
// @expected: CANERR_ILLPARA
//
- (void)testCheckFlagBrsInCan20OperationMode {
    can_bitrate_t bitrate = { TEST_BTRINDEX };
    can_status_t status = { CANSTAT_RESET };
    can_mode_t capa = { CANMODE_DEFAULT };
    can_mode_t mode = { CANMODE_DEFAULT };
    can_message_t message = {};
    int handle = INVALID_HANDLE;
    int rc = CANERR_FATAL;
    // transmit message
    message.id = 0x30FU;
    message.fdf = mode.fdoe ? 1 : 0;
    message.brs = mode.brse ? 1 : 0;
    message.xtd = 0;
    message.rtr = 0;
    message.esi = 0;
    message.sts = 0;
    message.dlc = mode.fdoe ? CANFD_MAX_DLC : CAN_MAX_DLC;
    memset(message.data, 0, CANFD_MAX_LEN);
    // @pre:
    // @- initialize DUT1 with configured settings
    handle = can_init(DUT1, TEST_CANMODE, TEST_PARAM(PAR1));
    XCTAssertLessThanOrEqual(0, handle);
    // @- get operation capability from DUT1
    rc = can_property(handle, CANPROP_GET_OP_CAPABILITY, (void*)&capa.byte, sizeof(UInt8));
    XCTAssertEqual(CANERR_NOERROR, rc);
    // @- tear down DUT1
    rc = can_exit(handle);
    XCTAssertEqual(CANERR_NOERROR, rc);
    // @- when CAN FD long and fast frames supported:
    if (capa.fdoe && capa.brse) {
        mode.fdoe = mode.brse = 0;
        // @--- initialize DUT1 with operation mode bit FDOE and BRSE cleared
        handle = can_init(DUT1, mode.byte, TEST_PARAM(PAR1));
        XCTAssertLessThanOrEqual(0, handle);
        // @-- get status of DUT1 and check to be in INIT state
        rc = can_status(handle, &status.byte);
        XCTAssertEqual(CANERR_NOERROR, rc);
        XCTAssertTrue(status.can_stopped);
        // @-- start DUT1 with configured bit-rate settings
        rc = can_start(handle, &bitrate);
        XCTAssertEqual(CANERR_NOERROR, rc);
        // @-- get status of DUT1 and check to be in RUNNING state
        rc = can_status(handle, &status.byte);
        XCTAssertEqual(CANERR_NOERROR, rc);
        XCTAssertFalse(status.can_stopped);

        // @test:
        // @-- try to send a message with bit BRS set
        message.fdf = 0;
        message.brs = 1;
        rc = can_write(handle, &message, 0U);
        XCTAssertEqual(CANERR_ILLPARA, rc);
        // @-- get status of DUT1 and check to be in RUNNING state
        rc = can_status(handle, &status.byte);
        XCTAssertEqual(CANERR_NOERROR, rc);
        XCTAssertFalse(status.can_stopped);

        // @post:
        // @-- send and receive some frames to/from DUT2 (optional)
#if (SEND_TEST_FRAMES != 0)
        CTester tester;
        XCTAssertEqual(TEST_FRAMES, tester.SendSomeFrames(handle, DUT2, TEST_FRAMES));
        XCTAssertEqual(TEST_FRAMES, tester.ReceiveSomeFrames(handle, DUT2, TEST_FRAMES));
        // @-- get status of DUT1 and check to be in RUNNING state
        rc = can_status(handle, &status.byte);
        XCTAssertEqual(CANERR_NOERROR, rc);
        XCTAssertFalse(status.can_stopped);
#endif
        // @-- stop/reset DUT1
        rc = can_reset(handle);
        XCTAssertEqual(CANERR_NOERROR, rc);
        // @-- get status of DUT1 and check to be in INIT state
        rc = can_status(handle, &status.byte);
        XCTAssertEqual(CANERR_NOERROR, rc);
        XCTAssertTrue(status.can_stopped);
        // @-- tear down DUT1
        rc = can_exit(handle);
        XCTAssertEqual(CANERR_NOERROR, rc);
    }
    // @end.
}

// @xctest TC05.17: Send a CAN FD message with flag BRS set in CAN FD operation mode (FDOE = 1) but bit-rate switching is not enabled (BRSE = 0)
//
// @expected: CANERR_ILLPARA
//
- (void)testCheckFlagBrsWithoutFlagFdf {
    can_bitrate_t bitrate = { TEST_BTRINDEX };
    can_status_t status = { CANSTAT_RESET };
    can_mode_t capa = { CANMODE_DEFAULT };
    can_mode_t mode = { CANMODE_DEFAULT };
    can_message_t message = {};
    int handle = INVALID_HANDLE;
    int rc = CANERR_FATAL;
    // transmit message
    message.id = 0x310U;
    message.fdf = mode.fdoe ? 1 : 0;
    message.brs = mode.brse ? 1 : 0;
    message.xtd = 0;
    message.rtr = 0;
    message.esi = 0;
    message.sts = 0;
    message.dlc = mode.fdoe ? CANFD_MAX_DLC : CAN_MAX_DLC;
    memset(message.data, 0, CANFD_MAX_LEN);
    // @pre:
    // @- initialize DUT1 with configured settings
    handle = can_init(DUT1, TEST_CANMODE, TEST_PARAM(PAR1));
    XCTAssertLessThanOrEqual(0, handle);
    // @- get operation capability from DUT1
    rc = can_property(handle, CANPROP_GET_OP_CAPABILITY, (void*)&capa.byte, sizeof(UInt8));
    XCTAssertEqual(CANERR_NOERROR, rc);
    // @- tear down DUT1
    rc = can_exit(handle);
    XCTAssertEqual(CANERR_NOERROR, rc);
    // @- when CAN FD long and fast frames supported:
    if (capa.fdoe && capa.brse) {
        mode.fdoe = mode.brse = 1;
        // @-- initialize DUT1 with operation mode bit FDOE and BRSE set
        handle = can_init(DUT1, mode.byte, TEST_PARAM(PAR1));
        XCTAssertLessThanOrEqual(0, handle);
        // @-- get status of DUT1 and check to be in INIT state
        rc = can_status(handle, &status.byte);
        XCTAssertEqual(CANERR_NOERROR, rc);
        XCTAssertTrue(status.can_stopped);
        // @-- start DUT1 with CAN FD bit-rate settings: 250kbps : 2'000kbps
        BITRATE_FD_250K2M(bitrate);
        rc = can_start(handle, &bitrate);
        XCTAssertEqual(CANERR_NOERROR, rc);
        // @-- get status of DUT1 and check to be in RUNNING state
        rc = can_status(handle, &status.byte);
        XCTAssertEqual(CANERR_NOERROR, rc);
        XCTAssertFalse(status.can_stopped);

        // @test:
        // @-- try to send a message with bit FDF cleared and BRS set
        message.fdf = 0;
        message.brs = 1;
        rc = can_write(handle, &message, 0U);
        XCTAssertEqual(CANERR_ILLPARA, rc);
        // @-- get status of DUT1 and check to be in RUNNING state
        rc = can_status(handle, &status.byte);
        XCTAssertEqual(CANERR_NOERROR, rc);
        XCTAssertFalse(status.can_stopped);

        // @post:
        // @-- send and receive some frames to/from DUT2 (optional)
#if (0)
        // FIXME: 2nd device must also be CAN FD capable!
#if (SEND_TEST_FRAMES != 0)
        CTester tester;
        XCTAssertEqual(TEST_FRAMES, tester.SendSomeFrames(handle, DUT2, TEST_FRAMES));
        XCTAssertEqual(TEST_FRAMES, tester.ReceiveSomeFrames(handle, DUT2, TEST_FRAMES));
        // @-- get status of DUT1 and check to be in RUNNING state
        rc = can_status(handle, &status.byte);
        XCTAssertEqual(CANERR_NOERROR, rc);
        XCTAssertFalse(status.can_stopped);
#endif
#endif
        // @-- stop/reset DUT1
        rc = can_reset(handle);
        XCTAssertEqual(CANERR_NOERROR, rc);
        // @-- get status of DUT1 and check to be in INIT state
        rc = can_status(handle, &status.byte);
        XCTAssertEqual(CANERR_NOERROR, rc);
        XCTAssertTrue(status.can_stopped);
        // @-- tear down DUT1
        rc = can_exit(handle);
        XCTAssertEqual(CANERR_NOERROR, rc);
    }
    // @end.
}

// @xctest TC05.18: Send a CAN message with flag STS set (status message)
//
// @expected: CANERR_ILLPARA
//
- (void)testCheckFlagSts {
    can_bitrate_t bitrate = { TEST_BTRINDEX };
    can_status_t status = { CANSTAT_RESET };
    can_mode_t mode = { CANMODE_DEFAULT };
    can_message_t message = {};
    int handle = INVALID_HANDLE;
    int rc = CANERR_FATAL;
    // transmit message
    message.id = 0x311U;
    message.fdf = mode.fdoe ? 1 : 0;
    message.brs = mode.brse ? 1 : 0;
    message.xtd = 0;
    message.rtr = 0;
    message.esi = 0;
    message.sts = 0;
    message.dlc = mode.fdoe ? CANFD_MAX_DLC : CAN_MAX_DLC;
    memset(message.data, 0, CANFD_MAX_LEN);
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
    // @test:
    // @- try to send a message with bit STS set
    message.sts = 1;
    rc = can_write(handle, &message, 0U);
    XCTAssertEqual(CANERR_ILLPARA, rc);
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

// @xctest TC05.19: Send a CAN message if transmitter is busy (transmit queue full)
//
// @expected: CANERR_TX_BUSY but status flag 'transmitter_busy' set
//
// @note: status flag 'transmitter_busy' is only set if the message is acknowledged by the CAN controller.
//
#if (FEATURE_WRITE_ACKNOWLEDGED == FEATURE_SUPPORTED)
- (void)testIfTransmitterIsBusy {
    can_bitrate_t bitrate = { TEST_BTRINDEX };
    can_status_t status = { CANSTAT_RESET };
    can_message_t message1 = {};
    can_mode_t mode = { TEST_CANMODE };
    int handle1 = INVALID_HANDLE;
    int handle2 = INVALID_HANDLE;
    int rc = CANERR_FATAL;
    int i;
    // transmit message
    message1.id = 0x500U;
    message1.fdf = mode.fdoe ? 1 : 0;
    message1.brs = mode.brse ? 1 : 0;
    message1.xtd = 0;
    message1.rtr = 0;
    message1.esi = 0;
    message1.sts = 0;
    message1.dlc = mode.fdoe ? CANFD_MAX_DLC : CAN_MAX_DLC;
    memset(message1.data, 0, CANFD_MAX_LEN);
    // @pre:
    mode.nxtd = 0;
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
    // @- fire a lot of messages from DUT1 until the transmitter tilt
    for (i = 0; i <= TEST_QUEUE_FULL; i++) {
        message1.data[0] = (uint8_t)((uint64_t)i >> 0);
        message1.data[1] = (uint8_t)((uint64_t)i >> 8);
        message1.data[2] = (uint8_t)((uint64_t)i >> 16);
        message1.data[3] = (uint8_t)((uint64_t)i >> 24);
        message1.data[4] = (uint8_t)((uint64_t)i >> 32);
        message1.data[5] = (uint8_t)((uint64_t)i >> 40);
        message1.data[6] = (uint8_t)((uint64_t)i >> 48);
        message1.data[7] = (uint8_t)((uint64_t)i >> 56);
        rc = can_write(handle1, &message1, 0U);
        if (CANERR_NOERROR != rc)
            break;
    }
    NSLog(@"%d frame(s) sent", i);
    XCTAssertEqual(CANERR_TX_BUSY, rc);
    // @- get status of DUT1 and check to be in RUNNING state
    rc = can_status(handle1, &status.byte);
    XCTAssertEqual(CANERR_NOERROR, rc);
    XCTAssertFalse(status.can_stopped);
    // @- check if bit CANSTAT_TX_BUSY is set in status register
    XCTAssertTrue(status.transmitter_busy);
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

// @xctest TC05.20: Send a CAN FD message with flag ESI set (error indicator)
//
// @expected: CANERR_xyz
//
// - (void)testWithFlagEsiInCanFdOperationMode {
//     // @test:
//     // @todo: insert coin here
//     // @end.
// }

// @xctest TC05.21: Send a CAN FD message with flag STS set (status message)
//
// @expected: CANERR_xyz
//
// - (void)tesWithFlagStsInCanFdOperationMode {
//     // @test:
//     // @todo: insert coin here
//     // @end.
// }

// @xctest TC05.22: Send a CAN FD message if transmitter is busy (transmit queue full)
//
// @expected: CANERR_xyz
//
// - (void)tesIfTransmitterIsBusyInCanFdOperationMode {
//     // @test:
//     // @todo: insert coin here
//     // @end.
// }

@end

// $Id: test_can_write.mm 1341 2024-06-15 16:43:48Z makemake $  Copyright (c) UV Software, Berlin //
