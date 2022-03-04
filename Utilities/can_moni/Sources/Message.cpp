//  SPDX-License-Identifier: BSD-2-Clause OR GPL-3.0-or-later
//
//  CAN Interface API, Version 3 (Message Formatter)
//
//  Copyright (c) 2020-2021 Uwe Vogt, UV Software, Berlin (info@uv-software.com)
//  All rights reserved.
//
//  This file is part of CAN API V3.
//
//  CAN API V3 is dual-licensed under the BSD 2-Clause "Simplified" License and
//  under the GNU General Public License v3.0 (or any later version).
//  You can choose between one of them if you use this file.
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
//  along with CAN API V3.  If not, see <http://www.gnu.org/licenses/>.
//
#include "Message.h"
#include "can_msg.h"

#include <stdio.h>
#include <string.h>

//  Methods to format a CAN message
//
bool CCanMessage::Format(TCanMessage message, uint64_t counter, char *string, size_t length) {
    char *szMessage = msg_format_message(&message, MSG_RX_MESSAGE, counter, 0);
    if (szMessage) {
        strncpy(string, szMessage, length);
        return true;
    } else
        return false;
}

bool CCanMessage::SetTimestampFormat(EFormatTimestamp option) {
    if (option == OptionAbsolute)
        (void) msg_set_fmt_time_format(MSG_FMT_TIME_HHMMSS);
    else
        (void) msg_set_fmt_time_format(MSG_FMT_TIME_SEC);
    return msg_set_fmt_time_stamp((msg_fmt_timestamp_t)option) ? true : false;
}

bool CCanMessage::SetIdentifierFormat(EFormatNumber option) {
    return msg_set_fmt_id((msg_fmt_number_t) option) ? true : false;
}

bool CCanMessage::SetDataFormat(EFormatNumber option) {
    return msg_set_fmt_data((msg_fmt_number_t) option) ? true : false;
}

bool CCanMessage::SetAsciiFormat(EFormatOption option) {
    return msg_set_fmt_ascii((msg_fmt_option_t) option) ? true : false;
}

bool CCanMessage::SetWraparound(EFormatWraparound option) {
    return msg_set_fmt_wraparound((msg_fmt_wraparound_t) option) ? true : false;
}
