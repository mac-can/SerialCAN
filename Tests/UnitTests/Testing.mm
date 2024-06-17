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

@interface Testing : XCTestCase

@end

@implementation Testing

- (void)setUp {
    // Put setup code here. This method is called before the invocation of each test method in the class.
}

- (void)tearDown {
    // Put teardown code here. This method is called after the invocation of each test method in the class.
    (void)can_exit(CANKILL_ALL);
}

// @xctest TC00.0: A test case
//
// @expected: CANERR_NOERROR
//
// - (void)testCase {
//     // @test:
//     // @todo: insert coin here
//     // @end.
// }

// @xctest TC00.1: The default scenario, suitable for long-term stress tests
//
// @expected: CANERR_NOERROR
//
- (void)testSmokeTest {
    can_bitrate_t bitrate = { TEST_BTRINDEX };
    can_status_t status = { CANSTAT_RESET };
    char name[CANPROP_MAX_BUFFER_SIZE];
    uint16_t version;
    uint8_t patch;
    uint32_t build;
    char *string = NULL;
    int handle = INVALID_HANDLE;
    int rc = CANERR_FATAL;
    // @pre:
    // @- get library information
    if ((can_property(INVALID_HANDLE, CANPROP_GET_VERSION, (void*)&version, sizeof(uint16_t)) == CANERR_NOERROR) &&
        (can_property(INVALID_HANDLE, CANPROP_GET_PATCH_NO, (void*)&patch, sizeof(uint8_t)) == CANERR_NOERROR) &&
        (can_property(INVALID_HANDLE, CANPROP_GET_BUILD_NO, (void*)&build, sizeof(uint32_t)) == CANERR_NOERROR) &&
        (can_property(INVALID_HANDLE, CANPROP_GET_LIBRARY_DLLNAME, (void*)name, CANPROP_MAX_BUFFER_SIZE) == CANERR_NOERROR))
        NSLog(@"CAN API V3 Testing: %s V%u.%u.%u (%x)\n", name, (version & 0xFF00U) >> 8, (version & 0x00FFU), patch, build);
    // @- get information from DUT1
    if ((handle = can_init(DUT1, CANMODE_DEFAULT, TEST_PARAM(PAR1))) != INVALID_HANDLE) {
        if ((string = can_hardware(handle)) != NULL)
            NSLog(@"DUT1: %s\n", string);
        (void)can_exit(handle);
    }
    // @- get information from DUT2
    if ((handle = can_init(DUT2, CANMODE_DEFAULT, TEST_PARAM(PAR2))) != INVALID_HANDLE) {
        if ((string = can_hardware(handle)) != NULL)
            NSLog(@"DUT2: %s\n", string);
        (void)can_exit(handle);
    }
    // @- probe if DUT1 is present and not occupied
    // @  todo: insert coin here
    // @- probe if DUT2 is present and not occupied
    // @  todo: insert coin here
    // @- check if different channels have been selected
    // @  todo: insert coin here
    // @test:
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
    // @- send some frames to DUT2 and receive some frames from DUT2
    CTester tester;
    NSLog(@"Be patient...");
    XCTAssertEqual(TEST_TRAFFIC, tester.SendSomeFrames(handle, DUT2, TEST_TRAFFIC));
    XCTAssertEqual(TEST_TRAFFIC, tester.ReceiveSomeFrames(handle, DUT2, TEST_TRAFFIC));
    // @- get status of DUT1 and check to be in RUNNING state
    rc = can_status(handle, &status.byte);
    XCTAssertEqual(CANERR_NOERROR, rc);
    XCTAssertFalse(status.can_stopped);
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

//- (void)testExample {
//    // This is an example of a functional test case.
//    // Use XCTAssert and related functions to verify your tests produce the correct results.
//    XCTAssertTrue(true);
//}

//- (void)testPerformanceExample {
//    // This is an example of a performance test case.
//    [self measureBlock:^{
//        // Put the code you want to measure the time of here.
//    }];
//}

@end

// $Id: Testing.mm 1341 2024-06-15 16:43:48Z makemake $  Copyright (c) UV Software, Berlin //
