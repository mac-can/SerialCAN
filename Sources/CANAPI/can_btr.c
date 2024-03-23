/*  SPDX-License-Identifier: BSD-2-Clause OR GPL-3.0-or-later */
/*
 *  CAN Interface API, Version 3 (Bit-rate Conversion)
 *
 *  Copyright (c) 2017-2023 Uwe Vogt, UV Software, Berlin (info@uv-software.com)
 *  All rights reserved.
 *
 *  This file is part of CAN API V3.
 *
 *  CAN API V3 is dual-licensed under the BSD 2-Clause "Simplified" License and
 *  under the GNU General Public License v3.0 (or any later version).
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
 *  along with CAN API V3.  If not, see <http://www.gnu.org/licenses/>.
 */
/** @file        can_btr.c
 *
 *  @brief       CAN Bit-rate conversion
 *
 *  @note        SJA1000 Bit-timing register BTR0BTR1:
 *
 *               +-f-+-e-+-d-+---+---+---+---+-8-+-7-+-6-+---+-4-+-3-+---+---+-0-+<br>
 *               |  SJW  |          BRP          |SAM|   TSEG2   |     TSEG1     |<br>
 *               +-7-+-6-+-5-+---+---+---+---+-0-+-7-+-6-+---+-4-+-3-+---+---+-0-+<br>
 *
 *  @author      $Author: makemake $
 *
 *  @version     $Rev: 1186 $
 *
 *  @addtogroup  can_btr
 *  @{
 */


/*  -----------  includes  -----------------------------------------------
 */

#ifdef _MSC_VER
//no Microsoft extensions please!
#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS 1
#endif
#endif
#include "can_btr.h"

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>
#endif
#include <math.h>

/*  -----------  defines  ------------------------------------------------
 */

#if (0)
#define BTR_FREQUENCY_MIN       (int32_t)BTR_FREQ_SJA1000
#else
#define BTR_FREQUENCY_MIN       (int32_t)1
#endif
#define BTR_STRING_MAX          1000

/*  - - - - - -  helper macros   - - - - - - - - - - - - - - - - - - - - -
 */
#define BTR_SJW(btr0btr1)       (((uint16_t)(btr0btr1) & 0xC000u) >> 14)
#define BTR_BRP(btr0btr1)       (((uint16_t)(btr0btr1) & 0x3F00u) >> 8)
#define BTR_SAM(btr0btr1)       (((uint16_t)(btr0btr1) & 0x0080u) >> 7)
#define BTR_TSEG2(btr0btr1)     (((uint16_t)(btr0btr1) & 0x0070u) >> 4)
#define BTR_TSEG1(btr0btr1)     (((uint16_t)(btr0btr1) & 0x000Fu) >> 0)
#define BTR_BTR0BTR1(sjw,brp,sam,tseg2,tseg1) \
                                ((((uint16_t)(sjw) & 0x0003) << 14)  | \
                                 (((uint16_t)(brp) & 0x003F) << 8)   | \
                                 (((uint16_t)(sam) & 0x0001) << 7)   | \
                                 (((uint16_t)(tseg2) & 0x0007) << 4) | \
                                 (((uint16_t)(tseg1) & 0x000F) << 0))
#ifdef _MSC_VER
//not #if defined(_WIN32) || defined(_WIN64) because we have strncasecmp in mingw
#define strncasecmp _strnicmp
#define strcasecmp _stricmp
#endif

/*  -----------  types  --------------------------------------------------
 */


/*  -----------  prototypes  ---------------------------------------------
 */

static int print_bitrate(const btr_bitrate_t *bitrate, bool data, bool sam, btr_string_t string, size_t maxbyte);
static int scan_bitrate(const btr_string_t string, btr_bitrate_t *bitrate, bool *data, bool *sam);

static char *scan_key(char *str);
static char *scan_value(char *str);
static char *skip_blanks(char *str);


/*  -----------  variables  ----------------------------------------------
 */

static const btr_sja1000_t sja1000_btr0btr1[BTR_SJA1000_ENTRIES] = {
    SJA1000_1M,    // 1000 kbps (SP=75.0%, SJW=1)
    SJA1000_800K,  //  800 kbps (SP=80.0%, SJW=1)
    SJA1000_500K,  //  500 kbps (SP=87.5%, SJW=1)
    SJA1000_250K,  //  250 kbps (SP=87.5%, SJW=1)
    SJA1000_125K,  //  125 kbps (SP=87.5%, SJW=1)
    SJA1000_100K,  //  100 kbps (SP=87.5%, SJW=2)
    SJA1000_50K,   //   50 kbps (SP=87.5%, SJW=2)
    SJA1000_20K,   //   20 kbps (SP=87.5%, SJW=2)
    SJA1000_10K,   //   10 kbps (SP=87.5%, SJW=2)
    SJA1000_5K     //    5 kbps (SP=68.0%, SJW=2)
};

/*  -----------  functions  ----------------------------------------------
 */

int btr_check_bitrate(const btr_bitrate_t *bitrate, bool fdoe, bool brse) {
    if (!bitrate)                       // check for null-pointer
        return BTRERR_NULLPTR;

    if (bitrate->index <= 0) {          // CAN 2.0 bit-rate index
        if (-BTR_SJA1000_ENTRIES >= bitrate->index)
            return BTRERR_BAUDRATE;
    }
    else {                              // CAN bit-rate settings
        if ((bitrate->btr.nominal.brp < BTR_NOMINAL_BRP_MIN) || (BTR_NOMINAL_BRP_MAX < bitrate->btr.nominal.brp))
            return BTRERR_BAUDRATE;
        if ((bitrate->btr.nominal.tseg1 < BTR_NOMINAL_TSEG1_MIN) || (BTR_NOMINAL_TSEG1_MAX < bitrate->btr.nominal.tseg1))
            return BTRERR_BAUDRATE;
        if ((bitrate->btr.nominal.tseg2 < BTR_NOMINAL_TSEG2_MIN) || (BTR_NOMINAL_TSEG2_MAX < bitrate->btr.nominal.tseg2))
            return BTRERR_BAUDRATE;
        if ((bitrate->btr.nominal.sjw < BTR_NOMINAL_SJW_MIN) || (BTR_NOMINAL_SJW_MAX < bitrate->btr.nominal.sjw))
            return BTRERR_BAUDRATE;
#if (OPTION_CAN_2_0_ONLY != OPTION_DISABLED)
        if ((bitrate->btr.nominal.sam != BTR_NOMINAL_SAM_SINGLE) && (BTR_NOMINAL_SAM_TRIPLE != bitrate->btr.nominal.sam))
            return BTRERR_BAUDRATE;
#else
        if (fdoe) {                     //   CAN FD:
            if (brse) {                 //     bit-rate switching enabled
                if ((bitrate->btr.data.brp < BTR_DATA_BRP_MIN) || (BTR_DATA_BRP_MAX < bitrate->btr.data.brp))
                    return BTRERR_BAUDRATE;
                if ((bitrate->btr.data.tseg1 < BTR_DATA_TSEG1_MIN) || (BTR_DATA_TSEG1_MAX < bitrate->btr.data.tseg1))
                    return BTRERR_BAUDRATE;
                if ((bitrate->btr.data.tseg2 < BTR_DATA_TSEG2_MIN) || (BTR_DATA_TSEG2_MAX < bitrate->btr.data.tseg2))
                    return BTRERR_BAUDRATE;
                if ((bitrate->btr.data.sjw < BTR_DATA_SJW_MIN) || (BTR_DATA_SJW_MAX < bitrate->btr.data.sjw))
                    return BTRERR_BAUDRATE;
            } else {                    //     bit-rate switching disabled
                // note: data phase fields will be ignored
            }
        }
        else {                          //   CAN 2.0: check SAM
            if ((bitrate->btr.nominal.sam != BTR_NOMINAL_SAM_SINGLE) && (BTR_NOMINAL_SAM_TRIPLE != bitrate->btr.nominal.sam))
                return BTRERR_BAUDRATE;
        }
#endif
    }
    return BTRERR_NOERROR;
}

int btr_compare_bitrates(const btr_bitrate_t *bitrate1, const btr_bitrate_t *bitrate2, bool fdoe, bool brse, bool cmp_sp) {
    btr_bitrate_t temporary1;           // bit-rate 1 settings
    uint64_t nom_speed1, nom_sp1;
#if (OPTION_CAN_2_0_ONLY == OPTION_DISABLED)
    uint64_t data_speed1, data_sp1;
#endif
    btr_bitrate_t temporary2;           // bit-rate 2 settings
    uint64_t nom_speed2, nom_sp2;
#if (OPTION_CAN_2_0_ONLY == OPTION_DISABLED)
    uint64_t data_speed2, data_sp2;
#endif
    int rc1 = 0, rc2 = 0;               // return values

    if (!bitrate1 || !bitrate2)         // check for null-pointer
        return (!bitrate1 && !bitrate2) ? 0 : !bitrate1 ? -9 : +9;

    /* note: all fields have to be checked for their limits beforehand. But don't care! */

    if (bitrate1->index <= 0) {          // CAN 2.0 bit-rate 1 index
        rc1 = btr_index2bitrate(bitrate1->index, &temporary1);
    }
    else {                              // CAN bit-rate 1 settings
       memcpy(&temporary1, bitrate1, sizeof(btr_bitrate_t));
    }
    if (bitrate2->index <= 0) {          // CAN 2.0 bit-rate 2 index
        rc2 = btr_index2bitrate(bitrate2->index, &temporary2);
    }
    else {                              // CAN bit-rate 2 settings
       memcpy(&temporary2, bitrate2, sizeof(btr_bitrate_t));
    }
    if ((rc1 != 0) || (rc2 != 0))       // check invalid indexes
        return ((rc1 != 0) && (rc2 != 0)) ? 0 : (rc1 != 0) ? -8 : +8;

    /* nominal bit-rates (CAN 2.0 and CAN FD):
     *
     * (1) speed = freq / (brp * (1 + tseg1 + tseg2))
     *
     * (2) sp = (1 + tseg1) / (1 + tseg1 + tseg2)
     */
    if (temporary1.btr.nominal.brp)
        nom_speed1 = ((((uint64_t)temporary1.btr.frequency * (uint64_t)10)
                     / ((uint64_t)temporary1.btr.nominal.brp * ((uint64_t)1 + (uint64_t)temporary1.btr.nominal.tseg1 + (uint64_t)temporary1.btr.nominal.tseg2))) + (uint64_t)5) / (uint64_t)10;
    else  // devide-by-zero
        nom_speed1 = 0U;
    nom_sp1 = (((((uint64_t)1 + (uint64_t)temporary1.btr.nominal.tseg1) * (uint64_t)1000)
               / ((uint64_t)1 + (uint64_t)temporary1.btr.nominal.tseg1 + (uint64_t)temporary1.btr.nominal.tseg2)) + (uint64_t)5) / (uint64_t)10;
    if (temporary2.btr.nominal.brp)
        nom_speed2 = ((((uint64_t)temporary2.btr.frequency * (uint64_t)10)
                     / ((uint64_t)temporary2.btr.nominal.brp * ((uint64_t)1 + (uint64_t)temporary2.btr.nominal.tseg1 + (uint64_t)temporary2.btr.nominal.tseg2))) + (uint64_t)5) / (uint64_t)10;
    else  // devide-by-zero
        nom_speed2 = 0U;
    nom_sp2 = (((((uint64_t)1 + (uint64_t)temporary2.btr.nominal.tseg1) * (uint64_t)1000)
               / ((uint64_t)1 + (uint64_t)temporary2.btr.nominal.tseg1 + (uint64_t)temporary2.btr.nominal.tseg2)) + (uint64_t)5) / (uint64_t)10;
    /* compare nominal bit-rates */
    if (nom_speed1 != nom_speed2)
        return (nom_speed1 < nom_speed2) ? -1 : +1;
    if (cmp_sp && (nom_sp1 != nom_sp2))
        return (nom_sp1 < nom_sp2) ? -3 : +3;
#if (OPTION_CAN_2_0_ONLY == OPTION_DISABLED)
    if (fdoe && brse) {
        /* data phase bit-rate (CAN FD only):
         *
         * (1) speed = freq / (brp * (1 + tseg1 + tseg2))
         *
         * (2) sp = (1 + tseg1) / (1 + tseg1 + tseg2)
         */
        if (temporary1.btr.data.brp)
            data_speed1 = ((((uint64_t)temporary1.btr.frequency * (uint64_t)10)
                          / ((uint64_t)temporary1.btr.data.brp * ((uint64_t)1 + (uint64_t)temporary1.btr.data.tseg1 + (uint64_t)temporary1.btr.data.tseg2))) + (uint64_t)5) / (uint64_t)10;
        else  // devide-by-zero
            data_speed1 = 0U;
        data_sp1 = (((((uint64_t)1 + (uint64_t)temporary1.btr.data.tseg1) * (uint64_t)1000)
                     / ((uint64_t)1 + (uint64_t)temporary1.btr.data.tseg1 + (uint64_t)temporary1.btr.data.tseg2)) + (uint64_t)5) / (uint64_t)10;
        if (temporary2.btr.data.brp)
            data_speed2 = ((((uint64_t)temporary2.btr.frequency * (uint64_t)10)
                          / ((uint64_t)temporary2.btr.data.brp * ((uint64_t)1 + (uint64_t)temporary2.btr.data.tseg1 + (uint64_t)temporary2.btr.data.tseg2))) + (uint64_t)5) / (uint64_t)10;
        else  // devide-by-zero
            data_speed2 = 0U;
        data_sp2 = (((((uint64_t)1 + (uint64_t)temporary2.btr.data.tseg1) * (uint64_t)1000)
                    / ((uint64_t)1 + (uint64_t)temporary2.btr.data.tseg1 + (uint64_t)temporary2.btr.data.tseg2)) + (uint64_t)5) / (uint64_t)10;
        /* compare data phase bit-rates */
        if (data_speed1 != data_speed2)
            return (data_speed1 < data_speed2) ? -2 : +2;
        if (cmp_sp && (data_sp1 != data_sp2))
            return (data_sp1 < data_sp2) ? -4 : +4;
    }
#endif
    /* if the bit-rates are equal return 1*/
    return 0;
}

int btr_bitrate2speed(const btr_bitrate_t *bitrate, btr_speed_t *speed) {
    btr_bitrate_t temporary;            // bit-rate settings
    int rc;                             // return value

    if (!bitrate || !speed)             // check for null-pointer
        return BTRERR_NULLPTR;

    /* note: all fields have to be checked for their limits beforehand. But don't care! */

    if (bitrate->index <= 0) {          // CAN 2.0 bit-rate index
        if ((rc = btr_index2bitrate(bitrate->index, &temporary)) != BTRERR_NOERROR)
            return rc;
    }
    else {                              // CAN bit-rate settings
       memcpy(&temporary, bitrate, sizeof(btr_bitrate_t));
    }
    /* nominal bit-rate:
     *
     * (1) speed = freq / (brp * (1 + tseg1 + tseg2))
     *
     * (2) sp = (1 + tseg1) / (1 + tseg1 + tseg2)
     */
    if (temporary.btr.nominal.brp)
        speed->nominal.speed = ((float)temporary.btr.frequency)
                             / ((float)temporary.btr.nominal.brp * (1.0f + (float)temporary.btr.nominal.tseg1 + (float)temporary.btr.nominal.tseg2));
    else  // devide-by-zero
        speed->nominal.speed = INFINITY;
    speed->nominal.samplepoint = (1.0f + (float)temporary.btr.nominal.tseg1)
                               / (1.0f + (float)temporary.btr.nominal.tseg1 + (float)temporary.btr.nominal.tseg2);
#if (OPTION_CAN_2_0_ONLY == OPTION_DISABLED)
    /* data phase bit-rate (CAN FD only):
     *
     * (1) speed = freq / (brp * (1 + tseg1 + tseg2))
     *
     * (2) sp = (1 + tseg1) / (1 + tseg1 + tseg2)
     */
    if (temporary.btr.data.brp)
        speed->data.speed = ((float)temporary.btr.frequency)
                          / ((float)temporary.btr.data.brp * (1.0f + (float)temporary.btr.data.tseg1 + (float)temporary.btr.data.tseg2));
    else  // devide-by-zero
        speed->data.speed = INFINITY;
    speed->data.samplepoint = (1.0f + (float)temporary.btr.data.tseg1)
                            / (1.0f + (float)temporary.btr.data.tseg1 + (float)temporary.btr.data.tseg2);
#endif
    return BTRERR_NOERROR;
}

int btr_index2bitrate(const btr_index_t index, btr_bitrate_t *bitrate) {
    btr_sja1000_t btr0btr1 = 0x0000u;   // SJA1000 registers
    int rc = BTRERR_FATAL;              // return value

    if (!bitrate)                       // check for null-pointer
        return BTRERR_NULLPTR;

    /* first convert the index into SJA1000 BTR0BTR1 register */
    if ((rc = btr_index2sja1000(index, &btr0btr1)) != BTRERR_NOERROR)
        return rc;
    /* then convert the SJA1000 BTR0BTR1 register into bit-rate settings */
    return btr_sja10002bitrate(btr0btr1, bitrate);
}

int btr_bitrate2index(const btr_bitrate_t *bitrate, btr_index_t *index) {
    btr_sja1000_t btr0btr1;             // SJA1000 register
    int rc = BTRERR_FATAL;              // return value
    int i;

    if (!bitrate || !index)             // check for null-pointer
        return BTRERR_NULLPTR;

    if (bitrate->index <= 0) {          // index already given
        if (bitrate->index <= -BTR_SJA1000_ENTRIES)
            return BTRERR_BAUDRATE;
        /* index is valid */
        *index = bitrate->index;
        return BTRERR_NOERROR;
    }
    /* first convert bit-rate into SJA1000 register BTR0BTR1 */
    if ((rc = btr_bitrate2sja1000(bitrate, &btr0btr1)) != BTRERR_NOERROR)
        return rc;
    /* then look in the table of predefined bit-timing indexes */
    for (i = 0; i < BTR_SJA1000_ENTRIES; i++) {
        if (btr0btr1 == sja1000_btr0btr1[i]) {
            *index = (btr_index_t)(i * (-1));
            return BTRERR_NOERROR;
        }
    }
    /* bad luck, nothing found:( */
    return BTRERR_BAUDRATE;
}

int btr_string2bitrate(const btr_string_t string, btr_bitrate_t *bitrate, bool *data, bool *sam) {
    if (!bitrate || !string)            // check for null-pointer
        return BTRERR_NULLPTR;
    if (!data || !sam)                  // don't forget the flags
        return BTRERR_NULLPTR;

    /* note: 'data' is set to true if any of the data phase keys are given
     *       'sam' is set to true it the number of samples key is given */
    return scan_bitrate(string, bitrate, data, sam) ? BTRERR_ILLPARA : BTRERR_NOERROR;
}

int btr_bitrate2string(const btr_bitrate_t *bitrate, bool data, bool sam, btr_string_t string, size_t maxbyte) {
    btr_bitrate_t temporary;            // bit rate settings
    int rc;                             // return value

    if (!bitrate || !string)            // check for null-pointer
        return BTRERR_NULLPTR;
    if (!maxbyte)                       // at least one byte
        return BTRERR_ILLPARA;
    string[0] = '\0';                   // clear string buffer

    if (bitrate->index <= 0) {          // CAN 2.0 bit-rate index
        if ((rc = btr_index2bitrate(bitrate->index, &temporary)) != BTRERR_NOERROR)
            return rc;
    }
    else {                              // CAN FD bit-rate settings
        memcpy(&temporary, bitrate, sizeof(btr_bitrate_t));
    }
    /* Give it to me, honey! */
    return print_bitrate(&temporary, data, sam, string, maxbyte);
}

int btr_sja10002bitrate(const btr_sja1000_t btr0btr1, btr_bitrate_t *bitrate) {
    if (!bitrate)                       // check for null-pointer
        return BTRERR_NULLPTR;

    memset(bitrate, 0, sizeof(btr_bitrate_t));

    /* convert SJA1000 register BTR0BTR1 (don't forget the +1) */
    bitrate->btr.frequency = BTR_FREQ_SJA1000;
    bitrate->btr.nominal.sjw = BTR_SJW(btr0btr1) + 1u;
    bitrate->btr.nominal.brp = BTR_BRP(btr0btr1) + 1u;
    bitrate->btr.nominal.sam = BTR_SAM(btr0btr1); // 0 = sample once, 1 = three times!
    bitrate->btr.nominal.tseg2 = BTR_TSEG2(btr0btr1) + 1u;
    bitrate->btr.nominal.tseg1 = BTR_TSEG1(btr0btr1) + 1u;
#if (OPTION_CAN_2_0_ONLY == OPTION_DISABLED)
    bitrate->btr.data.brp = 0U;
    bitrate->btr.data.tseg1 = 0U;
    bitrate->btr.data.tseg2 = 0U;
    bitrate->btr.data.sjw = 0U;
#endif
    return BTRERR_NOERROR;
}

int btr_bitrate2sja1000(const btr_bitrate_t *bitrate, btr_sja1000_t *btr0btr1) {
    if (!bitrate || !btr0btr1)          // check for null-pointer
        return BTRERR_NULLPTR;

    /* check for SJA1000 CAN clock and BTR0BTR1 register field values */
    if (bitrate->btr.frequency != BTR_FREQ_SJA1000)
        return BTRERR_BAUDRATE;
    if ((bitrate->btr.nominal.sjw < BTR_SJA1000_SJW_MIN) || (BTR_SJA1000_SJW_MAX < bitrate->btr.nominal.sjw))
        return BTRERR_BAUDRATE;
    if ((bitrate->btr.nominal.brp < BTR_SJA1000_BRP_MIN) || (BTR_SJA1000_BRP_MAX < bitrate->btr.nominal.brp))
        return BTRERR_BAUDRATE;
    if ((bitrate->btr.nominal.sam != BTR_SJA1000_SAM_SINGLE) && (BTR_SJA1000_SAM_TRIPLE != bitrate->btr.nominal.sam))
        return BTRERR_BAUDRATE;
    if ((bitrate->btr.nominal.tseg2 < BTR_SJA1000_TSEG2_MIN) || (BTR_SJA1000_TSEG2_MAX < bitrate->btr.nominal.tseg2))
        return BTRERR_BAUDRATE;
    if ((bitrate->btr.nominal.tseg1 < BTR_SJA1000_TSEG1_MIN) || (BTR_SJA1000_TSEG1_MAX < bitrate->btr.nominal.tseg1))
        return BTRERR_BAUDRATE;
    /* make SJA1000 register BTR0BTR1 (don't forget the -1) */
    *btr0btr1 = BTR_BTR0BTR1(bitrate->btr.nominal.sjw - 1u,
                             bitrate->btr.nominal.brp - 1u,
                             bitrate->btr.nominal.sam, // 0 = sample once, 1 = three times!
                             bitrate->btr.nominal.tseg2 - 1u,
                             bitrate->btr.nominal.tseg1 - 1u);

    return BTRERR_NOERROR;
}

int btr_index2sja1000(const btr_index_t index, btr_sja1000_t *btr0btr1)
{
    int rc = BTRERR_FATAL;              // return value

    if (!btr0btr1)                      // check for null-pointer
        return BTRERR_NULLPTR;
    if (index > 0)                      // must be negative value
        return BTRERR_BAUDRATE;

    /* get SJA1000 register BTR0BTR1 from table */
    if (-BTR_SJA1000_ENTRIES < index) {
        *btr0btr1 = sja1000_btr0btr1[-index];
        rc = BTRERR_NOERROR;
    }
    else
        rc = BTRERR_BAUDRATE;

    return rc;
}

/*  -----------  local functions  ----------------------------------------
 */

static int print_bitrate(const btr_bitrate_t *bitrate, bool data, bool sam, btr_string_t string, size_t maxbyte) {
    assert(bitrate && string && maxbyte);  // just to make sure

    /* note: all fields have to be checked for their limits beforehand. But don't care! */

#if (OPTION_CAN_2_0_ONLY == OPTION_DISABLED)
    if (data) {                         // with data phase key/value pairs
        if (!sam) {                     //      w/o 'nom_sam' key/value pair
            if (snprintf(string, maxbyte,
                         "f_clock=%i,nom_brp=%u,nom_tseg1=%u,nom_tseg2=%u,nom_sjw=%u,"
                         "data_brp=%u,data_tseg1=%u,data_tseg2=%u,data_sjw=%u",
                         bitrate->btr.frequency,
                         bitrate->btr.nominal.brp,
                         bitrate->btr.nominal.tseg1,
                         bitrate->btr.nominal.tseg2,
                         bitrate->btr.nominal.sjw,
                         bitrate->btr.data.brp,
                         bitrate->btr.data.tseg1,
                         bitrate->btr.data.tseg2,
                         bitrate->btr.data.sjw) < 0)
                return BTRERR_BAUDRATE;
        } else {                        //      with'nom_sam' key/value pair
            if (snprintf(string, maxbyte,
                         "f_clock=%i,nom_brp=%u,nom_tseg1=%u,nom_tseg2=%u,nom_sjw=%u,nom_sam=%u,"
                         "data_brp=%u,data_tseg1=%u,data_tseg2=%u,data_sjw=%u",
                         bitrate->btr.frequency,
                         bitrate->btr.nominal.brp,
                         bitrate->btr.nominal.tseg1,
                         bitrate->btr.nominal.tseg2,
                         bitrate->btr.nominal.sjw,
                         bitrate->btr.nominal.sam,
                         bitrate->btr.data.brp,
                         bitrate->btr.data.tseg1,
                         bitrate->btr.data.tseg2,
                         bitrate->btr.data.sjw) < 0)
                return BTRERR_BAUDRATE;
        }
    } else {                            // w/o data phase key/value pairs
#else
        (void)data; {  // That's ugly, but ...
#endif
        if (sam) {                      //     with 'nom_sam' key/value pair
            if (snprintf(string, maxbyte,
                         "f_clock=%i,nom_brp=%u,nom_tseg1=%u,nom_tseg2=%u,nom_sjw=%u,nom_sam=%u",
                         bitrate->btr.frequency,
                         bitrate->btr.nominal.brp,
                         bitrate->btr.nominal.tseg1,
                         bitrate->btr.nominal.tseg2,
                         bitrate->btr.nominal.sjw,
                         bitrate->btr.nominal.sam) < 0)
                return BTRERR_BAUDRATE;
        } else {                        //     w/o 'nom_sam' key/value pair
            if (snprintf(string, maxbyte,
                         "f_clock=%i,nom_brp=%u,nom_tseg1=%u,nom_tseg2=%u,nom_sjw=%u",
                         bitrate->btr.frequency,
                         bitrate->btr.nominal.brp,
                         bitrate->btr.nominal.tseg1,
                         bitrate->btr.nominal.tseg2,
                         bitrate->btr.nominal.sjw) < 0)
                return BTRERR_BAUDRATE;
        }
    }
    string[maxbyte-1] = '\0';
    return BTRERR_NOERROR;
}

static int scan_bitrate(const btr_string_t string, btr_bitrate_t *bitrate, bool *data, bool *sam) {
    assert(bitrate && string);          // just to make sure
    assert(data && sam);

    memset(bitrate, 0, sizeof(btr_bitrate_t));
    *data = false;
    *sam = false;

    char str[BTR_STRING_MAX], *ptr;     // local variables
    char *key, *value;
    long number;

    int f_clock = 0;
    int nom_brp = 0;
    int nom_tseg1 = 0;
    int nom_tseg2 = 0;
    int nom_sjw = 0;
    int nom_sam = 0;
#if (OPTION_CAN_2_0_ONLY == OPTION_DISABLED)
    int data_brp = 0;
    int data_tseg1 = 0;
    int data_tseg2 = 0;
    int data_sjw = 0;
#endif
    if (strlen(string) >= BTR_STRING_MAX)
        return BTRERR_BAUDRATE;
    strncpy(str, string, BTR_STRING_MAX);
    ptr = str;

    while (*ptr != '\0') {              // lexical analysis:
        // skip blanks and scan: <key> '='
        if (!(key = skip_blanks(ptr)))
            return BTRERR_BAUDRATE;
        if (!(ptr = scan_key(key)))
            return BTRERR_BAUDRATE;
        // skip blanks and scan: <value> [',']
        if (!(value = skip_blanks(ptr)))
            return BTRERR_BAUDRATE;
        if (!(ptr = scan_value(value)))
            return BTRERR_BAUDRATE;
        // evaluate <key> '=' <value> [',']
        if (!strlen(key) || !strlen(value))
            return BTRERR_BAUDRATE;
        // convert <value> = [0-9]+
        errno = 0;
        number = strtol(value, NULL, 10);
        if (errno != 0)  // ERANGE or EINVAL
            return BTRERR_BAUDRATE;
        // f_clock: 1..INT32_MAX
        if (!strcmp(key, "f_clock")) {
            if ((0L <= number) && (number <= (long)INT32_MAX))
                bitrate->btr.frequency = (int32_t)number;
            else
                return BTRERR_BAUDRATE;
            if (f_clock++)
                return BTRERR_BAUDRATE;
        }
        // f_clock_mhz: 1..INT32_MAX/1'000'000
        else if (!strcmp(key, "f_clock_mhz")) {
            if ((0L <= number) && (number <= ((long)INT32_MAX / 1000000L)))
                bitrate->btr.frequency = (int32_t)(number * 1000000L);
            else
                return BTRERR_BAUDRATE;
            if (f_clock++)
                return BTRERR_BAUDRATE;
        }
        // nom_brp: 0..UINT16_MAX
        else if (!strcmp(key, "nom_brp")) {
            if ((0L <= number) && (number <= (long)UINT16_MAX))
                bitrate->btr.nominal.brp = (uint16_t)number;
            else
                return BTRERR_BAUDRATE;
            if (nom_brp++)
                return BTRERR_BAUDRATE;
        }
        // nom_tseg1: 0..UINT16_MAX
        else if (!strcmp(key, "nom_tseg1")) {
            if ((0L <= number) && (number <= (long)UINT16_MAX))
                bitrate->btr.nominal.tseg1 = (uint16_t)number;
            else
                return BTRERR_BAUDRATE;
            if (nom_tseg1++)
                return BTRERR_BAUDRATE;
        }
        // nom_tseg2: 0..UINT16_MAX
        else if (!strcmp(key, "nom_tseg2")) {
            if ((0L <= number) && (number <= (long)UINT16_MAX))
                bitrate->btr.nominal.tseg2 = (uint16_t)number;
            else
                return BTRERR_BAUDRATE;
            if (nom_tseg2++)
                return BTRERR_BAUDRATE;
        }
        // nom_sjw: 0..UINT16_MAX
        else if (!strcmp(key, "nom_sjw")) {
            if ((0L <= number) && (number <= (long)UINT16_MAX))
                bitrate->btr.nominal.sjw = (uint16_t)number;
            else
                return BTRERR_BAUDRATE;
            if (nom_sjw++)
                return BTRERR_BAUDRATE;
        }
        // nom_sam: 0..UINT8_MAX
        else if (!strcmp(key, "nom_sam")) {
            if ((0L <= number) && (number <= (long)UINT8_MAX))
                bitrate->btr.nominal.sam = (uint8_t)number;
            else
                return BTRERR_BAUDRATE;
            if (nom_sam++)
                return BTRERR_BAUDRATE;
            *sam = true;
        }
#if (OPTION_CAN_2_0_ONLY == OPTION_DISABLED)
        // data_brp: 0..UINT16_MAX
        else if (!strcmp(key, "data_brp")) {
            if ((0L <= number) && (number <= (long)UINT16_MAX))
                bitrate->btr.data.brp = (uint16_t)number;
            else
                return BTRERR_BAUDRATE;
            if (data_brp++)
                return BTRERR_BAUDRATE;
            *data = true;
        }
        // data_tseg1: 0..UINT16_MAX
        else if (!strcmp(key, "data_tseg1")) {
            if ((0L <= number) && (number <= (long)UINT16_MAX))
                bitrate->btr.data.tseg1 = (uint16_t)number;
            else
                return BTRERR_BAUDRATE;
            if (data_tseg1++)
                return BTRERR_BAUDRATE;
            *data = true;
        }
        // data_tseg2: 0..UINT16_MAX
        else if (!strcmp(key, "data_tseg2")) {
            if ((0L <= number) && (number <= (long)UINT16_MAX))
                bitrate->btr.data.tseg2 = (uint16_t)number;
            else
                return BTRERR_BAUDRATE;
            if (data_tseg2++)
                return BTRERR_BAUDRATE;
            *data = true;
        }
        // data_sjw: 0..UINT16_MAX
        else if (!strcmp(key, "data_sjw")) {
            if ((0L <= number) && (number <= (long)UINT16_MAX))
                bitrate->btr.data.sjw = (uint16_t)number;
            else
                return BTRERR_BAUDRATE;
            if (data_sjw++)
                return BTRERR_BAUDRATE;
            *data = true;
        }
#endif
        else {
            // unknown key
            return BTRERR_BAUDRATE;
        }
    }
    // note: if the frequency is less or equal 0 then it will be
    //       interpreted as an index to a predefined bit-rate.
    if (bitrate->btr.frequency < BTR_FREQUENCY_MIN)
        return BTRERR_BAUDRATE;
    // note: a range check has to be done by the caller!
    return BTRERR_NOERROR;
}

static char *scan_key(char *str) {
    char *ptr = str;
    assert(str);

    while (('a' <= *ptr && *ptr <= 'z') || (*ptr == '_') || ('0' <= *ptr && *ptr <= '9') || ('A' <= *ptr && *ptr <= 'Z'))
        ptr++;
    while (*ptr == ' ')
        *ptr++ = '\0';
    if ((*ptr != '='))
        return NULL;
    *ptr++ = '\0';
    return ptr;
}

static char *scan_value(char *str) {
    char *ptr = str;
    assert(str);

    while ('0' <= *ptr && *ptr <= '9')
        ptr++;
    while (*ptr == ' ')
        *ptr++ = '\0';
    if ((*ptr != ',') && (*ptr != '\0'))
        return NULL;
    if (*ptr != '\0')
        *ptr++ = '\0';
    return ptr;
}

static char *skip_blanks(char *str) {
    char *ptr = str;

    assert(str);

    while (*ptr == ' ')
        *ptr++ = '\0';
    return ptr;
}

/** @}
 */
/*  ----------------------------------------------------------------------
 *  Uwe Vogt,  UV Software,  Chausseestrasse 33 A,  10115 Berlin,  Germany
 *  Tel.: +49-30-46799872,  Fax: +49-30-46799873,  Mobile: +49-170-3801903
 *  E-Mail: uwe.vogt@uv-software.de,  Homepage: http://www.uv-software.de/
 */
