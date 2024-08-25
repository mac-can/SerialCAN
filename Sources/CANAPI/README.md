### CAN Interface API, Version 3

_Copyright &copy; 2004-2024  Uwe Vogt, UV Software, Berlin (info@uv-software.com)_ \
_All rights reserved._

# A CAN Interface Wrapper Specification

Originally, the CAN Interface API was based on a CAN interface definition as part of a microcontroller hardware abstraction layer for an 82527-compatible on-chip CAN controller.
It was developed for use by (simple hand-coded) CANopen applications and migrated to different microcontroller types (even if the CAN peripherals on that micro had a different design).

## CAN API V1

What works on microcontrollers should also work on PC.
I started to use this interface definition as a wrapper specification for different CAN devices from various vendors: e.g. for IXXAT, PEAK, Vector, Kvaser, and also for Linux-CAN (aka SocketCAN).

## CAN API V2

Dealing around with 14 virtual Basic-CAN messages boxes and a FIFO upon a virtual Full-CAN message box was a little bit over-engineered and error-prone.
I optimized the interface definition to have an easy to use API following an _init-start-read-write-stop-exit_ pattern.

## CAN API V3

Version 3 is the latest adaption of the CAN API wrapper specification.
As new features it supports CAN FD long frames and fast frames, selectable operation-modes, blocking-read, and is multi-channel capable.
Additionally it provides companion modules for bit-rate conversion and message formatting.

# CAN Interface API, Version 3

In case of doubt the source code:

```C
#if (OPTION_CANAPI_LIBRARY != 0)
extern int can_test(int32_t library, int32_t channel, uint8_t mode, const void *param, int *result);
extern int can_init(int32_t library, int32_t channel, uint8_t mode, const void *param);
#else
extern int can_test(int32_t channel, uint8_t mode, const void *param, int *result);
extern int can_init(int32_t channel, uint8_t mode, const void *param);
#endif
extern int can_exit(int handle);
extern int can_kill(int handle);

extern int can_start(int handle, const can_bitrate_t *bitrate);
extern int can_reset(int handle);

extern int can_write(int handle, const can_message_t *message, uint16_t timeout);
extern int can_read(int handle, can_message_t *message, uint16_t timeout);

extern int can_status(int handle, uint8_t *status);
extern int can_busload(int handle, uint8_t *load, uint8_t *status);

extern int can_bitrate(int handle, can_bitrate_t *bitrate, can_speed_t *speed);
extern int can_property(int handle, uint16_t param, void *value, uint32_t nbyte);

extern char *can_hardware(int handle);
extern char *can_firmware(int handle);

#if (OPTION_CANAPI_LIBRARY != 0)
extern char *can_library(int handle);
#endif
extern char* can_version(void);
```
See header file `can_api.h` for a description of the provided functions.

## This and That

### CAN API V3 Reference

A generic documentation of the CAN API V3 application programming interface can be found [here](https://uv-software.github.io/CANAPI-Docs/#/).

### vanilla-json

The implementation is using [vanilla-json](https://github.com/uv-software/vanilla-json) to read CAN API wrapper configurations from JSON files.

**vanilla-json** is a very simple JSON parser by UV&nbsp;Software written in C90, and it has the same dual-license model as CAN API V3; see below.

### Dual-License

This work is dual-licensed under the terms of the BSD 2-Clause "Simplified" License and under the terms of the GNU General Public License v3.0 (or any later version).
You can choose between one of them if you use this work in whole or in part.

### Contact

E-Mail: mailto://info@uv-software.com \
Internet: https://www.uv-software.com
