#!/usr/bin/env python3
from CANAPI import *
import argparse
import signal
import sys


# ^C handler to abort the reception loop and exit
#
def sigterm(signo, frame):
    print()
    print('>>> can.exit()')
    result = can.exit()
    if result < 0:
        sys.exit('+++ error: can.exit returned {}'.format(result))
    else:
        sys.exit(0)


# CAN API V3 driver library
lib = 'libUVCANPCD.dylib'
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

# install ^C handler
signal.signal(signal.SIGTERM, sigterm)
signal.signal(signal.SIGINT, sigterm)

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
if int(bitRate.index) > 0:   # FIXME: Expected type 'int', got 'c_int32[int]' instead
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

# reception loop
print("Press ^C to abort...")
frames = 0
flag_xtd = ['S', 'X']
flag_rtr = [' ', 'R']
flag_fdf = [' ', 'F']
flag_brs = [' ', 'B']
flag_esi = [' ', 'E']
flag_sts = ['', ' <<< status frame']
while True:
    res, message = can.read()
    if res == CANERR_NOERROR:
        if message.flags.fdf:
            print('{}\t{:7}.{:04}\t{:03X}\t{}{}{}{}{} [{}] '.format(frames,
                                                                    message.timestamp.sec,
                                                                    int(message.timestamp.nsec / 100000),
                                                                    message.id,
                                                                    flag_xtd[message.flags.xtd],
                                                                    flag_rtr[message.flags.rtr],
                                                                    flag_fdf[message.flags.fdf],
                                                                    flag_brs[message.flags.brs],
                                                                    flag_esi[message.flags.esi],
                                                                    can.dlc2len(message.dlc)) +
                  ', '.join('{:02X}'.format(x) for x in message.data[:can.dlc2len(message.dlc)]) +
                  flag_sts[message.flags.sts])
        else:
            print('{}\t{:7}.{:04}\t{:03X}\t{}{} [{}] '.format(frames,
                                                              message.timestamp.sec,
                                                              int(message.timestamp.nsec / 100000),
                                                              message.id,
                                                              flag_xtd[message.flags.xtd],
                                                              flag_rtr[message.flags.rtr],
                                                              message.dlc) +
                  ' '.join('{:02X}'.format(x) for x in message.data[:message.dlc]) +
                  flag_sts[message.flags.sts])
        frames = frames + 1
    elif res != CANERR_RX_EMPTY:
        print('+++ error: can.read returned {}'.format(res))
        res, status = can.status()
        if res < CANERR_NOERROR:
            print('+++ error: can.status returned {}'.format(res))
        else:
            print('>>> can.status() >>> 0x{:02X}'.format(status.byte))
        break

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
