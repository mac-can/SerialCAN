//  SPDX-License-Identifier: BSD-2-Clause OR GPL-3.0-or-later
//
//  CAN Interface API, Version 3 (Testing)
//
//  Copyright (c) 2004-2022 Uwe Vogt, UV Software, Berlin (info@uv-software.com)
//  All rights reserved.
//
//  This file is part of CAN API V3.
//
//  CAN API V3 is dual-licensed under the BSD 2-Clause "Simplified" License and
//  under the GNU General Public License v3.0 (or any later version).
//  You can choose between one of them if you use this file.
//
//  BSD 2-Clause "Simplified" License:
//  Redistribution and use in source and binary forms, with or without
//  modification, are permitted provided that the following conditions are met:
//  1. Redistributions of source code must retain the above copyright notice, this
//     list of conditions and the following disclaimer.
//  2. Redistributions in binary form must reproduce the above copyright notice,
//     this list of conditions and the following disclaimer in the documentation
//     and/or other materials provided with the distribution.
//
//  CAN API V3 IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
//  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
//  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
//  DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
//  FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
//  DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
//  SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
//  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
//  OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
//  OF CAN API V3, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
//  GNU General Public License v3.0 or later:
//  CAN API V3 is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  CAN API V3 is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with CAN API V3.  If not, see <http://www.gnu.org/licenses/>.
//
#include "Bitrates.h"

#include <string.h>
#include <iostream>

enum EBitrateValidity {
    BTR_CORRECT,
    BTR_RANGE,
    BTR_DOUBLE,
    BTR_VALUE,
    BTR_KEY,
    BTR_WRONG
};
static const struct SBitrateString {
    EBitrateValidity m_eValidity;
    bool m_fCanFdWithBrse;
    TBitrateString m_szValue;
} aBitrateString[] = {
    // valid CAN bit-rate strings: default values w/o bit-rate switching
    { BTR_CORRECT, false, (TBitrateString)"f_clock=80000000,nom_brp=2,nom_tseg1=31,nom_tseg2=8,nom_sjw=8" },   // 1000K w/o BRS
    { BTR_CORRECT, false, (TBitrateString)"f_clock_mhz=80,nom_brp=2,nom_tseg1=63,nom_tseg2=16,nom_sjw=16" },    // 500K w/o BRS
    { BTR_CORRECT, false, (TBitrateString)"f_clock=80000000,nom_brp=2,nom_tseg1=127,nom_tseg2=32,nom_sjw=32" }, // 250K w/o BRS
    { BTR_CORRECT, false, (TBitrateString)"f_clock_mhz=80,nom_brp=2,nom_tseg1=255,nom_tseg2=64,nom_sjw=64" },   // 125K w/o BRS
    // valid CAN bit-rate strings at their range limits
    { BTR_CORRECT, false, (TBitrateString)"f_clock=8000000,nom_brp=2,nom_tseg1=127,nom_tseg2=32,nom_sjw=32" },  // note: minimum frequency is 8MHz (SJA1000)
    { BTR_CORRECT, false, (TBitrateString)"f_clock=2147483647,nom_brp=2,nom_tseg1=127,nom_tseg2=32,nom_sjw=32" },
    { BTR_CORRECT, false, (TBitrateString)"f_clock=80000000,nom_brp=1,nom_tseg1=127,nom_tseg2=32,nom_sjw=32" },
    { BTR_CORRECT, false, (TBitrateString)"f_clock=80000000,nom_brp=1024,nom_tseg1=127,nom_tseg2=32,nom_sjw=32" },
    { BTR_CORRECT, false, (TBitrateString)"f_clock=80000000,nom_brp=2,nom_tseg1=1,nom_tseg2=32,nom_sjw=32" },
    { BTR_CORRECT, false, (TBitrateString)"f_clock=80000000,nom_brp=2,nom_tseg1=256,nom_tseg2=32,nom_sjw=32" },
    { BTR_CORRECT, false, (TBitrateString)"f_clock=80000000,nom_brp=2,nom_tseg1=127,nom_tseg2=1,nom_sjw=32" },
    { BTR_CORRECT, false, (TBitrateString)"f_clock=80000000,nom_brp=2,nom_tseg1=127,nom_tseg2=128,nom_sjw=32" },
    { BTR_CORRECT, false, (TBitrateString)"f_clock=80000000,nom_brp=2,nom_tseg1=127,nom_tseg2=32,nom_sjw=1" },
    { BTR_CORRECT, false, (TBitrateString)"f_clock=80000000,nom_brp=2,nom_tseg1=127,nom_tseg2=32,nom_sjw=128" },
    // invalid CAN bit-rate strings (out of range)
    { BTR_VALUE,   false, (TBitrateString)"f_clock=0,nom_brp=2,nom_tseg1=127,nom_tseg2=32,nom_sjw=32" },  // note: wrong value because frequency is less than 8MHz
    { BTR_VALUE,   false, (TBitrateString)"f_clock_mhz=0,nom_brp=2,nom_tseg1=127,nom_tseg2=32,nom_sjw=32" },  //   -"-
    { BTR_RANGE,   false, (TBitrateString)"f_clock=80000000,nom_brp=0,nom_tseg1=127,nom_tseg2=32,nom_sjw=32" },
    { BTR_RANGE,   false, (TBitrateString)"f_clock=80000000,nom_brp=1025,nom_tseg1=127,nom_tseg2=32,nom_sjw=32" },
    { BTR_RANGE,   false, (TBitrateString)"f_clock=80000000,nom_brp=65535,nom_tseg1=127,nom_tseg2=32,nom_sjw=32" },
    { BTR_RANGE,   false, (TBitrateString)"f_clock=80000000,nom_brp=2,nom_tseg1=0,nom_tseg2=32,nom_sjw=32" },
    { BTR_RANGE,   false, (TBitrateString)"f_clock=80000000,nom_brp=2,nom_tseg1=257,nom_tseg2=32,nom_sjw=32" },
    { BTR_RANGE,   false, (TBitrateString)"f_clock=80000000,nom_brp=2,nom_tseg1=65535,nom_tseg2=32,nom_sjw=32" },
    { BTR_RANGE,   false, (TBitrateString)"f_clock=80000000,nom_brp=2,nom_tseg1=127,nom_tseg2=0,nom_sjw=32" },
    { BTR_RANGE,   false, (TBitrateString)"f_clock=80000000,nom_brp=2,nom_tseg1=127,nom_tseg2=129,nom_sjw=32" },
    { BTR_RANGE,   false, (TBitrateString)"f_clock=80000000,nom_brp=2,nom_tseg1=127,nom_tseg2=65535,nom_sjw=32" },
    { BTR_RANGE,   false, (TBitrateString)"f_clock=80000000,nom_brp=2,nom_tseg1=127,nom_tseg2=32,nom_sjw=0" },
    { BTR_RANGE,   false, (TBitrateString)"f_clock=80000000,nom_brp=2,nom_tseg1=127,nom_tseg2=32,nom_sjw=129" },
    { BTR_RANGE,   false, (TBitrateString)"f_clock=80000000,nom_brp=2,nom_tseg1=127,nom_tseg2=32,nom_sjw=65535" },
    // valid CAN bit-rate strings with field 'nom_sam'
    { BTR_CORRECT, false, (TBitrateString)"f_clock=80000000,nom_brp=2,nom_tseg1=127,nom_tseg2=32,nom_sjw=32,nom_sam=0" },
    { BTR_CORRECT, false, (TBitrateString)"f_clock=80000000,nom_brp=2,nom_tseg1=127,nom_tseg2=32,nom_sjw=32,nom_sam=1" },
    // invalid CAN bit-rate strings with field 'nom_sam'
    { BTR_RANGE,   false, (TBitrateString)"f_clock=80000000,nom_brp=2,nom_tseg1=127,nom_tseg2=32,nom_sjw=32,nom_sam=2" },  // note: to be ignored in CAN FD mode
    { BTR_RANGE,   false, (TBitrateString)"f_clock=80000000,nom_brp=2,nom_tseg1=127,nom_tseg2=32,nom_sjw=32,nom_sam=255" },  //     -"-
    // valid CAN bit-rate strings in arbitrary order
    { BTR_CORRECT, false, (TBitrateString)"f_clock=80000000,nom_tseg1=127,nom_brp=2,nom_sjw=32,nom_tseg2=32" },
    { BTR_CORRECT, false, (TBitrateString)"nom_brp=2,f_clock=80000000,nom_tseg2=32,nom_tseg1=127,nom_sjw=32" },
    { BTR_CORRECT, false, (TBitrateString)"nom_tseg1=127,nom_brp=2,f_clock=80000000,nom_sjw=32,nom_tseg2=32" },
    { BTR_CORRECT, false, (TBitrateString)"nom_tseg2=32,nom_tseg1=127,nom_brp=2,f_clock=80000000,nom_sjw=32" },
    { BTR_CORRECT, false, (TBitrateString)"nom_sjw=32,nom_brp=2,nom_tseg2=32,nom_tseg1=127,f_clock=80000000" },
    // incomplete CAN bit-rate strings (invalid)
    { BTR_VALUE,   false, (TBitrateString)"nom_brp=2,nom_tseg1=127,nom_tseg2=32,nom_sjw=32" },  // note: wrong because frequency is set to 0
    { BTR_RANGE,   false, (TBitrateString)"f_clock=80000000,nom_tseg1=127,nom_tseg2=32,nom_sjw=32" },
    { BTR_RANGE,   false, (TBitrateString)"f_clock=80000000,nom_brp=2,nom_tseg2=32,nom_sjw=32" },
    { BTR_RANGE,   false, (TBitrateString)"f_clock=80000000,nom_brp=2,nom_tseg1=127,nom_sjw=32" },
    { BTR_RANGE,   false, (TBitrateString)"f_clock=80000000,nom_brp=2,nom_tseg1=127,nom_tseg2=32" },
    // wrong key (typo, delimiter, unknown)
    { BTR_KEY,     false, (TBitrateString)"f_clock_Mhz=80,nom_brp=2,nom_tseg1=127,nom_tseg2=32,nom_sjw=32" },
    { BTR_KEY,     false, (TBitrateString)"g_clock=80000000,nom_brp=2,nom_tseg1=127,nom_tseg2=32,nom_sjw=32" },
    { BTR_KEY,     false, (TBitrateString)"f_clock=80000000,nom_bpr=2,nom_tseg1=127,nom_tseg2=32,nom_sjw=32" },
    { BTR_KEY,     false, (TBitrateString)"f_clock=80000000,nom_brp=2,nom-tseg1=127,nom_tseg2=32,nom_sjw=32" },
    { BTR_KEY,     false, (TBitrateString)"f_clock=80000000,nom_brp=2,nom_tseg1=127,nom_tseg3=32,nom_sjw=32" },
    { BTR_KEY,     false, (TBitrateString)"f_clock=80000000,nom_brp=2,nom_tseg1=127,nom_tseg2=32,nom_sjw:32" },
    { BTR_KEY,     false, (TBitrateString)"f_clock=80000000,nom_brp=2,nom_tseg1=127,nom_tseg2=32,nom_sjw=32,nom_key=42" },
    // wrong value (not a number, delimiter, exseeds its range)
    { BTR_VALUE,   false, (TBitrateString)"f_clock_mhz=eighty,nom_brp=2,nom_tseg1=127,nom_tseg2=32,nom_sjw=32" },
    { BTR_VALUE,   false, (TBitrateString)"f_clock_mhz=80,nom_brp=0x2,nom_tseg1=127,nom_tseg2=32,nom_sjw=32" },
    { BTR_VALUE,   false, (TBitrateString)"f_clock_mhz=80,nom_brp=2,nom_tseg1=1X7,nom_tseg2=32,nom_sjw=32" },
    { BTR_VALUE,   false, (TBitrateString)"f_clock_mhz=80,nom_brp=2,nom_tseg1=127,nom_tseg2=32h,nom_sjw=32" },
    { BTR_CORRECT, false, (TBitrateString)"f_clock_mhz=80,nom_brp=2,nom_tseg1=127,nom_tseg2=32,nom_sjw=32," },  // FIXME: bug in the scanner
    { BTR_VALUE,   false, (TBitrateString)"f_clock_mhz=80,nom_brp=2,nom_tseg1=127,nom_tseg2=32,nom_sjw=32;" },
    { BTR_VALUE,   false, (TBitrateString)"f_clock_mhz=2147484,nom_brp=2,nom_tseg1=127,nom_tseg2=32,nom_sjw=32" },
    { BTR_VALUE,   false, (TBitrateString)"f_clock_mhz=80,nom_brp=65536,nom_tseg1=127,nom_tseg2=32,nom_sjw=32" },
    { BTR_VALUE,   false, (TBitrateString)"f_clock_mhz=80,nom_brp=2,nom_tseg1=65536,nom_tseg2=32,nom_sjw=32" },
    { BTR_VALUE,   false, (TBitrateString)"f_clock_mhz=80,nom_brp=2,nom_tseg1=127,nom_tseg2=65536,nom_sjw=32" },
    { BTR_VALUE,   false, (TBitrateString)"f_clock_mhz=80,nom_brp=2,nom_tseg1=127,nom_tseg2=32,nom_sjw=65536" },
    { BTR_VALUE,   false, (TBitrateString)"f_clock_mhz=80,nom_brp=2,nom_tseg1=127,nom_tseg2=32,nom_sjw=32,nom_sam=256" },
    { BTR_VALUE,   false, (TBitrateString)"f_clock=2147483648,nom_brp=2,nom_tseg1=127,nom_tseg2=32,nom_sjw=32" },
    // duplicates (but valid)
    { BTR_DOUBLE,  false, (TBitrateString)"f_clock=80000000,nom_brp=2,nom_tseg1=127,nom_tseg2=32,nom_sjw=32," \
                                          "f_clock=80000000,nom_brp=2,nom_tseg1=127,nom_tseg2=32,nom_sjw=32" },
    { BTR_DOUBLE,  false, (TBitrateString)"f_clock=80000000,nom_brp=2,nom_tseg1=127,nom_tseg2=32,nom_sjw=32,f_clock_mhz=80" },
    { BTR_DOUBLE,  false, (TBitrateString)"f_clock=80000000,nom_brp=2,nom_tseg1=127,nom_tseg2=32,nom_sjw=32,nom_brp=2" },
    { BTR_DOUBLE,  false, (TBitrateString)"f_clock=80000000,nom_brp=2,nom_tseg1=127,nom_tseg2=32,nom_sjw=32,nom_tseg1=127" },
    { BTR_DOUBLE,  false, (TBitrateString)"f_clock=80000000,nom_brp=2,nom_tseg1=127,nom_tseg2=32,nom_sjw=32,nom_tseg2=32" },
    { BTR_DOUBLE,  false, (TBitrateString)"f_clock=80000000,nom_brp=2,nom_tseg1=127,nom_tseg2=32,nom_sjw=32,nom_sjw=32" },
    { BTR_DOUBLE,  false, (TBitrateString)"f_clock=80000000,nom_brp=2,nom_tseg1=127,nom_tseg2=32,nom_sjw=32,nom_sam=0,nom_sam=1" },
    // valid CAN bit-rate strings with blanks
    { BTR_CORRECT, false, (TBitrateString)" f_clock=80000000,nom_brp=2,nom_tseg1=127,nom_tseg2=32,nom_sjw=32" },
    { BTR_CORRECT, false, (TBitrateString)"f_clock =80000000,nom_brp=2,nom_tseg1=127,nom_tseg2=32,nom_sjw=32" },
    { BTR_CORRECT, false, (TBitrateString)"f_clock= 80000000,nom_brp=2,nom_tseg1=127,nom_tseg2=32,nom_sjw=32" },
    { BTR_CORRECT, false, (TBitrateString)"f_clock=80000000 ,nom_brp=2,nom_tseg1=127,nom_tseg2=32,nom_sjw=32" },
    { BTR_CORRECT, false, (TBitrateString)"f_clock=80000000, nom_brp=2,nom_tseg1=127,nom_tseg2=32,nom_sjw=32" },
    { BTR_CORRECT, false, (TBitrateString)"f_clock=80000000,nom_brp =2,nom_tseg1=127,nom_tseg2=32,nom_sjw=32" },
    { BTR_CORRECT, false, (TBitrateString)"f_clock=80000000,nom_brp=2 ,nom_tseg1=127,nom_tseg2=32,nom_sjw=32" },
    { BTR_CORRECT, false, (TBitrateString)"f_clock=80000000,nom_brp=2, nom_tseg1=127,nom_tseg2=32,nom_sjw=32" },
    { BTR_CORRECT, false, (TBitrateString)"f_clock=80000000,nom_brp=2,nom_tseg1 =127,nom_tseg2=32,nom_sjw=32" },
    { BTR_CORRECT, false, (TBitrateString)"f_clock=80000000,nom_brp=2,nom_tseg1= 127,nom_tseg2=32,nom_sjw=32" },
    { BTR_CORRECT, false, (TBitrateString)"f_clock=80000000,nom_brp=2,nom_tseg1=127 ,nom_tseg2=32,nom_sjw=32" },
    { BTR_CORRECT, false, (TBitrateString)"f_clock=80000000,nom_brp=2,nom_tseg1=127, nom_tseg2=32,nom_sjw=32" },
    { BTR_CORRECT, false, (TBitrateString)"f_clock=80000000,nom_brp=2,nom_tseg1=127,nom_tseg2 =32,nom_sjw=32" },
    { BTR_CORRECT, false, (TBitrateString)"f_clock=80000000,nom_brp=2,nom_tseg1=127,nom_tseg2= 32,nom_sjw=32" },
    { BTR_CORRECT, false, (TBitrateString)"f_clock=80000000,nom_brp=2,nom_tseg1=127,nom_tseg2=32 ,nom_sjw=32" },
    { BTR_CORRECT, false, (TBitrateString)"f_clock=80000000,nom_brp=2,nom_tseg1=127,nom_tseg2=32, nom_sjw=32" },
    { BTR_CORRECT, false, (TBitrateString)"f_clock=80000000,nom_brp=2,nom_tseg1=127,nom_tseg2=32,nom_sjw =32" },
    { BTR_CORRECT, false, (TBitrateString)"f_clock=80000000,nom_brp=2,nom_tseg1=127,nom_tseg2=32,nom_sjw= 32" },
    { BTR_CORRECT, false, (TBitrateString)"f_clock=80000000,nom_brp=2,nom_tseg1=127,nom_tseg2=32,nom_sjw=32 " },
    { BTR_CORRECT, false, (TBitrateString)" f_clock = 80000000 , nom_brp = 2 , nom_tseg1 = 127 , nom_tseg2 = 32 , nom_sjw = 32 " },
#if (OPTION_CAN_2_0_ONLY == OPTION_DISABLED)
    // valid CAN FD bit-rate strings: default values with bit-rate switching
    { BTR_CORRECT, true, (TBitrateString)"f_clock=80000000,nom_brp=2,nom_tseg1=31,nom_tseg2=8,nom_sjw=8,data_brp=2,data_tseg1=3,data_tseg2=1,data_sjw=1" },    // 1000K:8M
    { BTR_CORRECT, true, (TBitrateString)"f_clock_mhz=80,nom_brp=2,nom_tseg1=63,nom_tseg2=16,nom_sjw=16,data_brp=2,data_tseg1=7,data_tseg2=2,data_sjw=2" },     // 500K:4M
    { BTR_CORRECT, true, (TBitrateString)"f_clock=80000000,nom_brp=2,nom_tseg1=127,nom_tseg2=32,nom_sjw=32,data_brp=2,data_tseg1=15,data_tseg2=4,data_sjw=4" }, // 250K:2M
    { BTR_CORRECT, true, (TBitrateString)"f_clock_mhz=80,nom_brp=2,nom_tseg1=255,nom_tseg2=64,nom_sjw=64,data_brp=2,data_tseg1=31,data_tseg2=8,data_sjw=8" },   // 125K:1M
    // valid CAN FD bit-rate strings at their range limits
    { BTR_CORRECT, true, (TBitrateString)"f_clock=8000000,nom_brp=2,nom_tseg1=127,nom_tseg2=32,nom_sjw=32,data_brp=2,data_tseg1=15,data_tseg2=4,data_sjw=4" },  // note: minimum frequency is 8MHz (SJA1000)
    { BTR_CORRECT, true, (TBitrateString)"f_clock=2147483647,nom_brp=2,nom_tseg1=127,nom_tseg2=32,nom_sjw=32,data_brp=2,data_tseg1=15,data_tseg2=4,data_sjw=4" },
    { BTR_CORRECT, true, (TBitrateString)"f_clock=80000000,nom_brp=2,nom_tseg1=127,nom_tseg2=32,nom_sjw=32,data_brp=1,data_tseg1=15,data_tseg2=4,data_sjw=4" },
    { BTR_CORRECT, true, (TBitrateString)"f_clock=80000000,nom_brp=2,nom_tseg1=127,nom_tseg2=32,nom_sjw=32,data_brp=1024,data_tseg1=15,data_tseg2=4,data_sjw=4" },
    { BTR_CORRECT, true, (TBitrateString)"f_clock=80000000,nom_brp=2,nom_tseg1=127,nom_tseg2=32,nom_sjw=32,data_brp=2,data_tseg1=1,data_tseg2=4,data_sjw=4" },
    { BTR_CORRECT, true, (TBitrateString)"f_clock=80000000,nom_brp=2,nom_tseg1=127,nom_tseg2=32,nom_sjw=32,data_brp=2,data_tseg1=32,data_tseg2=4,data_sjw=4" },
    { BTR_CORRECT, true, (TBitrateString)"f_clock=80000000,nom_brp=2,nom_tseg1=127,nom_tseg2=32,nom_sjw=32,data_brp=2,data_tseg1=15,data_tseg2=1,data_sjw=4" },
    { BTR_CORRECT, true, (TBitrateString)"f_clock=80000000,nom_brp=2,nom_tseg1=127,nom_tseg2=32,nom_sjw=32,data_brp=2,data_tseg1=15,data_tseg2=16,data_sjw=4" },
    { BTR_CORRECT, true, (TBitrateString)"f_clock=80000000,nom_brp=2,nom_tseg1=127,nom_tseg2=32,nom_sjw=32,data_brp=2,data_tseg1=15,data_tseg2=4,data_sjw=1" },
    { BTR_CORRECT, true, (TBitrateString)"f_clock=80000000,nom_brp=2,nom_tseg1=127,nom_tseg2=32,nom_sjw=32,data_brp=2,data_tseg1=15,data_tseg2=4,data_sjw=16" },
    // invalid CAN FD bit-rate strings (out of range)
    { BTR_VALUE,   true, (TBitrateString)"f_clock=0,nom_brp=2,nom_tseg1=127,nom_tseg2=32,nom_sjw=32,data_brp=2,data_tseg1=15,data_tseg2=4,data_sjw=4" },  // note: wrong value because frequency is less than 8MHz (SJA1000)
    { BTR_VALUE,   true, (TBitrateString)"f_clock_mhz=0,nom_brp=2,nom_tseg1=127,nom_tseg2=32,nom_sjw=32,data_brp=2,data_tseg1=15,data_tseg2=4,data_sjw=4" },  //   -"-
    { BTR_RANGE,   true, (TBitrateString)"f_clock=80000000,nom_brp=2,nom_tseg1=127,nom_tseg2=32,nom_sjw=32,data_brp=0,data_tseg1=15,data_tseg2=4,data_sjw=4" },
    { BTR_RANGE,   true, (TBitrateString)"f_clock=80000000,nom_brp=2,nom_tseg1=127,nom_tseg2=32,nom_sjw=32,data_brp=1025,data_tseg1=15,data_tseg2=4,data_sjw=4" },
    { BTR_RANGE,   true, (TBitrateString)"f_clock=80000000,nom_brp=2,nom_tseg1=127,nom_tseg2=32,nom_sjw=32,data_brp=65535,data_tseg1=15,data_tseg2=4,data_sjw=4" },
    { BTR_RANGE,   true, (TBitrateString)"f_clock=80000000,nom_brp=2,nom_tseg1=127,nom_tseg2=32,nom_sjw=32,data_brp=2,data_tseg1=0,data_tseg2=4,data_sjw=4" },
    { BTR_RANGE,   true, (TBitrateString)"f_clock=80000000,nom_brp=2,nom_tseg1=127,nom_tseg2=32,nom_sjw=32,data_brp=2,data_tseg1=33,data_tseg2=4,data_sjw=4" },
    { BTR_RANGE,   true, (TBitrateString)"f_clock=80000000,nom_brp=2,nom_tseg1=127,nom_tseg2=32,nom_sjw=32,data_brp=2,data_tseg1=65535,data_tseg2=4,data_sjw=4" },
    { BTR_RANGE,   true, (TBitrateString)"f_clock=80000000,nom_brp=2,nom_tseg1=127,nom_tseg2=32,nom_sjw=32,data_brp=2,data_tseg1=15,data_tseg2=0,data_sjw=4" },
    { BTR_RANGE,   true, (TBitrateString)"f_clock=80000000,nom_brp=2,nom_tseg1=127,nom_tseg2=32,nom_sjw=32,data_brp=2,data_tseg1=15,data_tseg2=17,data_sjw=4" },
    { BTR_RANGE,   true, (TBitrateString)"f_clock=80000000,nom_brp=2,nom_tseg1=127,nom_tseg2=32,nom_sjw=32,data_brp=2,data_tseg1=15,data_tseg2=65535,data_sjw=4" },
    { BTR_RANGE,   true, (TBitrateString)"f_clock=80000000,nom_brp=2,nom_tseg1=127,nom_tseg2=32,nom_sjw=32,data_brp=2,data_tseg1=15,data_tseg2=4,data_sjw=0" },
    { BTR_RANGE,   true, (TBitrateString)"f_clock=80000000,nom_brp=2,nom_tseg1=127,nom_tseg2=32,nom_sjw=32,data_brp=2,data_tseg1=15,data_tseg2=4,data_sjw=17" },
    { BTR_RANGE,   true, (TBitrateString)"f_clock=80000000,nom_brp=2,nom_tseg1=127,nom_tseg2=32,nom_sjw=32,data_brp=2,data_tseg1=15,data_tseg2=4,data_sjw=65535" },
    // valid CAN FD bit-rate strings with field 'nom_sam' (to be ignored)
    { BTR_CORRECT, true, (TBitrateString)"f_clock=80000000,nom_brp=2,nom_tseg1=31,nom_tseg2=8,nom_sjw=8,nom_sam=0,data_brp=2,data_tseg1=3,data_tseg2=1,data_sjw=1" },
    { BTR_CORRECT, true, (TBitrateString)"f_clock_mhz=80,nom_brp=2,nom_tseg1=63,nom_tseg2=16,nom_sjw=16,nom_sam=1,data_brp=2,data_tseg1=7,data_tseg2=2,data_sjw=2" },
    { BTR_CORRECT, true, (TBitrateString)"f_clock=80000000,nom_brp=2,nom_tseg1=127,nom_tseg2=32,nom_sjw=32,nom_sam=2,data_brp=2,data_tseg1=15,data_tseg2=4,data_sjw=4" },
    { BTR_CORRECT, true, (TBitrateString)"f_clock_mhz=800,nom_brp=2,nom_tseg1=255,nom_tseg2=64,nom_sjw=64,nom_sam=255,data_brp=2,data_tseg1=31,data_tseg2=8,data_sjw=8" },
    // valid CAN FD bit-rate strings in arbitrary order
    { BTR_CORRECT, true, (TBitrateString)"f_clock=80000000,nom_brp=2,nom_tseg1=127,nom_tseg2=32,nom_sjw=32,data_tseg1=15,data_brp=2,data_sjw=4,data_tseg2=4" },
    { BTR_CORRECT, true, (TBitrateString)"data_brp=2,f_clock=80000000,nom_brp=2,nom_tseg1=127,nom_tseg2=32,nom_sjw=32,data_tseg2=4,data_tseg1=15,data_sjw=4" },
    { BTR_CORRECT, true, (TBitrateString)"data_tseg1=15,data_brp=2,f_clock=80000000,nom_brp=2,nom_tseg1=127,nom_tseg2=32,nom_sjw=32,data_sjw=4,data_tseg2=4" },
    { BTR_CORRECT, true, (TBitrateString)"data_tseg2=4,data_tseg1=15,data_brp=2,f_clock=80000000,nom_brp=2,nom_tseg1=127,nom_tseg2=32,nom_sjw=32,data_sjw=4" },
    { BTR_CORRECT, true, (TBitrateString)"data_sjw=4,data_brp=2,data_tseg2=4,data_tseg1=15,f_clock=80000000,nom_brp=2,nom_tseg1=127,nom_tseg2=32,nom_sjw=32" },
    // incomplete CAN bit-rate strings (invalid)
    { BTR_VALUE,   true, (TBitrateString)"nom_brp=2,nom_tseg1=127,nom_tseg2=32,nom_sjw=32,data_brp=2,data_tseg1=15,data_tseg2=4,data_sjw=4" },  // note: wrong because frequency is set to 0
    { BTR_RANGE,   true, (TBitrateString)"f_clock=80000000,nom_brp=2,nom_tseg1=127,nom_tseg2=32,nom_sjw=32,data_tseg1=15,data_tseg2=4,data_sjw=4" },
    { BTR_RANGE,   true, (TBitrateString)"f_clock=80000000,nom_brp=2,nom_tseg1=127,nom_tseg2=32,nom_sjw=32,data_brp=2,data_tseg2=4,data_sjw=4" },
    { BTR_RANGE,   true, (TBitrateString)"f_clock=80000000,nom_brp=2,nom_tseg1=127,nom_tseg2=32,nom_sjw=32,data_brp=2,data_tseg1=15,data_sjw=4" },
    { BTR_RANGE,   true, (TBitrateString)"f_clock=80000000,nom_brp=2,nom_tseg1=127,nom_tseg2=32,nom_sjw=32,data_brp=2,data_tseg1=15,data_tseg2=4" },
    // wrong key (typo, delimiter, unknown)
    { BTR_KEY,     true, (TBitrateString)"f_clock_Mhz=80,nom_brp=2,nom_tseg1=127,nom_tseg2=32,nom_sjw=32,data_brp=2,data_tseg1=15,data_tseg2=4,data_sjw=4" },
    { BTR_KEY,     true, (TBitrateString)"g_clock=80000000,nom_brp=2,nom_tseg1=127,nom_tseg2=32,nom_sjw=32,data_brp=2,data_tseg1=15,data_tseg2=4,data_sjw=4" },
    { BTR_KEY,     true, (TBitrateString)"f_clock=80000000,nom_brp=2,nom_tseg1=127,nom_tseg2=32,nom_sjw=32,data_bpr=2,data_tseg1=15,data_tseg2=4,data_sjw=4" },
    { BTR_KEY,     true, (TBitrateString)"f_clock=80000000,nom_brp=2,nom_tseg1=127,nom_tseg2=32,nom_sjw=32,data_brp=2,data-tseg1=127,data_tseg1=15,data_sjw=4" },
    { BTR_KEY,     true, (TBitrateString)"f_clock=80000000,nom_brp=2,nom_tseg1=127,nom_tseg2=32,nom_sjw=32,data_brp=2,data_tseg1=15,data_tseg3=32,data_sjw=4" },
    { BTR_KEY,     true, (TBitrateString)"f_clock=80000000,nom_brp=2,nom_tseg1=127,nom_tseg2=32,nom_sjw=32,data_brp=2,data_tseg1=15,data_tseg2=4,data_sjw:32" },
    { BTR_KEY,     true, (TBitrateString)"f_clock=80000000,nom_brp=2,nom_tseg1=127,nom_tseg2=32,nom_sjw=32,data_brp=2,data_tseg1=15,data_tseg2=4,data_sjw=4,data_key=42" },
    // wrong value (not a number, delimiter, exseeds its range)
    { BTR_VALUE,   true, (TBitrateString)"f_clock_mhz=eighty,nom_brp=2,nom_tseg1=127,nom_tseg2=32,nom_sjw=32,data_brp=2,data_tseg1=15,data_tseg2=4,data_sjw=4" },
    { BTR_VALUE,   true, (TBitrateString)"f_clock_mhz=80,nom_brp=2,nom_tseg1=127,nom_tseg2=32,nom_sjw=32,data_brp=0x2,data_tseg1=15,data_tseg2=4,data_sjw=4" },
    { BTR_VALUE,   true, (TBitrateString)"f_clock_mhz=80,nom_brp=2,nom_tseg1=127,nom_tseg2=32,nom_sjw=32,data_brp=2,data_tseg1=l5,data_tseg2=4,data_sjw=4" },
    { BTR_VALUE,   true, (TBitrateString)"f_clock_mhz=80,nom_brp=2,nom_tseg1=127,nom_tseg2=32,nom_sjw=32,data_brp=2,data_tseg1=15,data_tseg2=4h,data_sjw=4" },
    { BTR_CORRECT, true, (TBitrateString)"f_clock_mhz=80,nom_brp=2,nom_tseg1=127,nom_tseg2=32,nom_sjw=32,data_brp=2,data_tseg1=15,data_tseg2=4,data_sjw=4," },  // FIXME: bug in the scanner
    { BTR_VALUE,   true, (TBitrateString)"f_clock_mhz=80,nom_brp=2,nom_tseg1=127,nom_tseg2=32,nom_sjw=32,data_brp=2,data_tseg1=15,data_tseg2=4,data_sjw=4;" },
    { BTR_VALUE,   true, (TBitrateString)"f_clock_mhz=2147484,nom_brp=2,nom_tseg1=127,nom_tseg2=32,nom_sjw=32,data_brp=2,data_tseg1=15,data_tseg2=4,data_sjw=4" },
    { BTR_VALUE,   true, (TBitrateString)"f_clock_mhz=80,nom_brp=2,nom_tseg1=127,nom_tseg2=32,nom_sjw=32,data_brp=65536,data_tseg1=15,data_tseg2=4,data_sjw=4" },
    { BTR_VALUE,   true, (TBitrateString)"f_clock_mhz=80,nom_brp=2,nom_tseg1=127,nom_tseg2=32,nom_sjw=32,data_brp=2,data_tseg1=65536,data_tseg2=4,data_sjw=4" },
    { BTR_VALUE,   true, (TBitrateString)"f_clock_mhz=80,nom_brp=2,nom_tseg1=127,nom_tseg2=32,nom_sjw=32,data_brp=2,data_tseg1=15,data_tseg2=65536,data_sjw=4" },
    { BTR_VALUE,   true, (TBitrateString)"f_clock_mhz=80,nom_brp=2,nom_tseg1=127,nom_tseg2=32,nom_sjw=32,data_brp=2,data_tseg1=15,data_tseg2=4,data_sjw=65536" },
    { BTR_VALUE,   true, (TBitrateString)"f_clock=2147483648,nom_brp=2,nom_tseg1=127,nom_tseg2=32,nom_sjw=32,data_brp=2,data_tseg1=15,data_tseg2=4,data_sjw=4" },
    // duplicates (but valid)
    { BTR_DOUBLE,  true, (TBitrateString)"f_clock=80000000,nom_brp=2,nom_tseg1=127,nom_tseg2=32,nom_sjw=32,data_brp=2,data_tseg1=15,data_tseg2=4,data_sjw=4," \
                                         "f_clock=80000000,nom_brp=2,nom_tseg1=127,nom_tseg2=32,nom_sjw=32,data_brp=2,data_tseg1=15,data_tseg2=4,data_sjw=4" },
    { BTR_DOUBLE,  true, (TBitrateString)"f_clock=80000000,nom_brp=2,nom_tseg1=127,nom_tseg2=32,nom_sjw=32,data_brp=2,data_tseg1=15,data_tseg2=4,data_sjw=4,f_clock_mhz=80" },
    { BTR_DOUBLE,  true, (TBitrateString)"f_clock=80000000,nom_brp=2,nom_tseg1=127,nom_tseg2=32,nom_sjw=32,data_brp=2,data_tseg1=15,data_tseg2=4,data_sjw=4,data_brp=2" },
    { BTR_DOUBLE,  true, (TBitrateString)"f_clock=80000000,nom_brp=2,nom_tseg1=127,nom_tseg2=32,nom_sjw=32,data_brp=2,data_tseg1=15,data_tseg2=4,data_sjw=4,data_tseg1=15" },
    { BTR_DOUBLE,  true, (TBitrateString)"f_clock=80000000,nom_brp=2,nom_tseg1=127,nom_tseg2=32,nom_sjw=32,data_brp=2,data_tseg1=15,data_tseg2=4,data_sjw=4,data_tseg2=4" },
    { BTR_DOUBLE,  true, (TBitrateString)"f_clock=80000000,nom_brp=2,nom_tseg1=127,nom_tseg2=32,nom_sjw=32,data_brp=2,data_tseg1=15,data_tseg2=4,data_sjw=4,data_sjw=4" },
    // valid CAN bit-rate strings with blanks
    { BTR_CORRECT, true, (TBitrateString)" f_clock=80000000,nom_brp=2,nom_tseg1=127,nom_tseg2=32,nom_sjw=32,data_brp=2,data_tseg1=15,data_tseg2=4,data_sjw=4" },
    { BTR_CORRECT, true, (TBitrateString)"f_clock =80000000,nom_brp=2,nom_tseg1=127,nom_tseg2=32,nom_sjw=32,data_brp=2,data_tseg1=15,data_tseg2=4,data_sjw=4" },
    { BTR_CORRECT, true, (TBitrateString)"f_clock= 80000000,nom_brp=2,nom_tseg1=127,nom_tseg2=32,nom_sjw=32,data_brp=2,data_tseg1=15,data_tseg2=4,data_sjw=4" },
    { BTR_CORRECT, true, (TBitrateString)"f_clock=80000000 ,nom_brp=2,nom_tseg1=127,nom_tseg2=32,nom_sjw=32,data_brp=2,data_tseg1=15,data_tseg2=4,data_sjw=4" },
    { BTR_CORRECT, true, (TBitrateString)"f_clock=80000000,nom_brp=2,nom_tseg1=127,nom_tseg2=32,nom_sjw=32, data_brp=2,data_tseg1=15,data_tseg2=4,data_sjw=4" },
    { BTR_CORRECT, true, (TBitrateString)"f_clock=80000000,nom_brp=2,nom_tseg1=127,nom_tseg2=32,nom_sjw=32,data_brp =2,data_tseg1=15,data_tseg2=4,data_sjw=4" },
    { BTR_CORRECT, true, (TBitrateString)"f_clock=80000000,nom_brp=2,nom_tseg1=127,nom_tseg2=32,nom_sjw=32,data_brp=2 ,data_tseg1=15,data_tseg2=4,data_sjw=4" },
    { BTR_CORRECT, true, (TBitrateString)"f_clock=80000000,nom_brp=2,nom_tseg1=127,nom_tseg2=32,nom_sjw=32,data_brp=2, data_tseg1=15,data_tseg2=4,data_sjw=4" },
    { BTR_CORRECT, true, (TBitrateString)"f_clock=80000000,nom_brp=2,nom_tseg1=127,nom_tseg2=32,nom_sjw=32,data_brp=2,data_tseg1 =15,data_tseg2=4,data_sjw=4" },
    { BTR_CORRECT, true, (TBitrateString)"f_clock=80000000,nom_brp=2,nom_tseg1=127,nom_tseg2=32,nom_sjw=32,data_brp=2,data_tseg1= 15,data_tseg2=4,data_sjw=4" },
    { BTR_CORRECT, true, (TBitrateString)"f_clock=80000000,nom_brp=2,nom_tseg1=127,nom_tseg2=32,nom_sjw=32,data_brp=2,data_tseg1=15 ,data_tseg2=4,data_sjw=4" },
    { BTR_CORRECT, true, (TBitrateString)"f_clock=80000000,nom_brp=2,nom_tseg1=127,nom_tseg2=32,nom_sjw=32,data_brp=2,data_tseg1=15, data_tseg2=4,data_sjw=4" },
    { BTR_CORRECT, true, (TBitrateString)"f_clock=80000000,nom_brp=2,nom_tseg1=127,nom_tseg2=32,nom_sjw=32,data_brp=2,data_tseg1=15,data_tseg2 =4,data_sjw=4" },
    { BTR_CORRECT, true, (TBitrateString)"f_clock=80000000,nom_brp=2,nom_tseg1=127,nom_tseg2=32,nom_sjw=32,data_brp=2,data_tseg1=15,data_tseg2= 4,data_sjw=4" },
    { BTR_CORRECT, true, (TBitrateString)"f_clock=80000000,nom_brp=2,nom_tseg1=127,nom_tseg2=32,nom_sjw=32,data_brp=2,data_tseg1=15,data_tseg2=4 ,data_sjw=4" },
    { BTR_CORRECT, true, (TBitrateString)"f_clock=80000000,nom_brp=2,nom_tseg1=127,nom_tseg2=32,nom_sjw=32,data_brp=2,data_tseg1=15,data_tseg2=4, data_sjw=4" },
    { BTR_CORRECT, true, (TBitrateString)"f_clock=80000000,nom_brp=2,nom_tseg1=127,nom_tseg2=32,nom_sjw=32,data_brp=2,data_tseg1=15,data_tseg2=4,data_sjw =4" },
    { BTR_CORRECT, true, (TBitrateString)"f_clock=80000000,nom_brp=2,nom_tseg1=127,nom_tseg2=32,nom_sjw=32,data_brp=2,data_tseg1=15,data_tseg2=4,data_sjw= 4" },
    { BTR_CORRECT, true, (TBitrateString)"f_clock=80000000,nom_brp=2,nom_tseg1=127,nom_tseg2=32,nom_sjw=32,data_brp=2,data_tseg1=15,data_tseg2=4,data_sjw=4 " },
    { BTR_CORRECT, true, (TBitrateString)" f_clock = 80000000 , nom_brp = 2 , nom_tseg1 = 127 , nom_tseg2 = 32 , nom_sjw = 32 , data_brp = 2 , data_tseg1 = 15 , data_tseg2 = 4 , data_sjw = 4 " },
#endif
    // empty CAN bit-rate string (wrong value because frequency is set to 0 and this is less than 8MHz)
    { BTR_VALUE,   false, (TBitrateString)"" },
    // This is the end
    // Beautiful friend
    // This is the end
    // My only friend, the end
    { BTR_WRONG, false, nullptr }
};

CBitrates::CBitrates() {
    m_nIndex = -1;
}

TBitrateString CBitrates::GetFirstEntry(bool fValid) {
    m_nIndex = 0;
    while ((aBitrateString[m_nIndex].m_szValue != nullptr) &&
           (fValid != ((aBitrateString[m_nIndex].m_eValidity == BTR_CORRECT) ||
                       (aBitrateString[m_nIndex].m_eValidity == BTR_RANGE))))
        m_nIndex += 1;
    // note: last entry returns a NULL pointer
    return (TBitrateString)aBitrateString[m_nIndex].m_szValue;
}

TBitrateString CBitrates::GetNextEntry(bool fValid)  {
    if (aBitrateString[m_nIndex].m_szValue != nullptr)
        m_nIndex += 1;
    while ((aBitrateString[m_nIndex].m_szValue != nullptr) &&
           (fValid != ((aBitrateString[m_nIndex].m_eValidity == BTR_CORRECT) ||
                       (aBitrateString[m_nIndex].m_eValidity == BTR_RANGE))))
        m_nIndex += 1;
    // note: last entry returns a NULL pointer
    return (TBitrateString)aBitrateString[m_nIndex].m_szValue;
}

bool CBitrates::IsInRange() {
    return ((aBitrateString[m_nIndex].m_szValue != nullptr) &&
            (aBitrateString[m_nIndex].m_eValidity == BTR_CORRECT)) ? true : false;
}

bool CBitrates::IsCanFdWithBrse() {
    return ((aBitrateString[m_nIndex].m_szValue != nullptr) &&
            (aBitrateString[m_nIndex].m_fCanFdWithBrse)) ? true : false;
}

// $Id: Bitrates.cpp 1094 2023-07-23 17:20:14Z makemake $  Copyright (c) UV Software, Berlin //
