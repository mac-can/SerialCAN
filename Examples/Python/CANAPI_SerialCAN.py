#!/usr/bin/env python3
#
#   CAN Interface API, Version 3 (Interface Definition)
#
#   Copyright (C) 2004-2021  Uwe Vogt, UV Software, Berlin (info@uv-software.com)
#
#   This file is part of CAN API V3.
#
#   CAN API V3 is free software: you can redistribute it and/or modify
#   it under the terms of the GNU Lesser General Public License as published by
#   the Free Software Foundation, either version 3 of the License, or
#   (at your option) any later version.
#
#   CAN API V3 is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#   GNU Lesser General Public License for more details.
#
#   You should have received a copy of the GNU Lesser General Public License
#   along with CAN API V3.  If not, see <http://www.gnu.org/licenses/>.
#
"""
    CAN API V3 Python Wrapper for CAN-over-Serial-Line Interfaces.

    CAN API V3 is a wrapper specification to have a uniform CAN
    Interface API for various CAN interfaces from different
    vendors running under multiple operating systems.

    $Author: eris $

    $Rev: 923 $
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
