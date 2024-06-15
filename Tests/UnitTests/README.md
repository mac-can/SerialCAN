_Copyright &copy; 2004-2024 Uwe Vogt, UV Software, Berlin (info@uv-software.com)_ \
_All rights reserved._

Version $Rev: 1334 $

# A CAN Interface Wrapper Specification

[CAN API V3](https://mac-can.github.io/wrapper/canapi-v3/) is a wrapper specification by UV Software to have a uniform CAN Interface API for various CAN interfaces from different vendors running under multiple operating systems.
Due to the fact that the CAN APIs of the different OEMs are not compatible with each other, UV Software has defined a CAN Interface Wrapper specification.
Its goal is to have a multi-vendor, cross-platform CAN Interface API.

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

## CAN API V3 Testing

### Xcodetest

Tbd.

### GoogleTest

Tbd.

## This and That

### SVN Repo

The CAN API V3 Testing sources are maintained in a SVN repo to synchronized them between the different CAN API V3 wrapper repos.

### Dual-License

This work is dual-licensed under the terms of the BSD 2-Clause "Simplified" License and under the terms of the GNU General Public License v3.0 (or any later version).
You can choose between one of them if you use this work in whole or in part.

`SPDX-License-Identifier: BSD-2-Clause OR GPL-3.0-or-later`

### Contact

E-Mail: mailto://info@uv-software.com \
Internet: https://www.uv-software.com
