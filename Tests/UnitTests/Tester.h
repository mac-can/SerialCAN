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
#ifndef TESTER_H_INCLUDED
#define TESTER_H_INCLUDED

//  HowTo: make a copy of header 'Template.h' and name it 'Driver.h'
//         set define 'CDriverCAN' to your MacCAN Driver Under Test
//         and adapt the driver specific test settings there
#include "Driver.h"
#include "Settings.h"
#include "Parameter.h"

class CTester : CDriverCAN {
public:
    // constructor / destructor
    CTester() {};
    virtual ~CTester() {};
    // methods
    int SendSomeFrames(int handle, int32_t channel, int frames = 1, uint32_t canId = 0x100U);
    int ReceiveSomeFrames(int handle, int32_t channel, int frames = 1, uint32_t canId = 0x200U);
private:
    int CheckReceivedId(const CANAPI_Message_t &message, int32_t canId);
    int CheckReceivedDlc(const CANAPI_Message_t &message, uint8_t canDlc);
    int CheckReceivedData(const CANAPI_Message_t &message, uint64_t &expected);
public:
    static uint8_t Dlc2Len(uint8_t dlc) { return CCanApi::Dlc2Len(dlc); }
    static uint8_t Len2Dlc(uint8_t len) { return CCanApi::Len2Dlc(len); }};

#endif // TESTER_H_INCLUDED

// $Id: Tester.h 1341 2024-06-15 16:43:48Z makemake $  Copyright (c) UV Software, Berlin //
