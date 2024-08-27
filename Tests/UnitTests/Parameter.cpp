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
#include "Parameter.h"
#include "Settings.h"

#include <stddef.h>

#if (SERIAL_CAN_SUPPORTED != 0)
static can_sio_param_t sio_param[2] = {};
static int init = 0;

static can_sio_param_t *get_sio_param(int device) {
    if (!init) {
        sio_param[0].name = (char*)TEST_SERIAL1;
        sio_param[1].name = (char*)TEST_SERIAL2;
        sio_param[0].attr.protocol = TEST_PROTOCOL1;
        sio_param[1].attr.protocol = TEST_PROTOCOL2;
        sio_param[1].attr.baudrate = sio_param[0].attr.baudrate = CANSIO_BD57600;
        sio_param[1].attr.bytesize = sio_param[0].attr.bytesize = CANSIO_8DATABITS;
        sio_param[1].attr.stopbits = sio_param[0].attr.stopbits = CANSIO_1STOPBIT;
        sio_param[1].attr.parity = sio_param[0].attr.parity = CANSIO_NOPARITY;
        init = 1;
    }
    if ((0 <= device) && (device <= 1))
        return &sio_param[device];
    else
        return NULL;
}
#endif

void *GetParameter(int device) {
#if (SERIAL_CAN_SUPPORTED != 0)
    // CAN over serial devices require an additional parameter (COM port)
    return (void*)get_sio_param(device);
#else
    // other USB CAN devices not (plug 'n' play)
    return (void*)NULL;
#endif
}

// $Id: Parameter.cpp 1341 2024-06-15 16:43:48Z makemake $  Copyright (c) UV Software, Berlin //
