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
#ifndef MESSAGE_H_INCLUDED
#define MESSAGE_H_INCLUDED

#include "CANAPI_Types.h"

/// \name   MacCAN Message fromatter
/// \brief  Methods to format a CAN message.
/// \{
class CCanMessage {
public:
    enum EFormatOption {
        OptionOff = CANPARA_OPTION_OFF,
        OptionOn = CANPARA_OPTION_ON
    };
    enum EFormatNumber {
        OptionHex = CANPARA_NUMBER_HEX,
        OptionDec = CANPARA_NUMBER_DEC,
        OptionOct = CANPARA_NUMBER_OCT
    };
    enum EFormatTimestamp {
        OptionZero = CANPARA_TIMESTAMP_ZERO,
        OptionAbsolute = CANPARA_TIMESTAMP_ABS,
        OptionRelative = CANPARA_TIMESTAMP_REL
    };
    enum EFormatWraparound {
        OptionWraparoundNo = CANPARA_WRAPAROUND_NO,
        OptionWraparound8 = CANPARA_WRAPAROUND_8,
        OptionWraparound10 = CANPARA_WRAPAROUND_10,
        OptionWraparound16 = CANPARA_WRAPAROUND_16,
        OptionWraparound32 = CANPARA_WRAPAROUND_32,
        OptionWraparound64 = CANPARA_WRAPAROUND_64
    };
    typedef can_message_t TCanMessage;
    static bool SetTimestampFormat(EFormatTimestamp option);
    static bool SetIdentifierFormat(EFormatNumber option);
    static bool SetDataFormat(EFormatNumber option);
    static bool SetAsciiFormat(EFormatOption option);
    static bool SetWraparound(EFormatWraparound option);
    static bool Format(TCanMessage message, uint64_t counter, char *string, size_t length);
};
/// \}

#endif /* MESSAGE_H_INCLUDED */
