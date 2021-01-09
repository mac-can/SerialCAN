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
    CAN API V3 Python Wrapper for generic CAN Interfaces.

    CAN API V3 is a wrapper specification to have a uniform CAN
    Interface API for various CAN interfaces from different
    vendors running under multiple operating systems.

    $Author: haumea $

    $Rev: 924 $
"""
from ctypes import *
import platform
import argparse
import sys

# CAN API V3 - Python Wrapper
#
CAN_API_V3_PYTHON = {'major': 0, 'minor': 1, 'patch': 0}

# CAN Identifier Ranges
#
CAN_MAX_STD_ID = 0x7FF       # highest 11-bit identifier
CAN_MAX_XTD_ID = 0x1FFFFFFF  # highest 29-bit identifier

# CAN Payload Length and DLC Definition
#
CAN_MAX_DLC = 8  # max. data length code (CAN 2.0)
CAN_MAX_LEN = 8  # max. payload length (CAN 2.0)

# CAN FD Payload Length and DLC Definition
#
CANFD_MAX_DLC = 15  # max. data length code (CAN FD)
CANFD_MAX_LEN = 64  # max. payload length (CAN FD)
CANFD_DLC_TAB = {   # DLC to length (CAN FD)
    0x0: int(0),
    0x1: int(1),
    0x2: int(2),
    0x3: int(3),
    0x4: int(4),
    0x5: int(5),
    0x6: int(6),
    0x7: int(7),
    0x8: int(8),
    0x9: int(12),
    0xA: int(16),
    0xB: int(20),
    0xC: int(24),
    0xD: int(32),
    0xE: int(48),
    0xF: int(64)
}
CANFD_LEN_TAB = {   # length to DLC (CAN FD)
    0: c_uint8(0x0),
    1: c_uint8(0x1),
    2: c_uint8(0x2),
    3: c_uint8(0x3),
    4: c_uint8(0x4),
    5: c_uint8(0x5),
    6: c_uint8(0x6),
    7: c_uint8(0x7),
    8: c_uint8(0x8),
    (9, 10, 11, 12): c_uint8(0x9),
    (13, 14, 15, 16): c_uint8(0xA),
    (17, 18, 19, 20): c_uint8(0xB),
    (21, 22, 23, 24): c_uint8(0xC),
    (25, 26, 27, 28, 29, 30, 31, 32): c_uint8(0xD),
    (33, 34, 35, 36, 37, 39, 39, 40): c_uint8(0xE),
    (49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64): c_uint8(0xF)
}

# CAN 2.0 Predefined Bit-rates (as index acc. CiA)
#
CANBTR_INDEX_1M = c_int32(0)     # bit-rate: 1000 kbit/s
CANBTR_INDEX_800K = c_int32(-1)  # bit-rate:  800 kbit/s
CANBTR_INDEX_500K = c_int32(-2)  # bit-rate:  500 kbit/s
CANBTR_INDEX_250K = c_int32(-3)  # bit-rate:  250 kbit/s
CANBTR_INDEX_125K = c_int32(-4)  # bit-rate:  125 kbit/s
CANBTR_INDEX_100K = c_int32(-5)  # bit-rate:  100 kbit/s
CANBTR_INDEX_50K = c_int32(-6)   # bit-rate:   50 kbit/s
CANBTR_INDEX_20K = c_int32(-7)   # bit-rate:   20 kbit/s
CANBTR_INDEX_10K = c_int32(-8)   # bit-rate:   10 kbit/s

#  CAN Controller Frequencies (depending on the CAN controller)
#
CANBTR_FREQ_80MHz = c_int32(80000000)   # frequency: 80 MHz
CANBTR_FREQ_60MHz = c_int32(60000000)   # frequency: 60 MHz
CANBTR_FREQ_40MHz = c_int32(40000000)   # frequency: 40 MHz
CANBTR_FREQ_30MHz = c_int32(30000000)   # frequency: 30 MHz
CANBTR_FREQ_24MHz = c_int32(24000000)   # frequency: 24 MHz
CANBTR_FREQ_20MHz = c_int32(20000000)   # frequency: 20 MHz
CANBTR_FREQ_SJA1000 = c_int32(8000000)  # frequency:  8 MHz

# CAN Mode Flags
#
CANMODE_FDOE = c_uint8(0x80)     # CAN FD operation enable/disable
CANMODE_BRSE = c_uint8(0x40)     # bit-rate switch enable/disable
CANMODE_NISO = c_uint8(0x20)     # Non-ISO CAN FD enable/disable
CANMODE_SHRD = c_uint8(0x10)     # shared access enable/disable
CANMODE_NXTD = c_uint8(0x08)     # extended format disable/enable
CANMODE_NRTR = c_uint8(0x04)     # remote frames disable/enable
CANMODE_ERR = c_uint8(0x02)      # error frames enable/disable
CANMODE_MON = c_uint8(0x01)      # monitor mode enable/disable
CANMODE_DEFAULT = c_uint8(0x00)  # CAN 2.0 operation mode

# General CAN error codes (negative)
# Note: Codes less or equal than -100 are for vendor-specific error codes
#        and codes less or equal than -10000 are for OS-specific error codes
#        (add 10000 to get the reported OS error code, e.g. errno).
#
CANERR_NOERROR = 0        # no error!
CANERR_BOFF = (-1)        # CAN - busoff status
CANERR_EWRN = (-2)        # CAN - error warning status
CANERR_BERR = (-3)        # CAN - bus error
CANERR_OFFLINE = (-9)     # CAN - not started
CANERR_ONLINE = (-8)      # CAN - already started
CANERR_MSG_LST = (-10)    # CAN - message lost
CANERR_LEC_STUFF = (-11)  # LEC - stuff error
CANERR_LEC_FORM = (-12)   # LEC - form error
CANERR_LEC_ACK = (-13)    # LEC - acknowledge error
CANERR_LEC_BIT1 = (-14)   # LEC - recessive bit error
CANERR_LEC_BIT0 = (-15)   # LEC - dominant bit error
CANERR_LEC_CRC = (-16)    # LEC - checksum error
CANERR_TX_BUSY = (-20)    # USR - transmitter busy
CANERR_RX_EMPTY = (-30)   # USR - receiver empty
CANERR_ERR_FRAME = (-40)  # USR - error frame
CANERR_TIMEOUT = (-50)    # USR - time-out
CANERR_RESOURCE = (-90)   # USR - resource allocation
CANERR_BAUDRATE = (-91)   # USR - illegal baudrate
CANERR_HANDLE = (-92)     # USR - illegal handle
CANERR_ILLPARA = (-93)    # USR - illegal parameter
CANERR_NULLPTR = (-94)    # USR - null-pointer assignment
CANERR_NOTINIT = (-95)    # USR - not initialized
CANERR_YETINIT = (-96)    # USR - already initialized
CANERR_LIBRARY = (-97)    # USR - illegal library
CANERR_NOTSUPP = (-98)    # USR - not supported
CANERR_FATAL = (-99)      # USR - other errors
CANERR_VENDOR = (-100)    # USR - vendor specific

# CAN status from CAN controller
#
CANSTAT_RESET = c_uint8(0x80)     # CAN status: controller stopped
CANSTAT_BOFF = c_uint8(0x40)      # CAN status: busoff status
CANSTAT_EWRN = c_uint8(0x20)      # CAN status: error warning level
CANSTAT_BERR = c_uint8(0x10)      # CAN status: bus error (LEC)
CANSTAT_TX_BUSY = c_uint8(0x08)   # CAN status: transmitter busy
CANSTAT_RX_EMPTY = c_uint8(0x04)  # CAN status: receiver empty
CANSTAT_MSG_LST = c_uint8(0x02)   # CAN status: message lost
CANSTAT_QUE_OVR = c_uint8(0x01)   # CAN status: event-queue overrun

# Results of the board test
#
CANBRD_NOT_PRESENT = (-1)   # CAN board not present
CANBRD_PRESENT = 0          # CAN board present
CANBRD_OCCUPIED = (+1)      # CAN board present, but occupied
CANBRD_NOT_TESTABLE = (-2)  # CAN board not testable (e.g. legacy API)

# Control of blocking read
#
CANREAD_INFINITE = c_uint16(65535)  # infinite time-out (blocking read)


# CAN Status-register
#
class StatusBits(LittleEndianStructure):
    """
      CAN Status-register: bit-wise access
    """
    _fields_ = [
        ('queue_overrun', c_uint8, 1),
        ('message_lost', c_uint8, 1),
        ('receiver_empty', c_uint8, 1),
        ('transmitter_busy', c_uint8, 1),
        ('bus_error', c_uint8, 1),
        ('warning_level', c_uint8, 1),
        ('bus_off', c_uint8, 1),
        ('can_stopped', c_uint8, 1)
    ]


class Status(Union):
    """
      CAN Status-register (as a union)
    """
    _fields_ = [
        ('byte', c_uint8),
        ('bits', StatusBits)
    ]


# CAN Operation Mode
#
class OpModeBits(LittleEndianStructure):
    """
      CAN Operation Mode: bit-wise access
    """
    _fields_ = [
        ('mon', c_uint8, 1),
        ('err', c_uint8, 1),
        ('nrtr', c_uint8, 1),
        ('nxtd', c_uint8, 1),
        ('shrd', c_uint8, 1),
        ('niso', c_uint8, 1),
        ('brse', c_uint8, 1),
        ('fdoe', c_uint8, 1)
    ]


class OpMode(Union):
    """
      CAN Operation Mode (as a union)
    """
    _fields_ = [
        ('byte', c_uint8),
        ('bits', OpModeBits)
    ]


# CAN Bit-rate settings
#
class BitrateNominal(LittleEndianStructure):
    """
      CAN Bit-rate settings: nominal bit-timing fields
    """
    _fields_ = [
        ('brp', c_uint16),
        ('tseg1', c_uint16),
        ('tseg2', c_uint16),
        ('sjw', c_uint16),
        ('sam', c_uint8)
    ]


class BitrateData(LittleEndianStructure):
    """
      CAN Bit-rate settings: data bit-timing fields
    """
    _fields_ = [
        ('brp', c_uint16),
        ('tseg1', c_uint16),
        ('tseg2', c_uint16),
        ('sjw', c_uint16)
    ]


class BitrateRegister(LittleEndianStructure):
    """
      CAN Bit-rate settings: bit-timing register
    """
    _fields_ = [
        ('frequency', c_int32),
        ('nominal', BitrateNominal),
        ('data', BitrateData)
    ]


class Bitrate(Union):
    """
      CAN Bit-rate settings (as a union)
    """
    _fields_ = [
        ('index', c_int32),
        ('btr', BitrateRegister)
    ]


# CAN Transmission Rate
#
class SpeedNominal(LittleEndianStructure):
    """
      CAN Nominal Transmission Rate
    """
    _fields_ = [
        ('fdoe', c_bool),
        ('speed', c_float),
        ('samplepoint', c_float),
    ]


class SpeedData(LittleEndianStructure):
    """
      CAN Data Transmission Rate
    """
    _fields_ = [
        ('brse', c_bool),
        ('speed', c_float),
        ('samplepoint', c_float),
    ]


class Speed(LittleEndianStructure):
    """
      CAN Transmission Rate (nominal and data)
    """
    _fields_ = [
        ('nominal', SpeedNominal),
        ('data', SpeedData),
    ]


# CAN Time-stamp
#
class Timestamp(LittleEndianStructure):
    """
      CAN Time-stamp: 'struct timespec' with nanoseconds resolution
    """
    _fields_ = [
        ('sec', c_long),
        ('nsec', c_long)
    ]


# CAN Message (CAN FD payload required!)
#
class MessageFlags(LittleEndianStructure):
    """
      CAN Message Flags
    """
    _fields_ = [
        ('xtd', c_uint8, 1),
        ('rtr', c_uint8, 1),
        ('fdf', c_uint8, 1),
        ('brs', c_uint8, 1),
        ('esi', c_uint8, 1),
        ('_unused', c_uint8, 2),
        ('sts', c_uint8, 1)
    ]


class Message(LittleEndianStructure):
    """
      CAN Message (with time-stamp)
    """
    _fields_ = [
        ('id', c_uint32),
        ('flags', MessageFlags),
        ('dlc', c_uint8),
        ('data', c_uint8 * 64),
        ('timestamp', Timestamp)
    ]


# CAN API V3 for generic CAN Interfaces
#
class CANAPI:
    """
      CAN API V3 class implementation
    """
    def __init__(self, library):
        #
        # constructor: loads the given CAN API V3 driver library
        #
        try:
            self.__m_library = cdll.LoadLibrary(library)
        except Exception as e:
            print('+++ exception: {}'.format(e))
            raise
        self.__m_handle = -1

    def __exit__(self):
        #
        # destructor: shutdown the CAN interface
        #
        if self.__m_handle != -1:
            self.__m_library.can_exit(self.__m_handle)
        self.__m_handle = -1

    def test(self, channel, mode, param=None):
        """
          probes if the CAN interface (hardware and driver) given by the argument 'channel' is present,
          and if the requested operation mode is supported by the CAN controller.

          :param channel: channel number of the CAN interface
          :param mode: operation mode to be checked
          :param param: channel-specific parameters (optional)
          :return: result, state
            result: 0 if successful, or a negative value on error
            state: state of the CAN channel:
                    < 0 - channel is not present,
                    = 0 - channel is present,
                    > 0 - channel is present, but in use
        """
        try:
            __state = c_int(CANBRD_NOT_TESTABLE)
            if param is not None:
                result = self.__m_library.can_test(c_int32(channel), c_uint8(mode.byte), byref(param), byref(__state))
            else:
                result = self.__m_library.can_test(c_int32(channel), c_uint8(mode.byte), None, byref(__state))
            return int(result), int(__state.value)
        except Exception as e:
            print('+++ exception: {}'.format(e))
            raise

    def init(self, channel, mode, param=None):
        """
          initializes the CAN interface (hardware and driver) given by the argument 'channel'.

          The operation state of the CAN controller is set to 'stopped'; no communication is possible in this state.

          :param channel: channel number of the CAN interface
          :param mode: desired operation mode of the CAN controller
          :param param: channel-specific parameters (optional)
          :return: 0 if successful, or a negative value on error
        """
        try:
            if param is not None:
                result = self.__m_library.can_init(c_int32(channel), c_uint8(mode.byte), byref(param))
            else:
                result = self.__m_library.can_init(c_int32(channel), c_uint8(mode.byte), None)
            if result >= 0:
                self.__m_handle = result
                return 0
            else:
                self.__m_handle = -1
                return int(result)
        except Exception as e:
            print('+++ exception: {}'.format(e))
            raise

    def exit(self):
        """
          stops any operation of the CAN interface and sets the operation state of the CAN controller to 'stopped'.

          :return: 0 if successful, or a negative value on error
        """
        try:
            result = self.__m_library.can_exit(self.__m_handle)
            return int(result)
        except Exception as e:
            print('+++ exception: {}'.format(e))
            raise

    def kill(self):
        """
          signals a waiting event object of the CAN interface. This can be used to terminate a blocking operation
          in progress (e.g. by means of a Ctrl-C handler or similar).

          Some drivers are using waitable objects to realize blocking operations by a call to WaitForSingleObject
          (Windows) or pthread_cond_wait (POSIX), but these waitable objects are no cancellation points.
          This means that they cannot be terminated by Ctrl-C (SIGINT).

          :return: 0 if successful, or a negative value on error
        """
        try:
            result = self.__m_library.can_kill(self.__m_handle)
            return int(result)
        except Exception as e:
            print('+++ exception: {}'.format(e))
            raise

    def start(self, bitrate):
        """
          initializes the operation mode and the bit-rate settings of the CAN interface
          and sets the operation state of the CAN controller to 'running'.

          Note: All statistical counters (tx/rx/err) will be reset by this.

          :param bitrate: bit-rate as btr register or baud rate index
          :return: 0 if successful, or a negative value on error
        """
        try:
            result = self.__m_library.can_start(self.__m_handle, byref(bitrate))
            return int(result)
        except Exception as e:
            print('+++ exception: {}'.format(e))
            raise

    def reset(self):
        """
          stops any operation of the CAN interface and sets the operation state of the CAN controller to 'stopped';
          no communication is possible in this state.

          :return: 0 if successful, or a negative value on error
        """
        try:
            result = self.__m_library.can_reset(self.__m_handle)
            return int(result)
        except Exception as e:
            print('+++ exception: {}'.format(e))
            raise

    def write(self, message, timeout=None):
        """
          transmits one message over the CAN bus. The CAN controller must be in operation state 'running'.

          :param message: the message to be sent
          :param timeout: time to wait for the transmission of the message:
                            0 means the function returns immediately,
                            65535 means blocking write, and any other
                            value means the time to wait im milliseconds
          :return: 0 if successful, or a negative value on error
        """
        try:
            if timeout is not None:
                result = self.__m_library.can_write(self.__m_handle, byref(message), c_uint16(timeout))
            else:
                result = self.__m_library.can_write(self.__m_handle, byref(message), c_uint16(0))
            return int(result)
        except Exception as e:
            print('+++ exception: {}'.format(e))
            raise

    def read(self, timeout=None):
        """
          read one message from the message queue of the CAN interface, if any message was received.
          The CAN controller must be in operation state 'running'.

          :param timeout: time to wait for the reception of the message:
                            0 means the function returns immediately,
                            65535 means blocking read, and any other
                            value means the time to wait im milliseconds
          :return: result, message
            result: 0 if successful, or a negative value on error
            message: the message read from the message queue or None
        """
        try:
            __message = Message()
            if timeout is not None:
                result = self.__m_library.can_read(self.__m_handle, byref(__message), c_uint16(timeout))
            else:
                result = self.__m_library.can_read(self.__m_handle, byref(__message), CANREAD_INFINITE)
            if result == 0:
                return int(result), __message
            else:
                return int(result), None
        except Exception as e:
            print('+++ exception: {}'.format(e))
            raise

    def status(self):
        """
          retrieves the status register of the CAN interface.

          :return: result, status
            result: 0 if successful, or a negative value on error
            status: the 8-bit status register
        """
        try:
            __status = Status()
            result = self.__m_library.can_status(self.__m_handle, byref(__status))
            return int(result), __status
        except Exception as e:
            print('+++ exception: {}'.format(e))
            raise

    def busload(self):
        """
          retrieves the bus-load (in percent) of the CAN interface.

          :return: result, busload, status
            result: 0 if successful, or a negative value on error
            busload: bus-load in [%]
            status: the 8-bit status register
        """
        try:
            __busload = c_uint8(0)
            __status = Status()
            result = self.__m_library.can_busload(self.__m_handle, byref(__busload), byref(__status))
            return int(result), float(__busload.value), __status
        except Exception as e:
            print('+++ exception: {}'.format(e))
            raise

    def bitrate(self):
        """
          retrieves the bit-rate setting of the CAN interface.

          :return: result, bitrate, speed
            result: 0 if successful, or a negative value on error
            bitrate: the current bit-rate setting
            speed: the current transmission rate
        """
        try:
            __bitrate = Bitrate()
            __speed = Speed()
            result = self.__m_library.can_bitrate(self.__m_handle, byref(__bitrate), byref(__speed))
            return int(result), __bitrate, __speed
        except Exception as e:
            print('+++ exception: {}'.format(e))
            raise

    @staticmethod
    def version():
        """
          retrieves version information of the CAN API V3 Python Wrapper.

          :return: result, version
            result: 0 if successful, or a negative value on error
            version: version information as string or None
        """
        return 'CAN API V3 for generic CAN Interfaces (Python Wrapper {}.{}.{})'.format(
            CAN_API_V3_PYTHON['major'], CAN_API_V3_PYTHON['minor'], CAN_API_V3_PYTHON['patch'])

    @staticmethod
    def dlc2len(dlc):
        """
          converts a data length code (DLC) into a payload length.

          :param dlc: data length code (as c_uint8)
          :return: payload length (in byte)
        """
        return CANFD_DLC_TAB.get(int(dlc), '+++ error: invalid data length code (DLC)')

    @staticmethod
    def len2dlc(length):
        """
          converts a payload length into a data length code (DLC).

          :param length: payload length (in byte)
          :return: data length code (as c_uint8)
        """
        return CANFD_LEN_TAB[length]


if __name__ == '__main__':
    #
    # Simple testing of the wrapper
    #
    if platform.system() == 'Darwin':
        # macOS dynamic library
        lib = 'libUVCANPCB.dylib'
    elif platform.system() != 'Windows':
        # shared object library
        lib = 'libuvcansoc.so.1'
    else:
        # Windows DLL
        lib = 'u3canpcb.dll'
    chn = 81

    # parse the command line
    parser = argparse.ArgumentParser()
    parser.add_argument('input', type=str, nargs='?', default=lib,
                        help='CAN API V3 driver library, default=\'' + lib + '\'')
    parser.add_argument('--channel', type=int, nargs=1, default=[chn],
                        help='CAN interface (channel), default=' + str(chn))
    args = parser.parse_args()
    lib = args.input
    chn = args.channel[0]
    opMode = OpMode()
    opMode.byte = CANMODE_DEFAULT
    bitRate = Bitrate()
    bitRate.index = CANBTR_INDEX_250K

    # load the driver library
    print(CANAPI.version())
    print('>>> can = CANAPI(' + lib + ')')
    can = CANAPI(lib)

    # initialize the CAN interface
    print('>>> can.init({}, 0x{:02X})'.format(chn, opMode.byte))
    res = can.init(channel=chn, mode=opMode)
    if res < CANERR_NOERROR:
        sys.exit('+++ error: can.init returned {}'.format(res))
    res, status = can.status()
    if res < CANERR_NOERROR:
        print('+++ error: can.status returned {}'.format(res))
    else:
        print('>>> can.status() >>> 0x{:02X}'.format(status.byte))

    # start the CAN controller
    if bitRate.index > 0:   # FIXME: Expected type 'int', got 'c_int32[int]' instead
        print('>>> can.start([{},[{},{},{},{},{}],[{},{},{},{},])'.format(bitRate.btr.frequency,
                                                                          bitRate.btr.nominal.brp,
                                                                          bitRate.btr.nominal.tseg1,
                                                                          bitRate.btr.nominal.tseg2,
                                                                          bitRate.btr.nominal.sjw,
                                                                          bitRate.btr.nominal.sam,
                                                                          bitRate.btr.data.brp,
                                                                          bitRate.btr.data.tseg1,
                                                                          bitRate.btr.data.tseg2,
                                                                          bitRate.btr.data.sjw))
    else:
        print('>>> can.start([{}])'.format(bitRate.index))
    res = can.start(bitrate=bitRate)
    if res < CANERR_NOERROR:
        can.exit()
        sys.exit('+++ error: can.start returned {}'.format(res))
    res, status = can.status()
    if res < CANERR_NOERROR:
        print('+++ error: can.status returned {}'.format(res))
    else:
        print('>>> can.status() >>> 0x{:02X}'.format(status.byte))

    # insert coin here
    # ...

    # stop the CAN communication
    print('>>> can.reset()')
    res = can.reset()
    if res < CANERR_NOERROR:
        print('+++ error: can.reset returned {}'.format(res))
    res, status = can.status()
    if res < CANERR_NOERROR:
        print('+++ error: can.status returned {}'.format(res))
    else:
        print('>>> can.status() >>> 0x{:02X}'.format(status.byte))

    # shutdown the CAN interface
    print('>>> can.exit()')
    res = can.exit()
    if res < CANERR_NOERROR:
        print('+++ error: can.exit returned {}'.format(res))

    # have a great time
    print('Bye, bye!')

# * $Id: CANAPI.py 924 2021-01-09 15:54:05Z haumea $ *** (C) UV Software, Berlin ***
#
