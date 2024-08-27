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

#ifndef FEATURE_BITRATE_800K
#define FEATURE_BITRATE_800K  FEATURE_SUPPORTED
#warning FEATURE_BITRATE_800K not set, default=FEATURE_SUPPORTED
#endif

#ifndef FEATURE_BITRATE_SJA1000
#define FEATURE_BITRATE_SJA1000  FEATURE_SUPPORTED
#warning FEATURE_BITRATE_SJA1000 not set, default=FEATURE_SUPPORTED
#endif

#ifndef FEATURE_BITRATE_SAM
#define FEATURE_BITRATE_SAM  FEATURE_SUPPORTED
#warning FEATURE_BITRATE_SAM not set, default=FEATURE_SUPPORTED
#endif

#ifndef FEATURE_BITRATE_FD_SAM
#define FEATURE_BITRATE_FD_SAM  FEATURE_UNSUPPORTED
#warning FEATURE_BITRATE_FD_SAM not set, default=FEATURE_UNSUPPORTED
#endif

#ifndef FEATURE_BITRATE_FD_SJA1000
#define FEATURE_BITRATE_FD_SJA1000  FEATURE_UNSUPPORTED
#warning FEATURE_BITRATE_FD_SJA1000 not set, default=FEATURE_UNSUPPORTED
#endif

#define NOM_BRP_MIN    CANBTR_NOMINAL_BRP_MIN
#define NOM_BRP_MAX    CANBTR_NOMINAL_BRP_MAX
#define NOM_TSEG1_MIN  CANBTR_NOMINAL_TSEG1_MIN
#define NOM_TSEG1_MAX  CANBTR_NOMINAL_TSEG1_MAX
#define NOM_TSEG2_MIN  CANBTR_NOMINAL_TSEG2_MIN
#define NOM_TSEG2_MAX  CANBTR_NOMINAL_TSEG2_MAX
#define NOM_SJW_MIN    CANBTR_NOMINAL_SJW_MIN
#define NOM_SJW_MAX    CANBTR_NOMINAL_SJW_MAX

#define DATA_BRP_MIN    CANBTR_DATA_BRP_MIN
#define DATA_BRP_MAX    CANBTR_DATA_BRP_MAX
#define DATA_TSEG1_MIN  CANBTR_DATA_TSEG1_MIN
#define DATA_TSEG1_MAX  CANBTR_DATA_TSEG1_MAX
#define DATA_TSEG2_MIN  CANBTR_DATA_TSEG2_MIN
#define DATA_TSEG2_MAX  CANBTR_DATA_TSEG2_MAX
#define DATA_SJW_MIN    CANBTR_DATA_SJW_MIN
#define DATA_SJW_MAX    CANBTR_DATA_SJW_MAX

@interface test_can_start : XCTestCase

@end

@implementation test_can_start

- (void)setUp {
    // Put setup code here. This method is called before the invocation of each test method in the class.
}

- (void)tearDown {
    // Put teardown code here. This method is called after the invocation of each test method in the class.
    (void)can_exit(CANKILL_ALL);
}

// @xctest TC03.0: Start CAN controller (sunnyday scenario)
//
// @expected: CANERR_NOERROR
//
// - (void)testSunnydayScenario {
//     // @test:
//     // @todo: insert coin here
//     // @end.
// }

// @xctest TC03.1: Start CAN controller with invalid channel handle(s)
//
// @expected: CANERR_HANDLE
//
- (void)testWithInvalidHandle {
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
    // @test:
    // @- try to start DUT1 with invalid handle -1
    rc = can_start(INVALID_HANDLE, &bitrate);
    XCTAssertEqual(CANERR_HANDLE, rc);
    // @- try to start DUT1 with invalid handle INT32_MIN
    rc = can_start(INT32_MIN, &bitrate);
    XCTAssertEqual(CANERR_HANDLE, rc);
    // @- try to start DUT1 with invalid handle INT32_MAX
    rc = can_start(INT32_MAX, &bitrate);
    XCTAssertEqual(CANERR_HANDLE, rc);
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

// @xctest TC03.2: Give a NULL pointer as argument for parameter 'bitrate'
//
// @expected: CANERR_NULLPTR
//
- (void)testWithNullPointerForBitrate {
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
    // @test:
    // @- try to start DUT1 with NULL pointer for parameter 'bitrate'
    rc = can_start(handle, NULL);
    XCTAssertEqual(CANERR_NULLPTR, rc);
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

// @xctest TC03.3: Start CAN controller if CAN channel is not initialized
//
// @expected: CANERR_NOTINIT
//
- (void)testIfChannelNotInitialized {
    can_bitrate_t bitrate = { TEST_BTRINDEX };
    can_status_t status = { CANSTAT_RESET };
    int handle = INVALID_HANDLE;
    int rc = CANERR_FATAL;
    // @test:
    // @- try to start DUT1 with configured bit-rate settings
    rc = can_start(DUT1, &bitrate);
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

// @xctest TC03.4: Start CAN controller if CAN controller is already started
//
// @expected: CANERR_ONLINE
//
- (void)testIfControllerStarted {
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
    // @- get status of DUT1 and check to be in RUNNING state
    rc = can_status(handle, &status.byte);
    XCTAssertEqual(CANERR_NOERROR, rc);
    XCTAssertFalse(status.can_stopped);
    // @test:
    // @- try to start DUT1 again with configured bit-rate settings
    rc = can_start(handle, &bitrate);
    XCTAssertEqual(CANERR_ONLINE, rc);
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

// @xctest TC03.5: Start CAN controller if CAN controller was previously stopped
//
// @expected: CANERR_NOERROR
//
- (void)testIfControllerStopped {
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
    // @test:
    // @- start DUT1 again with configured bit-rate settings
    rc = can_start(handle, &bitrate);
    XCTAssertEqual(CANERR_NOERROR, rc);
    // @- get status of DUT1 and check to be in RUNNING state
    rc = can_status(handle, &status.byte);
    XCTAssertEqual(CANERR_NOERROR, rc);
    XCTAssertFalse(status.can_stopped);
    // @post:
    // @- send and receive some frames to/from DUT2 (optional)
#if (SEND_TEST_FRAMES != 0)
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

// @xctest TC03.6: Start CAN controller if CAN channel was previously torn down
//
// @expected: CANERR_NOTINIT
//
- (void)testIfChannelTornDown {
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
    // @- try to start DUT1 again with configured bit-rate settings
    rc = can_start(handle, &bitrate);
    XCTAssertEqual(CANERR_NOTINIT, rc);
    // @end.
}

// @xctest TC03.7: Start CAN controller with valid CAN 2.0 bit-timing indexes
//
// @remarks: TC03.8 to TC03.15 joined into TC03.7
//
// @expected: CANERR_NOERROR
//
- (void)testWithValidCanBitrateIndex {
    can_bitrate_t bitrate = { CANBTR_INDEX_1M };
    can_status_t status = { CANSTAT_RESET };
    int handle = INVALID_HANDLE;
    int rc = CANERR_FATAL;
    // @loop over selected CAN 2.0 bit-timing indexes
    // @note: predefined BTR0BTR1 bit-timing table has 10 entries, index 0 to 9.
    // @      The index must be given as negative value to 'bitrate.index'!
    // @      Remark: The CiA bit-timing table has only 9 entries!
    for (int i = 0; i < 10; i++) {
        switch (i) {
            // @sub(1): index 0 (1Mbps)
            case 0: bitrate.index = CANBTR_INDEX_1M; break;
            // @sub(2): index 1 (800kbps, not supported by all CAN controllers)
            case 1: bitrate.index = CANBTR_INDEX_800K; break;
            // @sub(3): index 2 (500kbps)
            case 2: bitrate.index = CANBTR_INDEX_500K; break;
            // @sub(4): index 3 (250kbps)
            case 3: bitrate.index = CANBTR_INDEX_250K; break;
            // @sub(5): index 4 (125kbps)
            case 4: bitrate.index = CANBTR_INDEX_125K; break;
            // @sub(6): index 5 (100kbps)
            case 5: bitrate.index = CANBTR_INDEX_100K; break;
            // @sub(7): index 6 (50kbps)
            case 6: bitrate.index = CANBTR_INDEX_50K; break;
            // @sub(8): index 7 (20kbps)
            case 7: bitrate.index = CANBTR_INDEX_20K; break;
            // @sub(9): index 8 (10kbps)
            case 8: bitrate.index = CANBTR_INDEX_10K; break;
            // @sub(10):    index 9 (5kbps, not supported by all CAN API SDK's)
            case 9: bitrate.index = SJA1000_INDEX_5K; break;
            default: return;  // Get out of here!
        }
#if (FEATURE_BITRATE_800K != FEATURE_SUPPORTED)
        if (bitrate.index == CANBTR_INDEX_800K)
            continue;
#endif
#if (FEATURE_BITRATE_SJA1000 != FEATURE_SUPPORTED) || (SERIAL_CAN_SUPPORTED != 0)
        if (bitrate.index == SJA1000_INDEX_5K)
            continue;
#endif
#if (TC03_7_ISSUE_TOUCAN_BITRATE_10K == WORKAROUND_ENABLED)
        if (bitrate.index == CANBTR_INDEX_10K) {
            NSLog(@"Sub-testcase %d skipped due to known hardware bug\n", i+1);
            continue;
        }
#endif
        NSLog(@"Execute sub-testcase %d:\n", i+1);
        // @pre:
        // @-- initialize DUT1 in CAN 2.0 operation mode
        handle = can_init(DUT1, CANMODE_DEFAULT, TEST_PARAM(PAR1));
        XCTAssertLessThanOrEqual(0, handle);
        // @-- get status of DUT1 and check to be in INIT state
        rc = can_status(handle, &status.byte);
        XCTAssertEqual(CANERR_NOERROR, rc);
        XCTAssertTrue(status.can_stopped);
        // @test:
        // @-- start DUT1 with selected index from CiA table
        rc = can_start(handle, &bitrate);
        XCTAssertEqual(CANERR_NOERROR, rc);
        // @-- get status of DUT1 and check to be in RUNNING state
        rc = can_status(handle, &status.byte);
        XCTAssertEqual(CANERR_NOERROR, rc);
        XCTAssertFalse(status.can_stopped);
        // @post:
        // @-- send and receive some frames to/from DUT2 (optional)
#if (SEND_TEST_FRAMES != 0) && (SEND_WITH_NONE_DEFAULT_BAUDRATE != 0)
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

// @xctest TC03.16: Start CAN controller with invalid CAN 2.0 bit-timing indexes
//
// @expected: CANERR_BAUDRATE
//
- (void)testWithInvalidCanBitrateIndex {
    can_bitrate_t bitrate = { TEST_BTRINDEX };
    can_status_t status = { CANSTAT_RESET };
    int handle = INVALID_HANDLE;
    int rc = CANERR_FATAL;
    // @pre:
    // @- initialize DUT1 in CAN 2.0 operation mode
    handle = can_init(DUT1, CANMODE_DEFAULT, TEST_PARAM(PAR1));
    XCTAssertLessThanOrEqual(0, handle);
    // @- get status of DUT1 and check to be in INIT state
    rc = can_status(handle, &status.byte);
    XCTAssertEqual(CANERR_NOERROR, rc);
    XCTAssertTrue(status.can_stopped);
    // @test: loop over invalid CiA bit-timing indexes
    // @note: predefined BTR0BTR1 bit-timing table has 10 entries, index 0 to 9.
    // @      The index must be given as negative value to 'bitrate.index'!
    // @      Remark: The CiA bit-timing table has only 9 entries!
    // @note: Positive values represent the CAN clock in Hertz, but there will
    // @      be probably no clock below 10 Hertz (or above 999'999'999 Hertz).
    for (int i = 0; i < 14; i++) {
        switch (i) {
            // @sub(1): invalid index -10
#if (FEATURE_BITRATE_SJA1000 == FEATURE_SUPPORTED)
            case 0: bitrate.index = SJA1000_INDEX_5K - 1; break;
#else
            case 0: bitrate.index = CANBTR_INDEX_10K - 1; break;
#endif
            // @sub(2): invalid index INT8_MIN
            case 1: bitrate.index = INT8_MIN; break;
            // @sub(3): invalid index INT16_MIN
            case 2: bitrate.index = INT16_MIN; break;
            // @sub(4): invalid index INT32_MIN+1
            case 3: bitrate.index = INT32_MIN+1; break;
            // @sub(5): invalid index INT32_MIN
            case 4: bitrate.index = INT32_MIN; break;
            // @sub(6): invalid index 1
            case 5: bitrate.index = CANBDR_800; XCTAssertEqual(1, bitrate.index); break;
            // @sub(7): invalid index 2
            case 6: bitrate.index = CANBDR_500; XCTAssertEqual(2, bitrate.index); break;
            // @sub(8): invalid index 3
            case 7: bitrate.index = CANBDR_250; XCTAssertEqual(3, bitrate.index); break;
            // @sub(9): invalid index 4
            case 8: bitrate.index = CANBDR_125; XCTAssertEqual(4, bitrate.index); break;
            // @sub(10): invalid index 5
            case 9: bitrate.index = CANBDR_100; XCTAssertEqual(5, bitrate.index); break;
            // @sub(11): invalid index 6
            case 10: bitrate.index = CANBDR_50; XCTAssertEqual(6, bitrate.index); break;
            // @sub(12): invalid index 7
            case 11: bitrate.index = CANBDR_20; XCTAssertEqual(7, bitrate.index); break;
            // @sub(13): invalid index 8
            case 12: bitrate.index = CANBDR_10; XCTAssertEqual(8, bitrate.index); break;
            // @sub(14): invalid index INT32_MAX
            case 13: bitrate.index = INT32_MAX; break;
            default: return;  // Get out of here!
        }
        NSLog(@"Execute sub-testcase %d:\n", i+1);
        // @-- try to start DUT1 with invalid index
        rc = can_start(handle, &bitrate);
        XCTAssertEqual(CANERR_BAUDRATE, rc);
        // @-- get status of DUT1 and check to be in INIT state
        rc = can_status(handle, &status.byte);
        XCTAssertEqual(CANERR_NOERROR, rc);
        XCTAssertTrue(status.can_stopped);
        // @note: stop/reset DUT1 if started anyway
        if (!status.can_stopped)
            (void)can_reset(handle);
    }
    // @post:
    bitrate.index = TEST_BTRINDEX;
    // @- start DUT1 with configured bit-rate settings
    rc = can_start(handle, &bitrate);
    XCTAssertEqual(CANERR_NOERROR, rc);
    // @- send and receive some frames to/from DUT2 (optional)
#if (SEND_TEST_FRAMES != 0) && (SEND_WITH_NONE_DEFAULT_BAUDRATE != 0)
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

// @xctest TC03.17: Re-Start CAN controller with the same CAN 2.0 bit-timing index
//
// @expected: CANERR_NOERROR
//
- (void)testWithSameCanBitrateIndexAfterCanStopped {
    can_bitrate_t bitrate = { TEST_BTRINDEX };
    can_status_t status = { CANSTAT_RESET };
    int handle = INVALID_HANDLE;
    int rc = CANERR_FATAL;
    // @test: loop over selected CAN 2.0 bit-timing indexes
    // @note: predefined BTR0BTR1 bit-timing table has 10 entries, index 0 to 9.
    // @      The index must be given as negative value to 'bitrate.index'!
    // @      Remark: The CiA bit-timing table has only 9 entries!
    for (int i = 0; i < 10; i++) {
        switch (i) {
            // @sub(1): index 0 (1Mbps)
            case 0: bitrate.index = CANBTR_INDEX_1M; break;
            // @sub(2): index 1 (800kbps, not supported by all CAN controllers)
            case 1: bitrate.index = CANBTR_INDEX_800K; break;
            // @sub(3): index 2 (500kbps)
            case 2: bitrate.index = CANBTR_INDEX_500K; break;
            // @sub(4): index 3 (250kbps)
            case 3: bitrate.index = CANBTR_INDEX_250K; break;
            // @sub(5): index 4 (125kbps)
            case 4: bitrate.index = CANBTR_INDEX_125K; break;
            // @sub(6): index 5 (100kbps)
            case 5: bitrate.index = CANBTR_INDEX_100K; break;
            // @sub(7): index 6 (50kbps)
            case 6: bitrate.index = CANBTR_INDEX_50K; break;
            // @sub(8): index 7 (20kbps)
            case 7: bitrate.index = CANBTR_INDEX_20K; break;
            // @sub(9): index 8 (10kbps)
            case 8: bitrate.index = CANBTR_INDEX_10K; break;
            // @sub(10):    index 9 (5kbps, not supported by all CAN API SDK's)
            case 9: bitrate.index = SJA1000_INDEX_5K; break;
            default: return;  // Get out of here!
        }
#if (FEATURE_BITRATE_800K != FEATURE_SUPPORTED)
        if (bitrate.index == CANBTR_INDEX_800K)
            continue;
#endif
#if (FEATURE_BITRATE_SJA1000 != FEATURE_SUPPORTED) || (SERIAL_CAN_SUPPORTED != 0)
        if (bitrate.index == SJA1000_INDEX_5K)
            continue;
#endif
#if (TC03_17_ISSUE_TOUCAN_BITRATE_10K == WORKAROUND_ENABLED)
        if (bitrate.index == CANBTR_INDEX_10K) {
            NSLog(@"Sub-testcase %d skipped due to known hardware bug\n", i+1);
            continue;
        }
#endif
        NSLog(@"Execute sub-testcase %d:\n", i+1);
        // @-- initialize DUT1 in CAN 2.0 operation mode
        handle = can_init(DUT1, CANMODE_DEFAULT, TEST_PARAM(PAR1));
        XCTAssertLessThanOrEqual(0, handle);
        // @-- get status of DUT1 and check to be in INIT state
        rc = can_status(handle, &status.byte);
        XCTAssertEqual(CANERR_NOERROR, rc);
        XCTAssertTrue(status.can_stopped);
        // @-- start DUT1 with selected bit-rate settings
        rc = can_start(handle, &bitrate);
        XCTAssertEqual(CANERR_NOERROR, rc);
        // @-- get status of DUT1 and check to be in RUNNING state
        rc = can_status(handle, &status.byte);
        XCTAssertEqual(CANERR_NOERROR, rc);
        XCTAssertFalse(status.can_stopped);
        // @-- send and receive some frames to/from DUT2 (optional)
#if (SEND_TEST_FRAMES != 0) && (SEND_WITH_NONE_DEFAULT_BAUDRATE != 0)
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
        // @-- start DUT1 again with selected bit-rate settings
        rc = can_start(handle, &bitrate);
        XCTAssertEqual(CANERR_NOERROR, rc);
        // @-- get status of DUT1 and check to be in RUNNING state
        rc = can_status(handle, &status.byte);
        XCTAssertEqual(CANERR_NOERROR, rc);
        XCTAssertFalse(status.can_stopped);
        // @-- send and receive some frames to/from DUT2 (optional)
#if (SEND_TEST_FRAMES != 0) && (SEND_WITH_NONE_DEFAULT_BAUDRATE != 0)
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

// @xctest TC03.18: Re-Start CAN controller with a different CAN 2.0 bit-timing index
//
// @expected: CANERR_NOERROR
//
- (void)testWithDifferentCanBitrateIndexAfterCanStopped {
    can_bitrate_t bitrate = { TEST_BTRINDEX };
    can_status_t status = { CANSTAT_RESET };
    int handle = INVALID_HANDLE;
    int rc = CANERR_FATAL;
    // @test: loop over CiA bit-timing table indexes 0 to 8
    for (int i = 0; i < 9; i++) {
        switch (i) {
            // @sub(1): index 0 (1Mbps)
            case 0: bitrate.index = CANBTR_INDEX_1M; break;
            // @sub(2): index 1 (800kbps, not supported by all CAN controllers)
            case 1: bitrate.index = CANBTR_INDEX_800K; break;
            // @sub(3): index 2 (500kbps)
            case 2: bitrate.index = CANBTR_INDEX_500K; break;
            // @sub(4): index 3 (250kbps)
            case 3: bitrate.index = CANBTR_INDEX_250K; break;
            // @sub(5): index 4 (125kbps)
            case 4: bitrate.index = CANBTR_INDEX_125K; break;
            // @sub(6): index 5 (100kbps)
            case 5: bitrate.index = CANBTR_INDEX_100K; break;
            // @sub(7): index 6 (50kbps)
            case 6: bitrate.index = CANBTR_INDEX_50K; break;
            // @sub(8): index 7 (20kbps)
            case 7: bitrate.index = CANBTR_INDEX_20K; break;
            // @sub(9): index 8 (10kbps)
            case 8: bitrate.index = CANBTR_INDEX_10K; break;
            default: return;  // Get out of here!
        }
#if (FEATURE_BITRATE_800K != FEATURE_SUPPORTED)
        if (bitrate.index == CANBTR_INDEX_800K)
            continue;
#endif
#if (TC03_18_ISSUE_TOUCAN_BITRATE_10K == WORKAROUND_ENABLED)
        if (bitrate.index == CANBTR_INDEX_10K) {
            NSLog(@"Sub-testcase %d skipped due to known hardware bug\n", i+1);
            continue;
        }
#endif
        NSLog(@"Execute sub-testcase %d:\n", i+1);
        // @-- initialize DUT1 in CAN 2.0 operation mode
        handle = can_init(DUT1, CANMODE_DEFAULT, TEST_PARAM(PAR1));
        XCTAssertLessThanOrEqual(0, handle);
        // @-- get status of DUT1 and check to be in INIT state
        rc = can_status(handle, &status.byte);
        XCTAssertEqual(CANERR_NOERROR, rc);
        XCTAssertTrue(status.can_stopped);
        // @-- start DUT1 with sekected bit-rate settings
        rc = can_start(handle, &bitrate);
        XCTAssertEqual(CANERR_NOERROR, rc);
        // @-- get status of DUT1 and check to be in RUNNING state
        rc = can_status(handle, &status.byte);
        XCTAssertEqual(CANERR_NOERROR, rc);
        XCTAssertFalse(status.can_stopped);
        // @-- send and receive some frames to/from DUT2 (optional)
#if (SEND_TEST_FRAMES != 0) && (SEND_WITH_NONE_DEFAULT_BAUDRATE != 0)
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
        // @-- new CiA bit-timing table index = 8 - old
        switch (i) {
            case 8: bitrate.index = CANBTR_INDEX_1M; break;
#if (FEATURE_BITRATE_800K == FEATURE_SUPPORTED)
            case 7: bitrate.index = CANBTR_INDEX_800K; break;
#else
            case 7: bitrate.index = CANBTR_INDEX_500K; break;
#endif
            case 6: bitrate.index = CANBTR_INDEX_500K; break;
            case 5: bitrate.index = CANBTR_INDEX_250K; break;
            case 4: bitrate.index = CANBTR_INDEX_125K; break;
            case 3: bitrate.index = CANBTR_INDEX_100K; break;
            case 2: bitrate.index = CANBTR_INDEX_50K; break;
            case 1: bitrate.index = CANBTR_INDEX_20K; break;
#if (TC03_18_ISSUE_TOUCAN_BITRATE_10K != WORKAROUND_ENABLED)
            case 0: bitrate.index = CANBTR_INDEX_10K; break;
#else
            case 0: bitrate.index = CANBTR_INDEX_20K; break;
#endif
            default: return;  // Get out of here!
        }
        // @-- start DUT1 again with a different bit-rate settings
        rc = can_start(handle, &bitrate);
        XCTAssertEqual(CANERR_NOERROR, rc);
        // @-- get status of DUT1 and check to be in RUNNING state
        rc = can_status(handle, &status.byte);
        XCTAssertEqual(CANERR_NOERROR, rc);
        XCTAssertFalse(status.can_stopped);
        // @-- send and receive some frames to/from DUT2 (optional)
#if (SEND_TEST_FRAMES != 0) && (SEND_WITH_NONE_DEFAULT_BAUDRATE != 0)
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

// @xctest TC03.19: Start CAN controller with valid CAN 2.0 bit-rate settings
//
// @expected: CANERR_NOERROR
//
- (void)testWithValidCanBitrateSettings {
    can_bitrate_t bitrate = { TEST_BTRINDEX };
    can_status_t status = { CANSTAT_RESET };
    int handle = INVALID_HANDLE;
    int rc = CANERR_FATAL;
    // @test: loop over selected CAN 2.0 bit-rate settings
    for (int i = 0; i < 8; i++) {
        memset(&bitrate, 0, sizeof(can_bitrate_t));
        switch (i) {
            // @sub(1): 1Mbps
            case 0: BITRATE_1M(bitrate); break;
            // @sub(2): 500kbps
            case 1: BITRATE_500K(bitrate); break;
            // @sub(3): 250kbps
            case 2: BITRATE_250K(bitrate); break;
            // @sub(4): 125kbps
            case 3: BITRATE_125K(bitrate); break;
            // @sub(5): 100kbps
            case 4: BITRATE_100K(bitrate); break;
            // @sub(6): 50kbps
            case 5: BITRATE_50K(bitrate); break;
            // @sub(7): 20kbps
            case 6: BITRATE_20K(bitrate); break;
            // @sub(8): 10kbps
            case 7: BITRATE_10K(bitrate); break;
            default: return;  // Get out of here!
        }
#if (TC03_19_ISSUE_TOUCAN_BITRATE_10K == WORKAROUND_ENABLED)
        if (i == 7) {
            NSLog(@"Sub-testcase %d skipped due to known hardware bug\n", i+1);
            continue;
        }
#endif
        NSLog(@"Execute sub-testcase %d:\n", i+1);
        // @-- initialize DUT1 in CAN 2.0 operation mode
        handle = can_init(DUT1, CANMODE_DEFAULT, TEST_PARAM(PAR1));
        XCTAssertLessThanOrEqual(0, handle);
        // @-- get status of DUT1 and check to be in INIT state
        rc = can_status(handle, &status.byte);
        XCTAssertEqual(CANERR_NOERROR, rc);
        XCTAssertTrue(status.can_stopped);
        // @-- start DUT1 with selected bit-rate settings
        rc = can_start(handle, &bitrate);
        XCTAssertEqual(CANERR_NOERROR, rc);
        // @-- get status of DUT1 and check to be in RUNNING state
        rc = can_status(handle, &status.byte);
        XCTAssertEqual(CANERR_NOERROR, rc);
        XCTAssertFalse(status.can_stopped);
        // @-- send and receive some frames to/from DUT2 (optional)
#if (SEND_TEST_FRAMES != 0) && (SEND_WITH_NONE_DEFAULT_BAUDRATE != 0)
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

// @xctest TC03.20: Start CAN controller with invalid CAN 2.0 bit-rate settings
//
// @expected: CANERR_BAUDRATE
//
- (void)testWithInvalidCanBitrateSettings {
    can_bitrate_t bitrate = { TEST_BTRINDEX };
    can_status_t status = { CANSTAT_RESET };
    int handle = INVALID_HANDLE;
    int rc = CANERR_FATAL;
    // @pre:
    // @- initialize DUT1 in CAN 2.0 operation mode
    handle = can_init(DUT1, CANMODE_DEFAULT, TEST_PARAM(PAR1));
    XCTAssertLessThanOrEqual(0, handle);
    // @- get status of DUT1 and check to be in INIT state
    rc = can_status(handle, &status.byte);
    XCTAssertEqual(CANERR_NOERROR, rc);
    XCTAssertTrue(status.can_stopped);
    // @test: loop over selected CAN 2.0 bit-rate settings
    for (int i = 0; i < 18; i++) {
        memset(&bitrate, 0, sizeof(can_bitrate_t));
        BITRATE_250K(bitrate);
        switch (i) {
            // @sub(1): set all fields to 0 (note: 'frequency' == 0 is CiA Index 0, set to 1 instead)
            case 0: bitrate.btr.frequency = 1; bitrate.btr.nominal.brp = 0; bitrate.btr.nominal.tseg1 = 0; bitrate.btr.nominal.tseg2 = 0; bitrate.btr.nominal.sjw = 0; bitrate.btr.nominal.sam = 0; break;
            // @sub(2): set all fields to MAX (acc. data type)
            case 1: bitrate.btr.frequency = INT32_MAX; bitrate.btr.nominal.brp = UINT16_MAX; bitrate.btr.nominal.tseg1 = UINT16_MAX; bitrate.btr.nominal.tseg2 = UINT16_MAX; bitrate.btr.nominal.sjw = UINT16_MAX; bitrate.btr.nominal.sam = UINT8_MAX; break;
            // @sub(3): set field 'frequency' to 1 (note: see above)
            case 2: bitrate.btr.frequency = 1; break;
            // @sub(4): set field 'frequency' to INT32_MAX
            case 3: bitrate.btr.frequency = INT32_MAX; break;
            // @sub(5): set field 'brp' to 0
            case 4: bitrate.btr.nominal.brp = 0; break;
            // @sub(6): set field 'brp' to 1025
            case 5: bitrate.btr.nominal.brp = NOM_BRP_MAX+1; break;
            // @sub(7): set field 'brp' to UINT16_MAX
            case 6: bitrate.btr.nominal.brp = UINT16_MAX; break;
            // @sub(8): set field 'tseg1' to 0
            case 7: bitrate.btr.nominal.tseg1 = 0; break;
            // @sub(9): set field 'tseg1' to 257
            case 8: bitrate.btr.nominal.tseg1 = NOM_TSEG1_MAX+1; break;
            // @sub(10): set field 'tseg1' to UINT16_MAX
            case 9: bitrate.btr.nominal.tseg1 = UINT16_MAX; break;
            // @sub(11): set field 'tseg2' to 0
            case 10: bitrate.btr.nominal.tseg2 = 0; break;
            // @sub(12): set field 'tseg2' to 129
            case 11: bitrate.btr.nominal.tseg2 = NOM_TSEG2_MAX+1; break;
            // @sub(13): set field 'tseg2' to UINT16_MAX
            case 12: bitrate.btr.nominal.tseg2 = UINT16_MAX; break;
            // @sub(14): set field 'sjw' to 0
            case 13: bitrate.btr.nominal.sjw = 0; break;
            // @sub(15): set field 'sjw' to 129
            case 14: bitrate.btr.nominal.sjw = NOM_SJW_MAX+1; break;
            // @sub(16): set field 'sjw' to UINT16_MAX
            case 15: bitrate.btr.nominal.sjw = UINT16_MAX; break;
            // @sub(17): set field 'sam' to 2 (note: SAM not supported by all CAN controller)
#if (TC03_20_ISSUE_KVASER_NOSAMP != WORKAROUND_ENABLED)
            // @issue(KvaserCAN): only SAM = 0 supported by Kvaser devices (noSamp = 1)
            case 16: bitrate.btr.nominal.sam = 2; break;
#else
            // @workaround: tread SAM = 1 as invalid (noSamp = 3)
            case 16: bitrate.btr.nominal.sam = 1; break;
#endif
            // @sub(18): set field 'sam' to UINT8_MAX (note: SAM not supported by all CAN controller)
            case 17: bitrate.btr.nominal.sam = UINT8_MAX; break;
            default: return;  // Get out of here!
        }
#if (FEATURE_BITRATE_SAM != FEATURE_SUPPORTED)
        if ((i == 16) || (i == 17))
            continue;
#endif
        NSLog(@"Execute sub-testcase %d:\n", i+1);
        // @-- try to start DUT1 with invalid CAN 2.0 bit-rate settings
        rc = can_start(handle, &bitrate);
        XCTAssertEqual(CANERR_BAUDRATE, rc);
        // @-- get status of DUT1 and check to be in INIT state
        rc = can_status(handle, &status.byte);
        XCTAssertEqual(CANERR_NOERROR, rc);
        XCTAssertTrue(status.can_stopped);
        // @note: stop/reset DUT1 if started anyway
        if (!status.can_stopped)
            (void)can_reset(handle);
    }
    // @post:
    memset(&bitrate, 0, sizeof(can_bitrate_t));
    BITRATE_250K(bitrate);
    // @- start DUT1 with configured bit-rate settings
    rc = can_start(handle, &bitrate);
    XCTAssertEqual(CANERR_NOERROR, rc);
    // @- send and receive some frames to/from DUT2 (optional)
#if (SEND_TEST_FRAMES != 0) && (SEND_WITH_NONE_DEFAULT_BAUDRATE != 0)
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

// @xctest TC03.21: Re-Start CAN controller with same CAN 2.0 bit-rate settings
//
// @expected: CANERR_NOERROR
//
- (void)testWithSameCanBitrateSettingsAfterCanStopped {
    can_bitrate_t bitrate = { TEST_BTRINDEX };
    can_status_t status = { CANSTAT_RESET };
    int handle = INVALID_HANDLE;
    int rc = CANERR_FATAL;
    // @test: loop over selected CAN 2.0 bit-rate settings
    for (int i = 0; i < 8; i++) {
        memset(&bitrate, 0, sizeof(can_bitrate_t));
        switch (i) {
            // @sub(1): 1Mbps
            case 0: BITRATE_1M(bitrate); break;
            // @sub(2): 500kbps
            case 1: BITRATE_500K(bitrate); break;
            // @sub(3): 250kbps
            case 2: BITRATE_250K(bitrate); break;
            // @sub(4): 125kbps
            case 3: BITRATE_125K(bitrate); break;
            // @sub(5): 100kbps
            case 4: BITRATE_100K(bitrate); break;
            // @sub(6): 50kbps
            case 5: BITRATE_50K(bitrate); break;
            // @sub(7): 20kbps
            case 6: BITRATE_20K(bitrate); break;
            // @sub(8): 10kbps
            case 7: BITRATE_10K(bitrate); break;
            default: return;  // Get out of here!
        }
#if (TC03_21_ISSUE_TOUCAN_BITRATE_10K == WORKAROUND_ENABLED)
        if (i == 7) {
            NSLog(@"Sub-testcase %d skipped due to known hardware bug\n", i+1);
            continue;
        }
#endif
        NSLog(@"Execute sub-testcase %d:\n", i+1);
        // @-- initialize DUT1 in CAN 2.0 operation mode
        handle = can_init(DUT1, CANMODE_DEFAULT, TEST_PARAM(PAR1));
        XCTAssertLessThanOrEqual(0, handle);
        // @-- get status of DUT1 and check to be in INIT state
        rc = can_status(handle, &status.byte);
        XCTAssertEqual(CANERR_NOERROR, rc);
        XCTAssertTrue(status.can_stopped);
        // @-- start DUT1 with selected bit-rate settings
        rc = can_start(handle, &bitrate);
        XCTAssertEqual(CANERR_NOERROR, rc);
        // @-- get status of DUT1 and check to be in RUNNING state
        rc = can_status(handle, &status.byte);
        XCTAssertEqual(CANERR_NOERROR, rc);
        XCTAssertFalse(status.can_stopped);
        // @-- send and receive some frames to/from DUT2 (optional)
#if (SEND_TEST_FRAMES != 0) && (SEND_WITH_NONE_DEFAULT_BAUDRATE != 0)
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
        // @-- start DUT1 again with selected bit-rate settings
        rc = can_start(handle, &bitrate);
        XCTAssertEqual(CANERR_NOERROR, rc);
        // @-- get status of DUT1 and check to be in RUNNING state
        rc = can_status(handle, &status.byte);
        XCTAssertEqual(CANERR_NOERROR, rc);
        XCTAssertFalse(status.can_stopped);
        // @-- send and receive some frames to/from DUT2 (optional)
#if (SEND_TEST_FRAMES != 0) && (SEND_WITH_NONE_DEFAULT_BAUDRATE != 0)
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

// @xctest TC03.22: Re-Start CAN controller with different CAN 2.0 bit-rate settings
//
// @expected: CANERR_NOERROR
//
- (void)testWithDifferentCanBitrateSettingsAfterCanStopped {
    can_bitrate_t bitrate = { TEST_BTRINDEX };
    can_status_t status = { CANSTAT_RESET };
    int handle = INVALID_HANDLE;
    int rc = CANERR_FATAL;
    // @test: loop over selected CAN 2.0 bit-rate settings
    for (int i = 0; i < 8; i++) {
        memset(&bitrate, 0, sizeof(can_bitrate_t));
        switch (i) {
            // @sub(1): 1Mbps
            case 0: BITRATE_1M(bitrate); break;
            // @sub(2): 500kbps
            case 1: BITRATE_500K(bitrate); break;
            // @sub(3): 250kbps
            case 2: BITRATE_250K(bitrate); break;
            // @sub(4): 125kbps
            case 3: BITRATE_125K(bitrate); break;
            // @sub(5): 100kbps
            case 4: BITRATE_100K(bitrate); break;
            // @sub(6): 50kbps
            case 5: BITRATE_50K(bitrate); break;
            // @sub(7): 20kbps
            case 6: BITRATE_20K(bitrate); break;
            // @sub(8): 10kbps
            case 7: BITRATE_10K(bitrate); break;
            default: return;  // Get out of here!
        }
#if (TC03_22_ISSUE_TOUCAN_BITRATE_10K == WORKAROUND_ENABLED)
        if (i == 7) {
            NSLog(@"Sub-testcase %d skipped due to known hardware bug\n", i+1);
            continue;
        }
#endif
        NSLog(@"Execute sub-testcase %d:\n", i+1);
        // @-- initialize DUT1 in CAN 2.0 operation mode
        handle = can_init(DUT1, CANMODE_DEFAULT, TEST_PARAM(PAR1));
        XCTAssertLessThanOrEqual(0, handle);
        // @-- get status of DUT1 and check to be in INIT state
        rc = can_status(handle, &status.byte);
        XCTAssertEqual(CANERR_NOERROR, rc);
        XCTAssertTrue(status.can_stopped);
        // @-- start DUT1 with selected bit-rate settings
        rc = can_start(handle, &bitrate);
        XCTAssertEqual(CANERR_NOERROR, rc);
        // @-- get status of DUT1 and check to be in RUNNING state
        rc = can_status(handle, &status.byte);
        XCTAssertEqual(CANERR_NOERROR, rc);
        XCTAssertFalse(status.can_stopped);
        // @-- send and receive some frames to/from DUT2 (optional)
#if (SEND_TEST_FRAMES != 0) && (SEND_WITH_NONE_DEFAULT_BAUDRATE != 0)
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
        // @-- new CAN 2.0 bit-rate settings
        memset(&bitrate, 0, sizeof(can_bitrate_t));
        switch (i) {
            case 7: BITRATE_1M(bitrate); break;
            case 6: BITRATE_500K(bitrate); break;
            case 5: BITRATE_250K(bitrate); break;
            case 4: BITRATE_125K(bitrate); break;
            case 3: BITRATE_100K(bitrate); break;
            case 2: BITRATE_50K(bitrate); break;
            case 1: BITRATE_20K(bitrate); break;
#if (TC03_22_ISSUE_TOUCAN_BITRATE_10K != WORKAROUND_ENABLED)
            case 0: BITRATE_10K(bitrate); break;
#else
            case 0: BITRATE_20K(bitrate); break;
#endif
            default: return;  // Get out of here!
        }
        // @-- start DUT1 again with different bit-rate settings
        rc = can_start(handle, &bitrate);
        XCTAssertEqual(CANERR_NOERROR, rc);
        // @-- get status of DUT1 and check to be in RUNNING state
        rc = can_status(handle, &status.byte);
        XCTAssertEqual(CANERR_NOERROR, rc);
        XCTAssertFalse(status.can_stopped);
        // @-- send and receive some frames to/from DUT2 (optional)
#if (SEND_TEST_FRAMES != 0) && (SEND_WITH_NONE_DEFAULT_BAUDRATE != 0)
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

#if (CAN_FD_SUPPORTED == FEATURE_SUPPORTED)
// @xctest TC03.23: Start CAN controller with valid CAN FD bit-rate settings
//
// @expected: CANERR_NOERROR
//
- (void)testWithValidCanFdBitrateSettings {
    uint8_t mode = (CANMODE_FDOE | CANMODE_BRSE);
    // @note: this test requires two CAN FD capable devices!
    if ((can_test(DUT1, mode, NULL, NULL) == CANERR_NOERROR) &&
        (can_test(DUT2, mode, NULL, NULL) == CANERR_NOERROR)) {
        can_bitrate_t bitrate = { TEST_BTRINDEX };
        can_status_t status = { CANSTAT_RESET };
        int handle = INVALID_HANDLE;
        int rc = CANERR_FATAL;
        // @test: loop over selected CAN FD bit-rate settings
        for (int i = 0; i < 8; i++) {
            memset(&bitrate, 0, sizeof(can_bitrate_t));
            switch (i) {
                // @sub(1): nominal 1Mbps (mode FDOE)
                case 0: BITRATE_FD_1M(bitrate); mode = CANMODE_FDOE; break;
                // @sub(2): nominal 500kbps (mode FDOE)
                case 1: BITRATE_FD_500K(bitrate); mode = CANMODE_FDOE; break;
                // @sub(3): nominal 250kbps (mode FDOE)
                case 2: BITRATE_FD_250K(bitrate); mode = CANMODE_FDOE; break;
                // @sub(4): nominal 125kbps (mode FDOE)
                case 3: BITRATE_FD_125K(bitrate); mode = CANMODE_FDOE; break;
                // @sub(5): nominal 1Mbps, data phase 8Mbps (mode FDOE+BRSE)
                case 4: BITRATE_FD_1M8M(bitrate); mode = (CANMODE_FDOE | CANMODE_BRSE); break;
                // @sub(6): nominal 500kbps, data phase 4Mbps (mode FDOE+BRSE)
                case 5: BITRATE_FD_500K4M(bitrate); mode = (CANMODE_FDOE | CANMODE_BRSE); break;
                // @sub(7): nominal 250kbps, data phase 2Mbps (mode FDOE+BRSE)
                case 6: BITRATE_FD_250K2M(bitrate); mode = (CANMODE_FDOE | CANMODE_BRSE); break;
                // @sub(8): nominal 125kbps, data phase 1Mbps (mode FDOE+BRSE)
                case 7: BITRATE_FD_125K1M(bitrate); mode = (CANMODE_FDOE | CANMODE_BRSE); break;
                default: return;  // Get out of here!
            }
            NSLog(@"Execute sub-testcase %d:\n", i+1);

            // @-- initialize DUT1 in CAN FD operation mode
            handle = can_init(DUT1, mode, NULL);
            XCTAssertLessThanOrEqual(0, handle);
            // @-- get status of DUT1 and check to be in INIT state
            rc = can_status(handle, &status.byte);
            XCTAssertEqual(CANERR_NOERROR, rc);
            XCTAssertTrue(status.can_stopped);
            // @-- start DUT1 with selected bit-rate settings
            rc = can_start(handle, &bitrate);
            XCTAssertEqual(CANERR_NOERROR, rc);
            // @-- get status of DUT1 and check to be in RUNNING state
            rc = can_status(handle, &status.byte);
            XCTAssertEqual(CANERR_NOERROR, rc);
            XCTAssertFalse(status.can_stopped);
            // @-- send and receive some frames to/from DUT2 (optional)
#if (SEND_TEST_FRAMES != 0) && (SEND_WITH_NONE_DEFAULT_BAUDRATE != 0)
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
    } else {
        NSLog(@"Test case skipped: CAN FD operation mode not supported by at least one device.");
    }
    // @end.
}
#endif

#if (CAN_FD_SUPPORTED == FEATURE_SUPPORTED)
// @xctest TC03.24: Start CAN controller with invalid CAN FD bit-rate settings
//
// @expected: CANERR_BAUDRATE
//
- (void)testWithInvalidCanFdBitrateSettings {
    uint8_t mode = (CANMODE_FDOE | CANMODE_BRSE);
    // @note: this test requires two CAN FD capable devices!
    if ((can_test(DUT1, mode, NULL, NULL) == CANERR_NOERROR) &&
        (can_test(DUT2, mode, NULL, NULL) == CANERR_NOERROR)) {
        can_bitrate_t bitrate = { TEST_BTRINDEX };
        can_status_t status = { CANSTAT_RESET };
        int handle = INVALID_HANDLE;
        int rc = CANERR_FATAL;
        // @pre:
        // @- initialize DUT1 in CAN FD operation mode
        handle = can_init(DUT1, mode, NULL);
        XCTAssertLessThanOrEqual(0, handle);
        // @- get status of DUT1 and check to be in INIT state
        rc = can_status(handle, &status.byte);
        XCTAssertEqual(CANERR_NOERROR, rc);
        XCTAssertTrue(status.can_stopped);
        // @test: loop over invalid CAN FD bit-rate settings
#if (FEATURE_BITRATE_FD_SAM == FEATURE_UNSUPPORTED)
        for (int i = 0; i < 28; i++) {  // @note: CAN FD does not specify number-of-samples attribute,
#else
        for (int i = 0; i < 30; i++) {  // @      but some drivers define the field SAM for CAN FD.
#endif
            memset(&bitrate, 0, sizeof(can_bitrate_t));
            BITRATE_FD_250K2M(bitrate);
            switch (i) {
                // @sub(1): set all fields to 0 (note: 'frequency' == 0 is CiA Index 0, set to 1 instead)
                case 0: bitrate.btr.frequency = 1;
                        bitrate.btr.nominal.brp = 0; bitrate.btr.nominal.tseg1 = 0; bitrate.btr.nominal.tseg2 = 0; bitrate.btr.nominal.sjw = 0; bitrate.btr.nominal.sam = 0;
                        bitrate.btr.data.brp = 0; bitrate.btr.data.tseg1 = 0; bitrate.btr.data.tseg2 = 0; bitrate.btr.data.sjw = 0; break;
                // @sub(2): set all fields to MAX value (acc. data type)
                case 1: bitrate.btr.frequency = INT32_MAX;
                        bitrate.btr.nominal.brp = UINT16_MAX; bitrate.btr.nominal.tseg1 = UINT16_MAX; bitrate.btr.nominal.tseg2 = UINT16_MAX; bitrate.btr.nominal.sjw = UINT16_MAX; bitrate.btr.nominal.sam = UINT8_MAX;
                        bitrate.btr.data.brp = UINT16_MAX; bitrate.btr.data.tseg1 = UINT16_MAX; bitrate.btr.data.tseg2 = UINT16_MAX; bitrate.btr.data.sjw = UINT16_MAX; break;
                // @sub(3): set field 'frequency' to 1 (note: see above)
                case 2: bitrate.btr.frequency = 1; break;
                // @sub(4): set field 'frequency' to INT32_MAX
                case 3: bitrate.btr.frequency = INT32_MAX; break;
                // @sub(5): set field 'brp' to 0
                case 4: bitrate.btr.nominal.brp = 0; break;
                // @sub(6): set field 'brp' to 1025
                case 5: bitrate.btr.nominal.brp = NOM_BRP_MAX+1; break;
                // @sub(7): set field 'brp' to UINT16_MAX
                case 6: bitrate.btr.nominal.brp = UINT16_MAX; break;
                // @sub(8): set field 'tseg1' to 0
                case 7: bitrate.btr.nominal.tseg1 = 0; break;
                // @sub(9): set field 'tseg1' to 257
                case 8: bitrate.btr.nominal.tseg1 = NOM_TSEG1_MAX+1; break;
                // @sub(10): set field 'tseg1' to UINT16_MAX
                case 9: bitrate.btr.nominal.tseg1 = UINT16_MAX; break;
                // @sub(11): set field 'tseg2' to 0
                case 10: bitrate.btr.nominal.tseg2 = 0; break;
                // @sub(12): set field 'tseg2' to 129
                case 11: bitrate.btr.nominal.tseg2 = NOM_TSEG2_MAX+1; break;
                // @sub(13): set field 'tseg2' to UINT16_MAX
                case 12: bitrate.btr.nominal.tseg2 = UINT16_MAX; break;
                // @sub(14): set field 'sjw' to 0
                case 13: bitrate.btr.nominal.sjw = 0; break;
                // @sub(15): set field 'sjw' to 129
                case 14: bitrate.btr.nominal.sjw = NOM_SJW_MAX+1; break;
                // @sub(16): set field 'sjw' to UINT16_MAX
                case 15: bitrate.btr.nominal.sjw = UINT16_MAX; break;
                // @sub(17): set field 'data.brp' to 0
                case 16: bitrate.btr.data.brp = 0; break;
                // @sub(18): set field 'data.brp' to 1025
                case 17: bitrate.btr.data.brp = DATA_BRP_MAX+1; break;
                // @sub(19): set field 'data.brp' to UINT16_MAX
                case 18: bitrate.btr.data.brp = UINT16_MAX; break;
                // @sub(20): set field 'data.tseg1' to 0
                case 19: bitrate.btr.data.tseg1 = 0; break;
                // @sub(21): set field 'data.tseg1' to 33
                case 20: bitrate.btr.data.tseg1 = DATA_TSEG1_MAX+1; break;
                // @sub(22): set field 'data.tseg1' to UINT16_MAX
                case 21: bitrate.btr.data.tseg1 = UINT16_MAX; break;
                // @sub(23): set field 'data.tseg2' to 0
                case 22: bitrate.btr.data.tseg2 = 0; break;
                // @sub(24): set field 'data.tseg2' to 17
                case 23: bitrate.btr.data.tseg2 = DATA_TSEG2_MAX+1; break;
                // @sub(25): set field 'data.tseg2' to UINT16_MAX
                case 24: bitrate.btr.data.tseg2 = UINT16_MAX; break;
                // @sub(26): set field 'data.sjw' to 0
                case 25: bitrate.btr.data.sjw = 0; break;
                // @sub(27): set field 'data.sjw' to 17
                case 26: bitrate.btr.data.sjw = DATA_SJW_MAX+1; break;
                // @sub(28): set field 'data.sjw' to UINT16_MAX
                case 27: bitrate.btr.data.sjw = UINT16_MAX; break;
#if (FEATURE_BITRATE_FD_SAM != FEATURE_SUPPORTED)
                // @sub(29): set field 'sam' to 2 (optional)
    #if (TC03_24_ISSUE_KVASER_NOSAMP != WORKAROUND_ENABLED)
                // @issue(KvaserCAN): only SAM = 0 supported by Kvaser devices (noSamp = 1)
                case 28: bitrate.btr.nominal.sam = 2; break;
    #else
                // @workaround: tread SAM = 1 as invalid (noSamp = 3)
                case 28: bitrate.btr.nominal.sam = 1; break;
    #endif
                // @sub(30): set field 'sam' to UINT8_MAX (optional)
                case 29: bitrate.btr.nominal.sam = UINT8_MAX; break;
#endif
                default: return;  // Get out of here!
            }
            NSLog(@"Execute sub-testcase %d:\n", i+1);

            // @-- try to start DUT1 with invalid CAN 2.0 bit-rate settings
            rc = can_start(handle, &bitrate);
            XCTAssertEqual(CANERR_BAUDRATE, rc);
            // @-- get status of DUT1 and check to be in INIT state
            rc = can_status(handle, &status.byte);
            XCTAssertEqual(CANERR_NOERROR, rc);
            XCTAssertTrue(status.can_stopped);
            // @note: stop/reset DUT1 if started anyway
            if (!status.can_stopped)
                (void)can_reset(handle);
        }
        // @post:
        memset(&bitrate, 0, sizeof(can_bitrate_t));
        BITRATE_FD_250K2M(bitrate);
        // @- start DUT1 with valid CAN FD bit-rate settings
        rc = can_start(handle, &bitrate);
        XCTAssertEqual(CANERR_NOERROR, rc);
        // @- send and receive some frames to/from DUT2 (optional)
#if (SEND_TEST_FRAMES != 0) && (SEND_WITH_NONE_DEFAULT_BAUDRATE != 0)
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

    } else {
        NSLog(@"Test case skipped: CAN FD operation mode not supported by at least one device.");
    }
    // @end.
}
#endif
    
#if (CAN_FD_SUPPORTED == FEATURE_SUPPORTED)
// @xctest TC03.25: Re-Start CAN controller with same CAN FD bit-rate settings
//
// @expected: CANERR_NOERROR
//
- (void)testWithSameCanFdBitrateSettingsAfterCanStopped {
    uint8_t mode = (CANMODE_FDOE | CANMODE_BRSE);
    // @note: this test requires two CAN FD capable devices!
    if ((can_test(DUT1, mode, NULL, NULL) == CANERR_NOERROR) &&
        (can_test(DUT2, mode, NULL, NULL) == CANERR_NOERROR)) {
        can_bitrate_t bitrate = { TEST_BTRINDEX };
        can_status_t status = { CANSTAT_RESET };
        int handle = INVALID_HANDLE;
        int rc = CANERR_FATAL;
        // @test: loop over selected CAN FD bit-rate settings
        for (int i = 0; i < 8; i++) {
            memset(&bitrate, 0, sizeof(can_bitrate_t));
            switch (i) {
                // @sub(1): nominal 1Mbps (mode FDOE)
                case 0: BITRATE_FD_1M(bitrate); mode = CANMODE_FDOE; break;
                // @sub(2): nominal 500kbps (mode FDOE)
                case 1: BITRATE_FD_500K(bitrate); mode = CANMODE_FDOE; break;
                // @sub(3): nominal 250kbps (mode FDOE)
                case 2: BITRATE_FD_250K(bitrate); mode = CANMODE_FDOE; break;
                // @sub(4): nominal 125kbps (mode FDOE)
                case 3: BITRATE_FD_125K(bitrate); mode = CANMODE_FDOE; break;
                // @sub(5): nominal 1Mbps, data phase 8Mbps (mode FDOE+BRSE)
                case 4: BITRATE_FD_1M8M(bitrate); mode = (CANMODE_FDOE | CANMODE_BRSE); break;
                // @sub(6): nominal 500kbps, data phase 4Mbps (mode FDOE+BRSE)
                case 5: BITRATE_FD_500K4M(bitrate); mode = (CANMODE_FDOE | CANMODE_BRSE); break;
                // @sub(7): nominal 250kbps, data phase 2Mbps (mode FDOE+BRSE)
                case 6: BITRATE_FD_250K2M(bitrate); mode = (CANMODE_FDOE | CANMODE_BRSE); break;
                // @sub(8): nominal 125kbps, data phase 1Mbps (mode FDOE+BRSE)
                case 7: BITRATE_FD_125K1M(bitrate); mode = (CANMODE_FDOE | CANMODE_BRSE); break;
                default: return;  // Get out of here!
            }
            NSLog(@"Execute sub-testcase %d:\n", i+1);

            // @-- initialize DUT1 in CAN FD operation mode
            handle = can_init(DUT1, mode, NULL);
            XCTAssertLessThanOrEqual(0, handle);
            // @-- get status of DUT1 and check to be in INIT state
            rc = can_status(handle, &status.byte);
            XCTAssertEqual(CANERR_NOERROR, rc);
            XCTAssertTrue(status.can_stopped);
            // @-- start DUT1 with selected bit-rate settings
            rc = can_start(handle, &bitrate);
            XCTAssertEqual(CANERR_NOERROR, rc);
            // @-- get status of DUT1 and check to be in RUNNING state
            rc = can_status(handle, &status.byte);
            XCTAssertEqual(CANERR_NOERROR, rc);
            XCTAssertFalse(status.can_stopped);
            // @-- send and receive some frames to/from DUT2 (optional)
#if (SEND_TEST_FRAMES != 0) && (SEND_WITH_NONE_DEFAULT_BAUDRATE != 0)
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
            
            // @-- start DUT1 again with same bit-rate settings
            rc = can_start(handle, &bitrate);
            XCTAssertEqual(CANERR_NOERROR, rc);
            // @-- get status of DUT1 and check to be in RUNNING state
            rc = can_status(handle, &status.byte);
            XCTAssertEqual(CANERR_NOERROR, rc);
            XCTAssertFalse(status.can_stopped);
            // @-- send and receive some frames to/from DUT2 (optional)
#if (SEND_TEST_FRAMES != 0) && (SEND_WITH_NONE_DEFAULT_BAUDRATE != 0)
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
    } else {
        NSLog(@"Test case skipped: CAN FD operation mode not supported by at least one device.");
    }
    // @end.
}
#endif

#if (CAN_FD_SUPPORTED == FEATURE_SUPPORTED)
// @xctest TC03.26: Re-Start CAN controller with different CAN FD bit-rate settings
//
// @expected: CANERR_NOERROR
//
- (void)testWithDifferentCanFdBitrateSettingsAfterCanStopped {
    uint8_t mode = (CANMODE_FDOE | CANMODE_BRSE);
    // @note: this test requires two CAN FD capable devices!
    if ((can_test(DUT1, mode, NULL, NULL) == CANERR_NOERROR) &&
        (can_test(DUT2, mode, NULL, NULL) == CANERR_NOERROR)) {
        can_bitrate_t bitrate = { TEST_BTRINDEX };
        can_status_t status = { CANSTAT_RESET };
        int handle = INVALID_HANDLE;
        int rc = CANERR_FATAL;
        // @test: loop over selected CAN FD bit-rate settings
        for (int i = 0; i < 8; i++) {
            memset(&bitrate, 0, sizeof(can_bitrate_t));
            switch (i) {
                // @sub(1): nominal 1Mbps (mode FDOE)
                case 0: BITRATE_FD_1M(bitrate); mode = CANMODE_FDOE; break;
                // @sub(2): nominal 500kbps (mode FDOE)
                case 1: BITRATE_FD_500K(bitrate); mode = CANMODE_FDOE; break;
                // @sub(3): nominal 250kbps (mode FDOE)
                case 2: BITRATE_FD_250K(bitrate); mode = CANMODE_FDOE; break;
                // @sub(4): nominal 125kbps (mode FDOE)
                case 3: BITRATE_FD_125K(bitrate); mode = CANMODE_FDOE; break;
                // @sub(5): nominal 1Mbps, data phase 8Mbps (mode FDOE+BRSE)
                case 4: BITRATE_FD_1M8M(bitrate); mode = (CANMODE_FDOE | CANMODE_BRSE); break;
                // @sub(6): nominal 500kbps, data phase 4Mbps (mode FDOE+BRSE)
                case 5: BITRATE_FD_500K4M(bitrate); mode = (CANMODE_FDOE | CANMODE_BRSE); break;
                // @sub(7): nominal 250kbps, data phase 2Mbps (mode FDOE+BRSE)
                case 6: BITRATE_FD_250K2M(bitrate); mode = (CANMODE_FDOE | CANMODE_BRSE); break;
                // @sub(8): nominal 125kbps, data phase 1Mbps (mode FDOE+BRSE)
                case 7: BITRATE_FD_125K1M(bitrate); mode = (CANMODE_FDOE | CANMODE_BRSE); break;
                default: return;  // Get out of here!
            }
            NSLog(@"Execute sub-testcase %d:\n", i+1);

            // @-- initialize DUT1 in CAN FD operation mode
            handle = can_init(DUT1, mode, NULL);
            XCTAssertLessThanOrEqual(0, handle);
            // @-- get status of DUT1 and check to be in INIT state
            rc = can_status(handle, &status.byte);
            XCTAssertEqual(CANERR_NOERROR, rc);
            XCTAssertTrue(status.can_stopped);
            // @-- start DUT1 with selected bit-rate settings
            rc = can_start(handle, &bitrate);
            XCTAssertEqual(CANERR_NOERROR, rc);
            // @-- get status of DUT1 and check to be in RUNNING state
            rc = can_status(handle, &status.byte);
            XCTAssertEqual(CANERR_NOERROR, rc);
            XCTAssertFalse(status.can_stopped);
            // @-- send and receive some frames to/from DUT2 (optional)
#if (SEND_TEST_FRAMES != 0) && (SEND_WITH_NONE_DEFAULT_BAUDRATE != 0)
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
            
            // @-- new CAN FD bit-rate settings
            memset(&bitrate, 0, sizeof(can_bitrate_t));
            switch (i) {
                case 3: BITRATE_FD_1M(bitrate); break;
                case 2: BITRATE_FD_500K(bitrate); break;
                case 1: BITRATE_FD_250K(bitrate); break;
                case 0: BITRATE_FD_125K(bitrate); break;
                case 7: BITRATE_FD_1M8M(bitrate); break;
                case 6: BITRATE_FD_500K4M(bitrate); break;
                case 5: BITRATE_FD_250K2M(bitrate); break;
                case 4: BITRATE_FD_125K1M(bitrate); break;
                default: return;  // Get out of here!
            }
            // @-- start DUT1 again with different bit-rate settings
            rc = can_start(handle, &bitrate);
            XCTAssertEqual(CANERR_NOERROR, rc);
            // @-- get status of DUT1 and check to be in RUNNING state
            rc = can_status(handle, &status.byte);
            XCTAssertEqual(CANERR_NOERROR, rc);
            XCTAssertFalse(status.can_stopped);
            // @-- send and receive some frames to/from DUT2 (optional)
#if (SEND_TEST_FRAMES != 0) && (SEND_WITH_NONE_DEFAULT_BAUDRATE != 0)
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
    } else {
        NSLog(@"Test case skipped: CAN FD operation mode not supported by at least one device.");
    }
    // @end.
}
#endif
    
#if (CAN_FD_SUPPORTED == FEATURE_SUPPORTED)
// @xctest TC03.27: Start CAN controller with CAN 2.0 bit-timing index in CAN FD operation mode (w/o bit-rate switching)
//
// @expected: CANERR_BAUDRATE
//
- (void)testWithCanBitrateIndexInCanFdMode {
    uint8_t mode = CANMODE_FDOE;
    // @note: this test requires two CAN FD capable devices!
    if ((can_test(DUT1, mode, NULL, NULL) == CANERR_NOERROR) &&
        (can_test(DUT2, mode, NULL, NULL) == CANERR_NOERROR)) {
        can_bitrate_t bitrate = { TEST_BTRINDEX };
        can_status_t status = { CANSTAT_RESET };
        int handle = INVALID_HANDLE;
        int rc = CANERR_FATAL;
        // @pre:
        // @- initialize DUT1 in CAN FD operation mode w/o BRSE
        handle = can_init(DUT1, mode, NULL);
        XCTAssertLessThanOrEqual(0, handle);
        // @- get status of DUT1 and check to be in INIT state
        rc = can_status(handle, &status.byte);
        XCTAssertEqual(CANERR_NOERROR, rc);
        XCTAssertTrue(status.can_stopped);
        
        // @test: loop over selected CAN 2.0 bit-timing indexes
        // @note: predefined BTR0BTR1 bit-timing table has 10 entries, index 0 to 9.
        // @      The index must be given as negative value to 'bitrate.index'!
        // @      Remark: The CiA bit-timing table has only 9 entries!
        for (int i = 0; i < 10; i++) {
            switch (i) {
                // @sub(1): index 0 (1Mbps)
                case 0: bitrate.index = CANBTR_INDEX_1M; break;
                // @sub(2): index 1 (800kbps, not supported by all CAN controllers)
                case 1: bitrate.index = CANBTR_INDEX_800K; break;
                // @sub(3): index 2 (500kbps)
                case 2: bitrate.index = CANBTR_INDEX_500K; break;
                // @sub(4): index 3 (250kbps)
                case 3: bitrate.index = CANBTR_INDEX_250K; break;
                // @sub(5): index 4 (125kbps)
                case 4: bitrate.index = CANBTR_INDEX_125K; break;
                // @sub(6): index 5 (100kbps)
                case 5: bitrate.index = CANBTR_INDEX_100K; break;
                // @sub(7): index 6 (50kbps)
                case 6: bitrate.index = CANBTR_INDEX_50K; break;
                // @sub(8): index 7 (20kbps)
                case 7: bitrate.index = CANBTR_INDEX_20K; break;
                // @sub(9): index 8 (10kbps)
                case 8: bitrate.index = CANBTR_INDEX_10K; break;
                // @sub(10):    index 9 (5kbps, not supported by all CAN API SDK's)
                case 9: bitrate.index = SJA1000_INDEX_5K; break;
                default: return;  // Get out of here!
            }
#if (FEATURE_BITRATE_800K != FEATURE_SUPPORTED)
            if (bitrate.index == CANBTR_INDEX_800K)
                continue;
#endif
#if (FEATURE_BITRATE_SJA1000 != FEATURE_SUPPORTED) || (SERIAL_CAN_SUPPORTED != 0)
            if (bitrate.index == SJA1000_INDEX_5K)
                continue;
#endif
            NSLog(@"Execute sub-testcase %d:\n", i+1);

            // @-- try to start DUT1 with selected bit-timing index
            rc = can_start(handle, &bitrate);
            XCTAssertEqual(CANERR_BAUDRATE, rc);
            // @-- get status of DUT1 and check to be in INIT state
            rc = can_status(handle, &status.byte);
            XCTAssertEqual(CANERR_NOERROR, rc);
            XCTAssertTrue(status.can_stopped);
            // @note: stop/reset DUT1 if started anyway
            if (!status.can_stopped)
                (void)can_reset(handle);
        }
        // @post:
        memset(&bitrate, 0, sizeof(can_bitrate_t));
        BITRATE_FD_1M(bitrate);
        // @- start DUT1 with valid bit-rate settings
        rc = can_start(handle, &bitrate);
        XCTAssertEqual(CANERR_NOERROR, rc);
        // @- get status of DUT1 and check to be in RUNNING state
        rc = can_status(handle, &status.byte);
        XCTAssertEqual(CANERR_NOERROR, rc);
        XCTAssertFalse(status.can_stopped);
        // @- send and receive some frames to/from DUT2 (optional)
#if (SEND_TEST_FRAMES != 0) && (SEND_WITH_NONE_DEFAULT_BAUDRATE != 0)
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
    } else {
        NSLog(@"Test case skipped: CAN FD operation mode not supported by at least one device.");
    }
    // @end.
}
#endif
    
#if (CAN_FD_SUPPORTED == FEATURE_SUPPORTED)
// @xctest TC03.28: Start CAN controller with CAN 2.0 bit-rate settings in CAN FD operation mode (w/o bit-rate switching)
//
// @expected: CANERR_BAUDRATE
//
- (void)testWithCanBitrateSettingsInCanFdMode {
    uint8_t mode = CANMODE_FDOE;
    // @note: this test requires two CAN FD capable devices!
    if ((can_test(DUT1, mode, NULL, NULL) == CANERR_NOERROR) &&
        (can_test(DUT2, mode, NULL, NULL) == CANERR_NOERROR)) {
        can_bitrate_t bitrate = { TEST_BTRINDEX };
        can_status_t status = { CANSTAT_RESET };
        int handle = INVALID_HANDLE;
        int rc = CANERR_FATAL;
        // @test: loop over selected CAN 2.0 bit-rate settings
        for (int i = 0; i < 4; i++) {
            memset(&bitrate, 0, sizeof(can_bitrate_t));
            switch (i) {
                // @sub(1): 1Mbps
                case 0: BITRATE_1M(bitrate); break;
                // @sub(2): 500kbps
                case 1: BITRATE_500K(bitrate); break;
                // @sub(3): 250kbps
                case 2: BITRATE_250K(bitrate); break;
                // @sub(4): 125kbps
                case 3: BITRATE_125K(bitrate); break;
#if (0)         // @note: lower bit-rates might lead to invalid prescaler
                // @sub(5): 100kbps (skipped)
                case 4: BITRATE_100K(bitrate); break;
                // @sub(6): 50kbps (skipped)
                case 5: BITRATE_50K(bitrate); break;
                // @sub(7): 20kbps (skipped)
                case 6: BITRATE_20K(bitrate); break;
                // @sub(8): 10kbps (skipped)
                case 7: BITRATE_10K(bitrate); break;
#endif
                default: return;  // Get out of here!
            }
            NSLog(@"Execute sub-testcase %d:\n", i+1);

            // @-- initialize DUT1 in CAN FD operation mode w/o BRSE
            handle = can_init(DUT1, mode, NULL);
            XCTAssertLessThanOrEqual(0, handle);
            // @-- get status of DUT1 and check to be in INIT state
            rc = can_status(handle, &status.byte);
            XCTAssertEqual(CANERR_NOERROR, rc);
            XCTAssertTrue(status.can_stopped);
#if (FEATURE_BITRATE_FD_SJA1000 == FEATURE_UNSUPPORTED)
            // @note: SJA1000 BTROBTR1 bit-timing is defined for CAN 2.0 only
            // @-- try to start DUT1 with selected bit-rate settings
            rc = can_start(handle, &bitrate);
            XCTAssertEqual(CANERR_BAUDRATE, rc);
#else
            // @      but some drivers can handle it in CAN FD operation mode
            // @-- start DUT1 with selected bit-rate settings
            rc = can_start(handle, &bitrate);
            XCTAssertEqual(CANERR_NOERROR, rc);
            // @-- get status of DUT1 and check to be in RUNNING state
            rc = can_status(handle, &status.byte);
            XCTAssertEqual(CANERR_NOERROR, rc);
            XCTAssertFalse(status.can_stopped);
            // @-- send and receive some frames to/from DUT2 (optional)
#if (SEND_TEST_FRAMES != 0) && (SEND_WITH_NONE_DEFAULT_BAUDRATE != 0)
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
#endif
            // @-- get status of DUT1 and check to be in INIT state
            rc = can_status(handle, &status.byte);
            XCTAssertEqual(CANERR_NOERROR, rc);
            XCTAssertTrue(status.can_stopped);
            // @-- tear down DUT1
            rc = can_exit(handle);
            XCTAssertEqual(CANERR_NOERROR, rc);
        }
    } else {
        NSLog(@"Test case skipped: CAN FD operation mode not supported by at least one device.");
    }
    // @end.
}
#endif
    
#if (CAN_FD_SUPPORTED == FEATURE_SUPPORTED)
// @xctest TC03.29: Start CAN controller with CAN FD bit-rate settings in CAN 2.0 operation mode
//
// @expected: CANERR_BAUDRATE
//
- (void)testWithCanFdBitrateSettingsInCan20Mode {
    can_bitrate_t bitrate = { TEST_BTRINDEX };
    can_status_t status = { CANSTAT_RESET };
    can_mode_t opCapa = { CANMODE_DEFAULT };
    int handle = INVALID_HANDLE;
    int rc = CANERR_FATAL;
    // @pre:
    // @- initialize DUT1 in CAN 2.0 operation mode
    handle = can_init(DUT1, CANMODE_DEFAULT, TEST_PARAM(PAR1));
    XCTAssertLessThanOrEqual(0, handle);
    // @- get status of DUT1 and check to be in INIT state
    rc = can_status(handle, &status.byte);
    XCTAssertEqual(CANERR_NOERROR, rc);
    XCTAssertTrue(status.can_stopped);
    // @- get operation mode capability
    rc = can_property(handle, CANPROP_GET_OP_CAPABILITY, (void*)&opCapa.byte, sizeof(uint8_t));
    XCTAssertEqual(CANERR_NOERROR, rc);
    // @test: loop over selected CAN FD bit-rate settings
    for (int i = 0; i < 8; i++) {
         memset(&bitrate, 0, sizeof(can_bitrate_t));
         switch (i) {
            // @sub(1): nominal 1Mbps
            case 0: BITRATE_FD_1M(bitrate); break;
            // @sub(2): nominal 500kbps
            case 1: BITRATE_FD_500K(bitrate); break;
            // @sub(3): nominal 250kbps
            case 2: BITRATE_FD_250K(bitrate); break;
            // @sub(4): nominal 125kbps
            case 3: BITRATE_FD_125K(bitrate); break;
            // @sub(5): nominal 1Mbps, data phase 8Mbps
            case 4: BITRATE_FD_1M8M(bitrate); break;
            // @sub(6): nominal 500kbps, data phase 4Mbps
            case 5: BITRATE_FD_500K4M(bitrate); break;
            // @sub(7): nominal 250kbps, data phase 2Mbps
            case 6: BITRATE_FD_250K2M(bitrate); break;
            // @sub(8): nominal 125kbps, data phase 1Mbps
            case 7: BITRATE_FD_125K1M(bitrate); break;
            default: return;  // Get out of here!
        }
        NSLog(@"Execute sub-testcase %d:\n", i+1);
        // @-- start DUT1 with CAN FD bit-rate settings
        rc = can_start(handle, &bitrate);
        if (opCapa.fdoe) {
#if (TC03_29_ISSUE_PCBUSB_BR_FD_IN_2_0 != WORKAROUND_ENABLED)
            // @issue(PeakCAN): only SJA1000 frequency allowed in CAN 2.0 mode
            XCTAssertEqual(CANERR_NOERROR, rc);
            // @--- get status of DUT1 and check to be in RUNNING state
            rc = can_status(handle, &status.byte);
            XCTAssertEqual(CANERR_NOERROR, rc);
            XCTAssertFalse(status.can_stopped);
            // @--- stop/reset DUT1
            rc = can_reset(handle);
            XCTAssertEqual(CANERR_NOERROR, rc);
#else
            // @workaround: nothing to do, just check the result
            XCTAssertEqual(CANERR_BAUDRATE, rc);
#endif
        } else {
            // @-- otherwise: refuse to accept it
            XCTAssertEqual(CANERR_BAUDRATE, rc);
        }
        // @-- get status of DUT1 and check to be in INIT state
        rc = can_status(handle, &status.byte);
        XCTAssertEqual(CANERR_NOERROR, rc);
        XCTAssertTrue(status.can_stopped);
    }
    // @post:
    memset(&bitrate, 0, sizeof(can_bitrate_t));
    BITRATE_250K(bitrate);
    // @- start DUT1 with CAN 2.0 bit-rate settings (250kbps)
    rc = can_start(handle, &bitrate);
    XCTAssertEqual(CANERR_NOERROR, rc);
    // @- send and receive some frames to/from DUT2 (optional)
#if (SEND_TEST_FRAMES != 0) && (SEND_WITH_NONE_DEFAULT_BAUDRATE != 0)
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
#endif

@end

// $Id: test_can_start.mm 1341 2024-06-15 16:43:48Z makemake $  Copyright (c) UV Software, Berlin //
