/*  SPDX-License-Identifier: BSD-2-Clause OR GPL-3.0-or-later */
/*
 *  CAN Interface API, Version 3 (Definitions and Options)
 *
 *  Copyright (c) 2004-2024 Uwe Vogt, UV Software, Berlin (info@uv-software.com)
 *  All rights reserved.
 *
 *  This file is part of CAN API V3.
 *
 *  CAN API V3 is dual-licensed under the BSD 2-Clause "Simplified" License
 *  and under the GNU General Public License v3.0 (or any later version).
 *  You can choose between one of them if you use this file.
 *
 *  BSD 2-Clause "Simplified" License:
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are met:
 *  1. Redistributions of source code must retain the above copyright notice, this
 *     list of conditions and the following disclaimer.
 *  2. Redistributions in binary form must reproduce the above copyright notice,
 *     this list of conditions and the following disclaimer in the documentation
 *     and/or other materials provided with the distribution.
 *
 *  CAN API V3 IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 *  DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 *  FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 *  DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 *  SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 *  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 *  OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *  OF CAN API V3, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *  GNU General Public License v3.0 or later:
 *  CAN API V3 is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  CAN API V3 is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with CAN API V3.  If not, see <https://www.gnu.org/licenses/>.
 */
/** @file        CANBTR_Defines.h
 *
 *  @brief       CAN API V3 for generic CAN Interfaces - Definitions and Options
 *
 *  @author      $Author: eris $
 *
 *  @version     $Rev: 1270 $
 *
 *  @addtogroup  can_btr
 *  @{
 */
#ifndef CANBTR_DEFINES_H_INCLUDED
#define CANBTR_DEFINES_H_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

/*  -----------  includes  ------------------------------------------------
 */


/*  -----------  options  ------------------------------------------------
 */


/*  -----------  defines  ------------------------------------------------
 */

/** @name  SJA1000 CAN Clock
 *  @brief Frequency of the SJA1000 CAN controller.
 *  @{ */
#define SJA1000_CAN_CLOCK  8000000  /**< 8 MHz */
/** @} */

/** @name  SJA1000 Bit-timing Values
 *  @brief Predefined BTR0BTR1 register values (complies to CiA CANopen DS-301 spec.).
 *
 *  @note  800 kbps is not supported by all CAN controller  <br>
 *         100 kbps is an obsolete CANopen bit-rate  <br/>
 *         5 kbps is not defined in CANopen
 *  @{ */
#define SJA1000_1M    0x0014U  /**< 1000 kbps (SP=75.0%, SJW=1) */
#define SJA1000_800K  0x0016U  /**<  800 kbps (SP=80.0%, SJW=1) */
#define SJA1000_500K  0x001CU  /**<  500 kbps (SP=87.5%, SJW=1) */
#define SJA1000_250K  0x011CU  /**<  250 kbps (SP=87.5%, SJW=1) */
#define SJA1000_125K  0x031CU  /**<  125 kbps (SP=87.5%, SJW=1) */
#define SJA1000_100K  0x441CU  /**<  100 kbps (SP=87.5%, SJW=2) */
#define SJA1000_50K   0x491CU  /**<   50 kbps (SP=87.5%, SJW=2) */
#define SJA1000_20K   0x581CU  /**<   20 kbps (SP=87.5%, SJW=2) */
#define SJA1000_10K   0x711CU  /**<   10 kbps (SP=87.5%, SJW=2) */
#define SJA1000_5K    0x7F7FU  /**<    5 kbps (SP=68.0%, SJW=2) */
/** @} */

/** @name  SJA1000  Bit-rate Settings
 *  @brief Predefined BTR0BTR1 register values as CAN API V3 bit-rate settings.
 *  @{ */
#define SJA1000_BR_1M(x)    do {x.btr.frequency=SJA1000_CAN_CLOCK;x.btr.nominal.brp=1; x.btr.nominal.tseg1=5; x.btr.nominal.tseg2=2;x.btr.nominal.sjw=1;x.btr.nominal.sam=0;} while(0)
#define SJA1000_BR_800K(x)  do {x.btr.frequency=SJA1000_CAN_CLOCK;x.btr.nominal.brp=1; x.btr.nominal.tseg1=7; x.btr.nominal.tseg2=2;x.btr.nominal.sjw=1;x.btr.nominal.sam=0;} while(0)
#define SJA1000_BR_500K(x)  do {x.btr.frequency=SJA1000_CAN_CLOCK;x.btr.nominal.brp=1; x.btr.nominal.tseg1=13;x.btr.nominal.tseg2=2;x.btr.nominal.sjw=1;x.btr.nominal.sam=0;} while(0)
#define SJA1000_BR_250K(x)  do {x.btr.frequency=SJA1000_CAN_CLOCK;x.btr.nominal.brp=2; x.btr.nominal.tseg1=13;x.btr.nominal.tseg2=2;x.btr.nominal.sjw=1;x.btr.nominal.sam=0;} while(0)
#define SJA1000_BR_125K(x)  do {x.btr.frequency=SJA1000_CAN_CLOCK;x.btr.nominal.brp=4; x.btr.nominal.tseg1=13;x.btr.nominal.tseg2=2;x.btr.nominal.sjw=1;x.btr.nominal.sam=0;} while(0)
#define SJA1000_BR_100K(x)  do {x.btr.frequency=SJA1000_CAN_CLOCK;x.btr.nominal.brp=5; x.btr.nominal.tseg1=13;x.btr.nominal.tseg2=2;x.btr.nominal.sjw=2;x.btr.nominal.sam=0;} while(0)
#define SJA1000_BR_50K(x)   do {x.btr.frequency=SJA1000_CAN_CLOCK;x.btr.nominal.brp=10;x.btr.nominal.tseg1=13;x.btr.nominal.tseg2=2;x.btr.nominal.sjw=2;x.btr.nominal.sam=0;} while(0)
#define SJA1000_BR_20K(x)   do {x.btr.frequency=SJA1000_CAN_CLOCK;x.btr.nominal.brp=25;x.btr.nominal.tseg1=13;x.btr.nominal.tseg2=2;x.btr.nominal.sjw=2;x.btr.nominal.sam=0;} while(0)
#define SJA1000_BR_10K(x)   do {x.btr.frequency=SJA1000_CAN_CLOCK;x.btr.nominal.brp=50;x.btr.nominal.tseg1=13;x.btr.nominal.tseg2=2;x.btr.nominal.sjw=2;x.btr.nominal.sam=0;} while(0)
#define SJA1000_BR_5K(x)    do {x.btr.frequency=SJA1000_CAN_CLOCK;x.btr.nominal.brp=64;x.btr.nominal.tseg1=16;x.btr.nominal.tseg2=8;x.btr.nominal.sjw=2;x.btr.nominal.sam=0;} while(0)
/** @} */

/** @name  CAN API CAN Clock
 *  @brief Default value for CAN 2.0 and CAN FD operation modes.
 *  @{ */
#define DEFAULT_CAN_CLOCK  SJA1000_CAN_CLOCK  /**< SJA1000 CAN Clock (8 MHz) */
#define DEFAULT_CAN_FD_CLOCK        80000000  /**< CAN FD Clock (80 MHz) */
/** @} */

/** @name  CAN 2.0 Bit-rate Settings
 *  @brief Default values according to CANopen spec. (CAN API requirement).
 *  @{ */
#define DEFAULT_CAN_BR_1M(x)    SJA1000_BR_1M(x)
#define DEFAULT_CAN_BR_800K(x)  SJA1000_BR_800K(x)
#define DEFAULT_CAN_BR_500K(x)  SJA1000_BR_500K(x)
#define DEFAULT_CAN_BR_250K(x)  SJA1000_BR_250K(x)
#define DEFAULT_CAN_BR_125K(x)  SJA1000_BR_125K(x)
#define DEFAULT_CAN_BR_100K(x)  SJA1000_BR_100K(x)
#define DEFAULT_CAN_BR_50K(x)   SJA1000_BR_50K(x)
#define DEFAULT_CAN_BR_20K(x)   SJA1000_BR_20K(x)
#define DEFAULT_CAN_BR_10K(x)   SJA1000_BR_10K(x)
#define DEFAULT_CAN_BR_5K(x)    SJA1000_BR_5K(x)
/** @} */

#if (OPTION_CAN_2_0_ONLY == 0)
/** @name  CAN FD Bit-rate Settings w/o Bit-rate Switching
 *  @brief Default values for long frames only (0 to 64 bytes).
 *
 *  @note  Actual bit-rate settings for specific controllers may vary.
 *         The settings made here are for Peak CAN FD interfaces.
 *  @{ */
#define DEFAULT_CAN_FD_BR_1M(x)      do {x.btr.frequency=DEFAULT_CAN_FD_CLOCK;x.btr.nominal.brp=2;x.btr.nominal.tseg1=31; x.btr.nominal.tseg2=8; x.btr.nominal.sjw=8; } while(0)
#define DEFAULT_CAN_FD_BR_500K(x)    do {x.btr.frequency=DEFAULT_CAN_FD_CLOCK;x.btr.nominal.brp=2;x.btr.nominal.tseg1=63; x.btr.nominal.tseg2=16;x.btr.nominal.sjw=16;} while(0)
#define DEFAULT_CAN_FD_BR_250K(x)    do {x.btr.frequency=DEFAULT_CAN_FD_CLOCK;x.btr.nominal.brp=2;x.btr.nominal.tseg1=127;x.btr.nominal.tseg2=32;x.btr.nominal.sjw=32;} while(0)
#define DEFAULT_CAN_FD_BR_125K(x)    do {x.btr.frequency=DEFAULT_CAN_FD_CLOCK;x.btr.nominal.brp=2;x.btr.nominal.tseg1=255;x.btr.nominal.tseg2=64;x.btr.nominal.sjw=64;} while(0)
/** @} */

/** @name  CAN FD Bit-rate Settings with Bit-rate Switching
 *  @brief Default values for long and fast frames only (up to 8 Mbps).
 *
 *  @note  Actual bit-rate settings for specific controllers may vary.
 *         The settings made here are for Peak CAN FD interfaces.
 *  @{ */
#define DEFAULT_CAN_FD_BR_1M8M(x)    do {x.btr.frequency=DEFAULT_CAN_FD_CLOCK;x.btr.nominal.brp=2;x.btr.nominal.tseg1=31; x.btr.nominal.tseg2=8; x.btr.nominal.sjw=8;  x.btr.data.brp=2; x.btr.data.tseg1=3;  x.btr.data.tseg2=1; x.btr.data.sjw=1; } while(0)
#define DEFAULT_CAN_FD_BR_500K4M(x)  do {x.btr.frequency=DEFAULT_CAN_FD_CLOCK;x.btr.nominal.brp=2;x.btr.nominal.tseg1=63; x.btr.nominal.tseg2=16;x.btr.nominal.sjw=16; x.btr.data.brp=2; x.btr.data.tseg1=7;  x.btr.data.tseg2=2; x.btr.data.sjw=2; } while(0)
#define DEFAULT_CAN_FD_BR_250K2M(x)  do {x.btr.frequency=DEFAULT_CAN_FD_CLOCK;x.btr.nominal.brp=2;x.btr.nominal.tseg1=127;x.btr.nominal.tseg2=32;x.btr.nominal.sjw=32; x.btr.data.brp=2; x.btr.data.tseg1=15; x.btr.data.tseg2=4; x.btr.data.sjw=4; } while(0)
#define DEFAULT_CAN_FD_BR_125K1M(x)  do {x.btr.frequency=DEFAULT_CAN_FD_CLOCK;x.btr.nominal.brp=2;x.btr.nominal.tseg1=255;x.btr.nominal.tseg2=64;x.btr.nominal.sjw=64; x.btr.data.brp=2; x.btr.data.tseg1=31; x.btr.data.tseg2=8; x.btr.data.sjw=8; } while(0)
/** @} */
#endif
#ifdef __cplusplus
}
#endif
#endif /* CANBTR_DEFINES_H_INCLUDED */
/** @}
 */
/*  ----------------------------------------------------------------------
 *  Uwe Vogt,  UV Software,  Chausseestrasse 33 A,  10115 Berlin,  Germany
 *  Tel.: +49-30-46799872,  Fax: +49-30-46799873,  Mobile: +49-170-3801903
 *  E-Mail: uwe.vogt@uv-software.de,  Homepage: http://www.uv-software.de/
 */
