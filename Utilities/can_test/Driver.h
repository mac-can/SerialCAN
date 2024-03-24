//  SPDX-License-Identifier: GPL-3.0-or-later
//
//  CAN Tester for CAN-over-Serial-Line Interfaces
//
//  Copyright (C) 2007,2016-2024  Uwe Vogt, UV Software, Berlin (info@uv-software.com)
//
//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program.  If not, see <https://www.gnu.org/licenses/>.
//
#ifndef DRIVER_H_INCLUDED
#define DRIVER_H_INCLUDED
#include "build_no.h"
#define VERSION_MAJOR      0
#define VERSION_MINOR      1
#define VERSION_PATCH      1
#define VERSION_BUILD      BUILD_NO
#define VERSION_STRING     TOSTRING(VERSION_MAJOR) "." TOSTRING(VERSION_MINOR) "." TOSTRING(VERSION_PATCH) " (" TOSTRING(BUILD_NO) ")"
#if defined(_WIN64)
#define PLATFORM          "x64"
#elif defined(_WIN32)
#define PLATFORM          "x86"
#elif defined(__linux__)
#define PLATFORM          "Linux"
#elif defined(__APPLE__)
#define PLATFORM          "macOS"
#elif defined(__CYGWIN__)
#define PLATFORM          "Cygwin"
#else
#error Platform not supported
#endif
#if (OPTION_CAN_2_0_ONLY != 0)
#ifdef _MSC_VER
#pragma message ( "Compilation with legacy CAN 2.0 frame format!" )
#else
#warning Compilation with legacy CAN 2.0 frame format!
#endif
#else
#define CAN_FD_SUPPORTED   1  // don't touch that dial!
#endif
#define SERIAL_CAN_SUPPORTED  1  // requires additional parameter

#define TESTER_INTEFACE   "CAN-over-Serial-Line Interfaces"
#define TESTER_COPYRIGHT  "2007,2016-2024 by Uwe Vogt, UV Software, Berlin"

#include "SerialCAN.h"

typedef CSerialCAN  CCanDriver;

#endif // DRIVER_H_INCLUDED
