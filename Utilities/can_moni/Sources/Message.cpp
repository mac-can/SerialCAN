/*
*  CAN Interface API, Version 3 (Message Formatter)
*
*  Copyright (C) 2020  Uwe Vogt, UV Software, Berlin (info@uv-software.com)
*
*  This file is part of CAN API V3.
*
*  CAN API V3 is free software: you can redistribute it and/or modify
*  it under the terms of the GNU Lesser General Public License as published by
*  the Free Software Foundation, either version 3 of the License, or
*  (at your option) any later version.
*
*  CAN API V3 is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*  GNU Lesser General Public License for more details.
*
*  You should have received a copy of the GNU Lesser General Public License
*  along with CAN API V3.  If not, see <http://www.gnu.org/licenses/>.
*/
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
