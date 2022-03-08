#!/usr/bin/env python3
#
#	SerialCAN - CAN API V3 Driver for CAN over Serial-line Interfaces
#
#	Copyright (C) 2020-2022  Uwe Vogt, UV Software, Berlin (info@uv-software.com)
#	All rights reserved.
#
#	This file is part of SerialCAN.
#
#	SerialCAN is dual-licensed under the BSD 2-Clause "Simplified" License
#	and under the GNU General Public License v3.0 (or any later version). You can
#	choose between one of them if you use SerialCAN in whole or in part.
#
#	BSD 2-Clause "Simplified" License:
#	Redistribution and use in source and binary forms, with or without
#	modification, are permitted provided that the following conditions are met:
#	1. Redistributions of source code must retain the above copyright notice, this
#	   list of conditions and the following disclaimer.
#	2. Redistributions in binary form must reproduce the above copyright notice,
#	   this list of conditions and the following disclaimer in the documentation
#	   and/or other materials provided with the distribution.
#
#	SerialCAN IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
#	AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
#	IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
#	DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
#	FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
#	DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
#	SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
#	CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
#	OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
#	OF SerialCAN, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#
#	GNU General Public License v3.0 or later:
#	SerialCAN is free software: you can redistribute it and/or modify
#	it under the terms of the GNU General Public License as published by
#	the Free Software Foundation, either version 3 of the License, or
#	(at your option) any later version.
#
#	SerialCAN is distributed in the hope that it will be useful,
#	but WITHOUT ANY WARRANTY; without even the implied warranty of
#	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#	GNU General Public License for more details.
#
#	You should have received a copy of the GNU General Public License
#	along with SerialCAN.  If not, see <http://www.gnu.org/licenses/>.
#
"""
    CAN API V3 Python Wrapper for CAN-over-Serial-Line Interfaces.

    CAN API V3 is a wrapper specification to have a uniform CAN
    Interface API for various CAN interfaces from different
    vendors running under multiple operating systems.

    $Author: haumea $

    $Rev: 1046 $
"""
from CANAPI import *
from ctypes import *


# CAN-over-Serial-Line interfaces
#
CANDEV_SERIAL = -1  # channel ID for serial port device

# SerialCAN protocol options
#
CANSIO_SLCAN = 0x00  # Lawicel SLCAN protocol

# Number of data bits (5, 6, 7, 8)
#
CANSIO_5DATABITS = 5  # 5 bits per data byte
CANSIO_6DATABITS = 6  # 6 bits per data byte
CANSIO_7DATABITS = 7  # 7 bits per data byte
CANSIO_8DATABITS = 8  # 8 bits per data byte

# Parity bit (None, Even, Odd)
#
CANSIO_NOPARITY = 0    # no parity
CANSIO_ODDPARITY = 1   # odd parity
CANSIO_EVENPARITY = 2  # even parity

# Number of stop bits (1 or 2)
#
CANSIO_1STOPBIT = 1   # 1 stop bit
CANSIO_2STOPBITS = 2  # 2 stop bits


class SerialAttr(LittleEndianStructure):
    """
      SerialCAN port attributes
    """
    _fields_ = [
        ('baudrate', c_uint32),
        ('bytesize', c_uint8),
        ('parity', c_uint8),
        ('stopbits', c_uint8),
        ('options', c_uint8)
    ]
    def __init__(self):
        self.options = CANSIO_SLCAN
        self.baudrate = 115200
        self.bytesize = CANSIO_8DATABITS
        self.parity = CANSIO_NOPARITY
        self.stopbits = CANSIO_1STOPBIT

class SerialPort(LittleEndianStructure):
    """
      SerialCAN port parameters
    """
    _fields_ = [
        ('name', c_char_p),
        ('attr', SerialAttr)
    ]
