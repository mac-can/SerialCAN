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
#include "Tester.h"
#include "Timer.h"

#include "can_api.h"

#include <string.h>
#include <iostream>

#define ERROR_MSG_ID  (-70)
#define ERROR_MSG_DLC  (-71)
#define ERROR_MSG_LOST  (-72)
#define ERROR_MSG_FATAL  (-79)

int CTester::SendSomeFrames(int handle, int32_t channel, int frames, uint32_t canId) {
    CANAPI_Message_t trmMessage = {}, rcvMessage = {};
    CANAPI_Bitrate_t bitrate = { CANBTR_INDEX_250K };
    CANAPI_OpMode_t opMode = { CANMODE_DEFAULT };
    CANAPI_Return_t retVal = CTester::FatalError;
    void *param = NULL;
    int i, n = 0;
    
    CTimer timer = CTimer((uint32_t)frames * 100U * CTimer::MSEC);
    uint64_t expected = 0ULL;
    
    if ((retVal = can_property(handle, CANPROP_GET_OP_MODE, (void*)&opMode.byte, sizeof(uint8_t))) < CANERR_NOERROR)
        return retVal;
    if ((retVal = can_property(handle, CANPROP_GET_BITRATE, (void*)&bitrate, sizeof(CANAPI_Bitrate_t))) < CANERR_NOERROR)
        return retVal;
#if (SERIAL_CAN_SUPPORTED != 0)
    param = GetParameter((channel == CAN_DEVICE1) ? PAR1 : PAR2);
#endif
    retVal = InitializeChannel(channel, opMode, param);
    if (retVal != CTester::NoError)
        return (int)retVal;
    retVal = StartController(bitrate);
    if (retVal != CTester::NoError)
        goto exitSendSomeFrames;
    PCBUSB_INIT_DELAY();

    trmMessage.id = canId;
    trmMessage.fdf = opMode.fdoe ? 1 : 0;
    trmMessage.brs = opMode.brse ? 1 : 0;
    trmMessage.xtd = (canId > CAN_MAX_STD_ID) ? 1 : 0;
    trmMessage.rtr = 0;
    trmMessage.esi = 0;
    trmMessage.sts = 0;
    trmMessage.dlc = opMode.fdoe ? CANFD_MAX_DLC : CAN_MAX_DLC;
    memset(trmMessage.data, 0, CANFD_MAX_LEN);
    
    for (i = 0; i < frames; i++) {
        trmMessage.data[0] = (uint8_t)((uint64_t)i >> 0);
        trmMessage.data[1] = (uint8_t)((uint64_t)i >> 8);
        trmMessage.data[2] = (uint8_t)((uint64_t)i >> 16);
        trmMessage.data[3] = (uint8_t)((uint64_t)i >> 24);
        trmMessage.data[4] = (uint8_t)((uint64_t)i >> 32);
        trmMessage.data[5] = (uint8_t)((uint64_t)i >> 40);
        trmMessage.data[6] = (uint8_t)((uint64_t)i >> 48);
        trmMessage.data[7] = (uint8_t)((uint64_t)i >> 56);
        do {
            retVal = can_write(handle, &trmMessage, 0U);
            if (retVal == CANERR_TX_BUSY)
                CTimer::Delay(CTimer::MSEC);
        } while (retVal == CANERR_TX_BUSY);
        if (retVal != CANERR_NOERROR)
            goto exitSendSomeFrames;
        PCBUSB_QXMT_DELAY();
        memset(&rcvMessage, 0, sizeof(can_message_t));
        if ((retVal = ReadMessage(rcvMessage, 0U)) == CTester::NoError) {
            if (rcvMessage.sts)
                continue;
            retVal = CheckReceivedId(rcvMessage, trmMessage.id);
            if (retVal != CTester::NoError)
                goto exitSendSomeFrames;
            retVal = CheckReceivedDlc(rcvMessage, trmMessage.dlc);
            if (retVal != CTester::NoError)
                goto exitSendSomeFrames;
            retVal = CheckReceivedData(rcvMessage, expected);
            if (retVal != CTester::NoError)
                goto exitSendSomeFrames;
            n++;
        } else if (retVal != CTester::ReceiverEmpty)
            goto exitSendSomeFrames;
    }
    while ((n < i) && ! timer.Timeout()) {
        memset(&rcvMessage, 0, sizeof(can_message_t));
        if ((retVal = ReadMessage(rcvMessage, 50U)) == CTester::NoError) {
            if (rcvMessage.sts)
                continue;
            retVal = CheckReceivedId(rcvMessage, trmMessage.id);
            if (retVal != CTester::NoError)
                goto exitSendSomeFrames;
            retVal = CheckReceivedDlc(rcvMessage, trmMessage.dlc);
            if (retVal != CTester::NoError)
                goto exitSendSomeFrames;
            retVal = CheckReceivedData(rcvMessage, expected);
            if (retVal != CTester::NoError)
                goto exitSendSomeFrames;
            n++;
        } else if (retVal != CTester::ReceiverEmpty)
            goto exitSendSomeFrames;
    }
    retVal = n;
exitSendSomeFrames:
    (void)TeardownChannel();
    return retVal;
}

int CTester::ReceiveSomeFrames(int handle, int32_t channel, int frames, uint32_t canId) {
    CANAPI_Message_t trmMessage = {}, rcvMessage = {};
    CANAPI_Bitrate_t bitrate = { CANBTR_INDEX_250K };
    CANAPI_OpMode_t opMode = { CANMODE_DEFAULT };
    CANAPI_Return_t retVal = CTester::FatalError;
    void *param = NULL;
    int i, n = 0;
    
    CTimer timer = CTimer((uint32_t)frames * 100U * CTimer::MSEC);
    uint64_t expected = 0ULL;
    
    if ((retVal = can_property(handle, CANPROP_GET_OP_MODE, (void*)&opMode.byte, sizeof(uint8_t))) < CANERR_NOERROR)
        return retVal;
    if ((retVal = can_property(handle, CANPROP_GET_BITRATE, (void*)&bitrate, sizeof(CANAPI_Bitrate_t))) < CANERR_NOERROR)
        return retVal;
#if (SERIAL_CAN_SUPPORTED != 0)
    param = GetParameter((channel == CAN_DEVICE1) ? PAR1 : PAR2);
#endif
    retVal = InitializeChannel(channel, opMode, param);
    if (retVal != CTester::NoError)
        return (int)retVal;
    retVal = StartController(bitrate);
    if (retVal != CTester::NoError)
        goto exitReceiveSomeFrames;
    PCBUSB_INIT_DELAY();

    trmMessage.id = canId;
    trmMessage.fdf = opMode.fdoe ? 1 : 0;
    trmMessage.brs = opMode.brse ? 1 : 0;
    trmMessage.xtd = (canId > CAN_MAX_STD_ID) ? 1 : 0;
    trmMessage.rtr = 0;
    trmMessage.esi = 0;
    trmMessage.sts = 0;
    trmMessage.dlc = opMode.fdoe ? CANFD_MAX_DLC : CAN_MAX_DLC;
    memset(trmMessage.data, 0, CANFD_MAX_LEN);
    
    for (i = 0; i < frames; i++) {
        trmMessage.data[0] = (uint8_t)((uint64_t)i >> 0);
        trmMessage.data[1] = (uint8_t)((uint64_t)i >> 8);
        trmMessage.data[2] = (uint8_t)((uint64_t)i >> 16);
        trmMessage.data[3] = (uint8_t)((uint64_t)i >> 24);
        trmMessage.data[4] = (uint8_t)((uint64_t)i >> 32);
        trmMessage.data[5] = (uint8_t)((uint64_t)i >> 40);
        trmMessage.data[6] = (uint8_t)((uint64_t)i >> 48);
        trmMessage.data[7] = (uint8_t)((uint64_t)i >> 56);
        do {
            retVal = WriteMessage(trmMessage);
            if (retVal == CTester::TransmitterBusy)
                CTimer::Delay(CTimer::MSEC);
        } while (retVal == CTester::TransmitterBusy);
        if (retVal != CTester::NoError)
            goto exitReceiveSomeFrames;
        PCBUSB_QXMT_DELAY();
        memset(&rcvMessage, 0, sizeof(can_message_t));
        if ((retVal = can_read(handle, &rcvMessage, 0U)) == CANERR_NOERROR) {
            if (rcvMessage.sts)
                continue;
            retVal = CheckReceivedId(rcvMessage, trmMessage.id);
            if (retVal != CANERR_NOERROR)
                goto exitReceiveSomeFrames;
            retVal = CheckReceivedDlc(rcvMessage, trmMessage.dlc);
            if (retVal != CANERR_NOERROR)
                goto exitReceiveSomeFrames;
            retVal = CheckReceivedData(rcvMessage, expected);
            if (retVal != CANERR_NOERROR)
                goto exitReceiveSomeFrames;
            n++;
        } else if (retVal != CANERR_RX_EMPTY)
            goto exitReceiveSomeFrames;
    }
    while ((n < i) && ! timer.Timeout()) {
        memset(&rcvMessage, 0, sizeof(can_message_t));
        if ((retVal = can_read(handle, &rcvMessage, 50U)) == CANERR_NOERROR) {
            if (rcvMessage.sts)
                continue;
            retVal = CheckReceivedId(rcvMessage, trmMessage.id);
            if (retVal != CANERR_NOERROR)
                goto exitReceiveSomeFrames;
            retVal = CheckReceivedDlc(rcvMessage, trmMessage.dlc);
            if (retVal != CANERR_NOERROR)
                goto exitReceiveSomeFrames;
            retVal = CheckReceivedData(rcvMessage, expected);
            if (retVal != CANERR_NOERROR)
                goto exitReceiveSomeFrames;
            n++;
        } else if (retVal != CANERR_RX_EMPTY)
            goto exitReceiveSomeFrames;
    }
    retVal = n;
exitReceiveSomeFrames:
    (void)TeardownChannel();
    return retVal;
}

int CTester::CheckReceivedId(const CANAPI_Message_t &message, int32_t canId) {
    int rc = CANERR_NOERROR;
    
    if (canId != message.id) {
        rc = ERROR_MSG_ID;  // not CAN API compliant
    }
    return rc;
}

int CTester::CheckReceivedDlc(const CANAPI_Message_t &message, uint8_t canDlc) {
    int rc = CANERR_NOERROR;
    
    if (canDlc != message.dlc) {
        rc = ERROR_MSG_DLC;  // not CAN API compliant
    }
    return rc;
}

int CTester::CheckReceivedData(const CANAPI_Message_t &message, uint64_t &expected) {
    int rc = CANERR_NOERROR;
    
    uint64_t received = 0x0000000000000000U;
    received |= (message.dlc > 0) ? (uint64_t)message.data[0] << 0 : 0x0000000000000000U;
    received |= (message.dlc > 1) ? (uint64_t)message.data[1] << 8 : 0x0000000000000000U;
    received |= (message.dlc > 2) ? (uint64_t)message.data[2] << 16 : 0x0000000000000000U;
    received |= (message.dlc > 3) ? (uint64_t)message.data[3] << 24 : 0x0000000000000000U;
    received |= (message.dlc > 4) ? (uint64_t)message.data[4] << 32 : 0x0000000000000000U;
    received |= (message.dlc > 5) ? (uint64_t)message.data[5] << 40 : 0x0000000000000000U;
    received |= (message.dlc > 6) ? (uint64_t)message.data[6] << 48 : 0x0000000000000000U;
    received |= (message.dlc > 7) ? (uint64_t)message.data[7] << 56 : 0x0000000000000000U;
    
    expected &= (message.dlc == 7) ? 0x00FFFFFFFFFFFFFFU : 0xFFFFFFFFFFFFFFFFU;
    expected &= (message.dlc == 6) ? 0x0000FFFFFFFFFFFFU : 0xFFFFFFFFFFFFFFFFU;
    expected &= (message.dlc == 5) ? 0x000000FFFFFFFFFFU : 0xFFFFFFFFFFFFFFFFU;
    expected &= (message.dlc == 4) ? 0x00000000FFFFFFFFU : 0xFFFFFFFFFFFFFFFFU;
    expected &= (message.dlc == 3) ? 0x0000000000FFFFFFU : 0xFFFFFFFFFFFFFFFFU;
    expected &= (message.dlc == 2) ? 0x000000000000FFFFU : 0xFFFFFFFFFFFFFFFFU;
    expected &= (message.dlc == 1) ? 0x00000000000000FFU : 0xFFFFFFFFFFFFFFFFU;
    expected &= (message.dlc == 0) ? 0x0000000000000000U : 0xFFFFFFFFFFFFFFFFU;
    
    if (expected != received) {
        if (expected < received)
            rc = ERROR_MSG_LOST;  // not CAN API compliant
        else
            rc = ERROR_MSG_FATAL;  // issue #198 occurred!
    } else
        expected += 1U;
    return rc;
}

// $Id: Tester.cpp 1341 2024-06-15 16:43:48Z makemake $  Copyright (c) UV Software, Berlin //
