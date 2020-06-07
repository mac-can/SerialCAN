### Driver for CAN-over-Serial-Line Interfaces (SLCAN Protocol)

_Copyright &copy; 2016-2020  Uwe Vogt, UV Software, Berlin (info@uv-software.com)_

# CAN API V3 Driver for CAN-over-Serial-Line Interfaces

CAN API V3 is a wrapper specification to have a uniform CAN Interface API for various CAN interfaces from different vendors running under multiple operating systems.

This repo contains the source code for a _CAN-over-Serial-Line_ driver based on
[Lawicel SLCAN protocol](http://www.can232.com/docs/canusb_manual.pdf).
It provides the build environments to build dynamic libraries with GNU C/C++&reg; compilers,
either as a C++ class library ([_libSerialCAN_](#libSerialCAN)),
or as a _CAN API V3_ driver library ([_libUVCANSLC_](#libUVCANSLC)),
as well as some C/C++ example programs ([`can_moni`](can_moni-for-serialcan) and [`can_test`](can_test-for-serialcan)).

## SerialCAN API

```C++
/// \name   SerialCAN API
/// \brief  CAN API V3 driver for CAN-over-Serial-Line interfaces
/// \note   See CCANAPI for a description of the overridden methods
/// \{
class CSerialCAN : public CCANAPI {
private:
    char m_szTtyName[CANPROP_MAX_BUFFER_SIZE];  ///< TTY device name
    CANAPI_OpMode_t m_OpMode;  ///< CAN operation mode
    CANAPI_Status_t m_Status;  ///< CAN status register
    CANAPI_Bitrate_t m_Bitrate;  ///< CAN bitrate settings
    struct {
        uint64_t u64TxMessages;  ///< number of transmitted CAN messages
        uint64_t u64RxMessages;  ///< number of received CAN messages
        uint64_t u64ErrorFrames;  ///< number of received status messages
    } m_Counter;
    // opaque data type
    struct SSLCAN;  ///< C++ forward declaration
    SSLCAN* m_pSLCAN;  ///< serial line interface
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
    static CANAPI_Return_t ProbeChannel(const char *device, CANAPI_OpMode_t opMode, EChannelState &state);
    static CANAPI_Return_t ProbeChannel(const char *device, CANAPI_OpMode_t opMode, SSerialAttributes sioAttr, EChannelState &state);

    CANAPI_Return_t InitializeChannel(const char *device, can_mode_t opMode);
    CANAPI_Return_t InitializeChannel(const char *device, can_mode_t opMode, SSerialAttributes sioAttr);

    // CCANAPI overrides
    static CANAPI_Return_t ProbeChannel(int32_t channel, CANAPI_OpMode_t opMode, const void *param, EChannelState &state);
    static CANAPI_Return_t ProbeChannel(int32_t channel, CANAPI_OpMode_t opMode, EChannelState &state);

    CANAPI_Return_t InitializeChannel(int32_t channel, can_mode_t opMode, const void *param = NULL);
    CANAPI_Return_t TeardownChannel();

    CANAPI_Return_t StartController(CANAPI_Bitrate_t bitrate);
    CANAPI_Return_t ResetController();

    CANAPI_Return_t WriteMessage(CANAPI_Message_t message, uint16_t timeout = 0U);
    CANAPI_Return_t ReadMessage(CANAPI_Message_t &message, uint16_t timeout = CANREAD_INFINITE);

    CANAPI_Return_t GetStatus(CANAPI_Status_t &status);
    CANAPI_Return_t GetBusLoad(uint8_t &load);

    CANAPI_Return_t GetBitrate(CANAPI_Bitrate_t &bitrate);
    CANAPI_Return_t GetBusSpeed(CANAPI_BusSpeed_t &speed);

    CANAPI_Return_t GetProperty(uint16_t param, void *value, uint32_t nbytes);
    CANAPI_Return_t SetProperty(uint16_t param, const void *value, uint32_t nbytes);

    char *GetHardwareVersion();  // (for compatibility reasons)
    char *GetFirmwareVersion();  // (for compatibility reasons)
    static char *GetVersion();  // (for compatibility reasons)

    static CANAPI_Return_t MapIndex2Bitrate(int32_t index, CANAPI_Bitrate_t &bitrate);
    static CANAPI_Return_t MapString2Bitrate(const char *string, CANAPI_Bitrate_t &bitrate);
    static CANAPI_Return_t MapBitrate2String(CANAPI_Bitrate_t bitrate, char *string, size_t length);
    static CANAPI_Return_t MapBitrate2Speed(CANAPI_Bitrate_t bitrate, CANAPI_BusSpeed_t &speed);
private:
    CANAPI_Return_t MapBitrate2Sja1000(CANAPI_Bitrate_t bitrate, uint16_t &btr0btr1);
    CANAPI_Return_t MapSja10002Bitrate(uint16_t btr0btr1, CANAPI_Bitrate_t &bitrate);
};
/// \}
```

### Build Targets

_Important note_: To build any of the following build targets run the `build_no.sh` script to generate a pseudo build number.
```
uv-pc013mac:~ eris$ cd ~/Projects/MacCAN/SerialCAN/Sources/
uv-pc013mac:Sources eris$ ./build_no.sh
```
Repeat this step after each `git commit`, `git pull`, `git clone`, etc.

Then go back to root folder and compile the whole _bleep_ by typing the usual commands:
```
uv-pc013mac:Sources eris$ cd ~/Projects/MacCAN/SerialCAN
uv-pc013mac:SerialCAN eris$ make clean
uv-pc013mac:SerialCAN eris$ make all
uv-pc013mac:SerialCAN eris$ sudo make install
```
_(The version number of the libraries can be adapted by editing the `Makefile`s in the subfolders and changing the variable `VERSION` accordingly.)_

#### libSerialCAN

___libSerialCAN___ is a dynamic library with a CAN API V3 compatible application programming interface for use in __C++__ applications.
See header file `SerialCAN.h` for a description of all class members.

#### libUVCANSLC

___libUVCANSLC___ is a dynamic library with a CAN API V3 compatible application programming interface for use in __C__ applications.
See header file `can_api.h` for a description of all API functions.

#### libSLCAN

___libSLCAN___ is a dynamic library with a basic SLCAN application programming interface for use in __C__ applications.
See header file `slcan.h` for a description of all API functions.

#### can_moni for SerialCAN

`can_moni` is a command line tool to view incoming CAN messages.
I hate this messing around with binary masks for identifier filtering.
So I wrote this little program to have an exclude list for single identifiers or identifier ranges (see program option `--exclude` or just `-x`). Precede the list with a `~` and you get an include list.

Type `can_moni --help` to display all program options.

#### can_test for SerialCAN

`can_test` is a command line tool to test CAN communication.
Originally developed for electronic environmental tests on an embedded Linux system with SocketCAN, I´m using it for many years as a traffic generator for CAN stress-tests.

Type `can_test --help` to display all program options.

### Target Platforms

POSIX&reg; compatible operating systems:

1. macOS&reg;
2. Linux&reg;
3. Cygwin&reg;

### Development Environments

#### macOS Catalina

- macOS Catalina (10.15) on a MacBook Pro (2019)
- Apple clang version 11.0.3 (clang-1103.0.32.62)
- Xcode Version 11.5 (11E608c)

#### macOS High Sierra

- macOS High Sierra (10.13.6) on a MacBook Pro (late 2011)
- Apple LLVM version 10.0.0 (clang-1000.11.45.5)
- Xcode Version 10.1 (10B61)

#### Debian Stretch (9.12)

- Debian 4.9.210-1 (2020-01-20) x86_64 GNU/Linux
- gcc (Debian 6.3.0-18+deb9u1) 6.3.0 20170516

#### Cygwin (64-bit)

- Cygwin 3.1.5(0.340/5/3) under Windows 10 Pro (Version 1909)
- GNU C/C++ Compiler (GCC) 9.3.0

### CAN Hardware

- Lawicel CANUSB (Hardware 1.0, Firmware 1.1)

## Known Bugs and Caveats

1. To probe a CAN channel is not supported.

2. Time-stamps are currently not supported.

3. Serial line attributes (baud rate and mode) cannot be changed
   (default: 115.2kBaud, 8-N-1).

4. Transmitting messages over the TTY is extremely slow; approx. 16ms per frame.
   I guess this is because the transmission is acknowledged by the CAN device.

5. Findings from Code Analysis:
   The companion module `can_btr.c` contains some dead stores.

6. No libraries are build under Cygwin; only the example programs
   [`can_moni`](can_moni-for-serialcan) and [`can_test`](can_test-for-serialcan).

## This and That

### CAN API V3 Repo

The CAN API V3 sources are maintained in a SVN repo to synchronized them
between the different CAN API V3 driver repos via Git SVN bridge.

### Licenses

#### CAN API V3 License

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

#### SerialCAN License

SerialCAN is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

SerialCAN is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with SerialCAN.  If not, see <http://www.gnu.org/licenses/>.

### Trademarks

Mac and macOS are trademarks of Apple Inc., registered in the U.S. and other countries. \
POSIX is a registered trademark of the Institute of Electrical and Electronic Engineers, Inc. \
Windows is a registered trademark of Microsoft Corporation in the United States and/or other countries. \
GNU C/C++ is a registered trademark of Free Software Foundation, Inc. \
Linux is a registered trademark of Linus Torvalds. \
Cygwin is a registered trademark of Red Hat, Inc.

### Hazard Note

_If you connect your CAN device to a real CAN network when using this library, you might damage your application._

### Contact

Uwe Vogt \
UV Software \
Chausseestrasse 33a \
10115 Berlin \
Germany

E-Mail: mailto://info@mac.can.com \
Internet: https://www.mac-can.com

##### *Enjoy!*
