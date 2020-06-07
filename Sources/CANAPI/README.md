### CAN Interface API, Version 3

_Copyright &copy; 2004-2020  Uwe Vogt, UV Software, Berlin (info@uv-software.com)_

Version $Rev: 902 $

# A CAN Interface Wrapper Specification

Originally, the CAN Interface API was based on a CAN interface definition as part of a microcontroller hardware abstraction layer for a 82527 compatible on-chip CAN controller.
It was developed for use by (simple hand-coded) CANopen applications and migrated to different microcontroller types, even if the CAN peripherals on the micro had a different design.

## CAN API V1

What works on microcontrollers should also work on PC.
So I started to use this interface definition as a wrapper specification for different CAN devices from various vendors: e.g. for IXXAT, PEAK, Vector, Kvaser, and also for Linux-CAN (a.k.a. SocketCAN).

## CAN API V2

Dealing around with 14 virtual Basic-CAN messages boxes and a FIFO upon a virtual Full-CAN message box was a little bit over-engineered and error-prone.
So I optimized the interface definition to have an easy to use API following an _init-start-read-write-stop-exit_ pattern.

## CAN API V3

Version 3 is the latest adaption of the CAN API wrapper specification.
As new features it supports CAN FD long frames and fast frames, selectable operation-modes, blocking-read, and is multi-channel capable.
Additionally it provides companion modules for bit-rate conversion and message formatting.

# CAN Interface API, Version 3

In case of doubt the source code:

```C
#if (OPTION_CANAPI_LIBRARY != 0)
extern int can_test(int32_t library, int32_t board, uint8_t mode, const void *param, int *result);
extern int can_init(int32_t library, int32_t board, uint8_t mode, const void *param);
#else
extern int can_test(int32_t board, uint8_t mode, const void *param, int *result);
extern int can_init(int32_t board, uint8_t mode, const void *param);
#endif
#if defined(_WIN32) || defined(_WIN64)
extern int can_kill(int handle);
#endif
extern int can_exit(int handle);

extern int can_start(int handle, const can_bitrate_t *bitrate);
extern int can_reset(int handle);

extern int can_write(int handle, const can_message_t *message, uint16_t timeout);
extern int can_read(int handle, can_message_t *message, uint16_t timeout);

extern int can_status(int handle, uint8_t *status);
extern int can_busload(int handle, uint8_t *load, uint8_t *status);

extern int can_bitrate(int handle, can_bitrate_t *bitrate, can_speed_t *speed);
extern int can_property(int handle, uint16_t param, void *value, uint32_t nbytes);

extern char *can_hardware(int handle);
extern char *can_software(int handle);

#if (OPTION_CANAPI_LIBRARY != 0)
extern char *can_library(int handle);
#endif
extern char* can_version();
```
See header file `can_api.h` for a description of the provided functions.

## This and That

### SVN Repo

The CAN API V3 sources are maintained in a SVN repo to synchronized them between the different CAN API V3 wrapper repos via Git SVN bridge.

### License

CAN API V3 is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

CAN API V3 is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with CAN API V3.  If not, see <http://www.gnu.org/licenses/>.


### Contact

Uwe Vogt \
UV Software \
Chausseestrasse 33a \
10115 Berlin \
Germany

E-Mail: mailto://info@uv-software.com \
Internet: https://www.uv-software.com

##### *Do one thing, and do it well!*
