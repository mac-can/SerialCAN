### Library for CAN-over-Serial-Line Interfaces (SLCAN Protocol)

_Copyright &copy; 2016, 2020-2024  Uwe Vogt, UV Software, Berlin (info@uv-software.com)_ \
_All rights reserved._

# CAN API V3 Library for CAN-over-Serial-Line Interfaces

CAN API V3 is a wrapper specification to have a uniform CAN Interface API for various CAN interfaces from different vendors running under multiple operating systems.

This repo contains the source code for _CAN-over-Serial-Line_ interfaces based on
[Lawicel SLCAN protocol](http://www.can232.com/docs/canusb_manual.pdf).
It provides the build environments to build dynamic libraries on macOS&reg;, Linux&reg;, Cygwin&reg; and Windows&reg;,
either as a C++ class library ([_libSerialCAN_](#libSerialCAN)),
or as a _CAN API V3_ driver library ([_libUVCANSLC_](#libUVCANSLC)),
as well as my beloved utilities [`can_moni`](#can_moni) and [`can_test`](#can_test),
and some C/C++ example programs.

## SerialCAN API

```C++
/// \name   SerialCAN API
/// \brief  CAN API V3 driver for CAN-over-Serial-Line interfaces
/// \note   See CCanApi for a description of the overridden methods
/// \{
class CSerialCAN : public CCanApi {
public:
    // constructor / destructor
    CSerialCAN();
    ~CSerialCAN();
    // CSerialCAN-specific error codes (CAN API V3 extension)
    enum EErrorCodes {
        // note: range 0...-99 is reserved by CAN API V3
        GeneralError = VendorSpecific
    };
    // serial line attributes
    typedef can_sio_attr_t SSerialAttributes;

    // CSerial methods
    static CANAPI_Return_t ProbeChannel(const char *device, const CANAPI_OpMode_t &opMode, EChannelState &state);
    static CANAPI_Return_t ProbeChannel(const char *device, const CANAPI_OpMode_t &opMode, const SSerialAttributes &sioAttr, EChannelState &state);

    CANAPI_Return_t InitializeChannel(const char *device, const CANAPI_OpMode_t &opMode);
    CANAPI_Return_t InitializeChannel(const char *device, const CANAPI_OpMode_t &opMode, const SSerialAttributes &sioAttr);

    // CCanApi overrides
    static CANAPI_Return_t ProbeChannel(int32_t channel, const CANAPI_OpMode_t &opMode, const void *param, EChannelState &state);
    static CANAPI_Return_t ProbeChannel(int32_t channel, const CANAPI_OpMode_t &opMode, EChannelState &state);

    CANAPI_Return_t InitializeChannel(int32_t channel, const CANAPI_OpMode_t &opMode, const void *param = NULL);
    CANAPI_Return_t TeardownChannel();
    CANAPI_Return_t SignalChannel();

    CANAPI_Return_t StartController(CANAPI_Bitrate_t bitrate);
    CANAPI_Return_t ResetController();

    CANAPI_Return_t WriteMessage(CANAPI_Message_t message, uint16_t timeout = 0U);
    CANAPI_Return_t ReadMessage(CANAPI_Message_t &message, uint16_t timeout = CANREAD_INFINITE);

    CANAPI_Return_t GetStatus(CANAPI_Status_t &status);
    CANAPI_Return_t GetBusLoad(uint8_t &load);

    CANAPI_Return_t GetBitrate(CANAPI_Bitrate_t &bitrate);
    CANAPI_Return_t GetBusSpeed(CANAPI_BusSpeed_t &speed);

    CANAPI_Return_t GetProperty(uint16_t param, void *value, uint32_t nbyte);
    CANAPI_Return_t SetProperty(uint16_t param, const void *value, uint32_t nbyte);

    char *GetHardwareVersion();  // (for compatibility reasons)
    char *GetFirmwareVersion();  // (for compatibility reasons)
    static char *GetVersion();  // (for compatibility reasons)
};
/// \}
```

### Build Targets

_Important note_: To build any of the following build targets run the `build_no.sh` script to generate a pseudo build number.
```
uranus@uv-pc007linux:~$ cd ~/Projects/CAN/Drivers/SerialCAN
uranus@uv-pc007linux:~/Projects/CAN/Drivers/SerialCAN$ ./build_no.sh
```
Repeat this step after each `git commit`, `git pull`, `git clone`, etc.

Then you can build the whole _bleep_ by typing the usual commands:
```
uranus@uv-pc007linux:~$ cd ~/Projects/CAN/Drivers/SerialCAN
uranus@uv-pc007linux:~/Projects/CAN/Drivers/SerialCAN$ make clean
uranus@uv-pc007linux:~/Projects/CAN/Drivers/SerialCAN$ make all
uranus@uv-pc007linux:~/Projects/CAN/Drivers/SerialCAN$ sudo make install
```
_(The version number of the libraries can be adapted by editing the `Makefile`s in the subfolders and changing the variable `VERSION` accordingly.  Don´t forget to set the version number also in the source files.)_

#### libSerialCAN

___libSerialCAN___ is a dynamic library with a CAN API V3 compatible application programming interface for use in __C++__ applications.
See header file `SerialCAN.h` for a description of all class members.

#### libUVCANSLC

___libUVCANSLC___ is a dynamic library with a CAN API V3 compatible application programming interface for use in __C__ applications.
See header file `can_api.h` for a description of all API functions.

#### libSLCAN

___libSLCAN___ is a dynamic library with a basic SLCAN application programming interface for use in __C__ applications.
See header file `slcan.h` for a description of all API functions.

#### can_moni

`can_moni` is a command line tool to view incoming CAN messages.
I hate this messing around with binary masks for identifier filtering.
So I wrote this little program to have an exclude list for single identifiers or identifier ranges (see program option `--exclude` or just `-x`). Precede the list with a `~` and you get an include list.

Type `can_moni --help` to display all program options.

#### can_test

`can_test` is a command line tool to test CAN communication.
Originally developed for electronic environmental tests on an embedded Linux system with SocketCAN, I´m using it for many years as a traffic generator for CAN stress-tests.

Type `can_test --help` to display all program options.

### Target Platforms

POSIX&reg; compatible operating systems:

1. macOS&reg;
1. Linux&reg;
1. Cygwin&reg;

Windows&reg; (x64 operating system):

1. Windows 10 Pro
1. Windows 11 Pro

### Development Environments

#### macOS Sonoma

- macOS Sonoma (14.4) on a Mac mini (M1, 2020)
- Apple clang version 15.0.0 (clang-1500.3.9.4)
- Xcode Version 15.3 (15E204a)

#### macOS Monterey

- macOS Monterey (12.7.4) on a MacBook Pro (2019)
- Apple clang version 13.0.0 (clang-1300.0.29.30)
- Xcode Version 13.2.1 (13C100)

#### macOS High Sierra

- macOS High Sierra (10.13.6) on a MacBook Pro (late 2011)
- Apple LLVM version 10.0.0 (clang-1000.11.45.5)
- Xcode Version 10.1 (10B61)

#### Debian Bullseye (11.2)

- Debian 4.19.160-2 (2020-11-18) x86_64 GNU/Linux
- gcc (Debian 10.2.1-6) 10.2.1 20210110

#### Cygwin (64-bit)

- Cygwin  3.5.1-1.x86_64 under Windows 10 Pro
- GNU C/C++ Compiler (GCC) 11.4.0

#### Windows 10 & 11

- Microsoft Visual Studio Community 2022 (Version 17.9.4)

### CAN Hardware

- Lawicel CANUSB (Hardware 1.0, Firmware 1.1)

## Known Bugs and Caveats

1. Transmitting messages over the TTY is extremely slow; approx. 16ms per frame.
   I guess this is because the transmission is acknowledged by the CAN device.

2. Time-stamps are currently not supported.

3. Python Ctrl+C issue still unsolved.

## This and That

### CAN API V3 Reference

A generic documentation of the CAN API V3 application programming interface can be found [here](https://uv-software.github.io/CANAPI-Docs/#/).

### Dual-License

This work is dual-licensed under the terms of the BSD 2-Clause "Simplified" License
and under the terms of the GNU General Public License v3.0 (or any later version).
You can choose between one of them if you use this work in whole or in part.

`SPDX-License-Identifier: BSD-2-Clause OR GPL-3.0-or-later`

### Trademarks

Mac and macOS are trademarks of Apple Inc., registered in the U.S. and other countries. \
POSIX is a registered trademark of the Institute of Electrical and Electronic Engineers, Inc. \
Windows is a registered trademark of Microsoft Corporation in the United States and/or other countries. \
GNU C/C++ is a registered trademark of Free Software Foundation, Inc. \
Linux is a registered trademark of Linus Torvalds. \
Cygwin is a registered trademark of Red Hat, Inc. \
All other company, product and service names mentioned herein may be trademarks, registered trademarks, or service marks of their respective owners.

### Hazard Note

_If you connect your CAN device to a real CAN network when using this library, you might damage your application._

### Contact

E-Mail: mailto://info@mac.can.com \
Internet: https://www.mac-can.net
