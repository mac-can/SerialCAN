/*
 *  CAN Interface API, Version 3 (Bit-rate Conversion)
 *
 *  Copyright (C) 2017-2020  Uwe Vogt, UV Software, Berlin (info@uv-software.com)
 *
 *  This file is part of CAN API V3.
 *
 *  CAN API V3 is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  CAN API V3 is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with CAN API V3.  If not, see <http://www.gnu.org/licenses/>.
 */
/** @file        can_btr.c
 *
 *  @brief       CAN Bit-rate conversion
 *
 *  @note        SJA1000 Bit-timing register BTR0 an BTR1:
 *
 *               +---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+<br>
 *               |  SJW  |          BRP          |SAM|   TSEG2   |     TSEG1     |<br>
 *               +---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+<br>
 *
 *  @author      $Author: eris $
 *
 *  @version     $Rev: 902 $
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

/*  -----------  defines  ------------------------------------------------
 */

#define BTR_FREQUENCY_MIN       (uint32_t)BTR_FREQ_SJA1000
#define BTR_FREQUENCY_MAX       (uint32_t)999999999
#define BTR_FREQUENCY_MHZ_MIN   (BTR_FREQUENCY_MIN / (uint32_t)1000000)
#define BTR_FREQUENCY_MHZ_MAX   (BTR_FREQUENCY_MAX / (uint32_t)1000000)

#define BTR_SJA1000_MAX_INDEX   10

/*  - - - - - -  helper macros   - - - - - - - - - - - - - - - - - - - - -
 */
#define BTR_INDEX(index)        (((index) < 0) ? ((index) * (-1)) : (index))

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

static int print_bitrate(const btr_bitrate_t *bitrate, bool brse, btr_string_t string);
static int scan_bitrate(const btr_string_t string, btr_bitrate_t *bitrate, bool *brse);

static char *scan_key(char *str);
static char *scan_value(char *str);
static char *skip_blanks(char *str);


/*  -----------  variables  ----------------------------------------------
 */

static const btr_sja1000_t sja1000_btr0btr1[BTR_SJA1000_MAX_INDEX] = {
    0x0014U,  // 1000 kbps (SP=75,0%, SJW=1)
    0x0016U,  //  800 kbps (SP=80,0%, SJW=1)
    0x001CU,  //  500 kbps (SP=87,5%, SJW=1)
    0x011CU,  //  250 kbps (SP=87,5%, SJW=1)
    0x031CU,  //  125 kbps (SP=87,5%, SJW=1)
    0x441CU,  //  100 kbps (SP=87,5%, SJW=2)
    0x491CU,  //   50 kbps (SP=87,5%, SJW=2)
    0x581CU,  //   20 kbps (SP=87,5%, SJW=2)
    0x711CU,  //   10 kbps (SP=87,5%, SJW=2)
    0x7F7FU   //    5 kbps (SP=68,0%, SJW=2)
};

/*  -----------  functions  ----------------------------------------------
 */

int btr_bitrate2speed(const btr_bitrate_t *bitrate, bool fdoe, bool brse, btr_speed_t *speed)
{
    btr_bitrate_t temporary;            // bit-rate settings
    bool data = brse;                   // to convert also data bit-rate settings
    int rc = BTRERR_FATAL;              // return value

    if(!bitrate || !speed)              // check for null-pointer
        return BTRERR_NULLPTR;

    if(bitrate->index <= 0) {           // CAN 2.0 bit-rate index
        if((rc = btr_index2bitrate(bitrate->index, &temporary)) != BTRERR_NOERROR)
            return rc;
        fdoe = brse = data = false;     //   could not be CAN FD!
    }
    else {                              // CAN FD bit-rate settings
        if((bitrate->btr.nominal.brp < BTR_NOMINAL_BRP_MIN) || (BTR_NOMINAL_BRP_MAX < bitrate->btr.nominal.brp))
            return BTRERR_BAUDRATE;
        if((bitrate->btr.nominal.tseg1 < BTR_NOMINAL_TSEG1_MIN) || (BTR_NOMINAL_TSEG1_MAX < bitrate->btr.nominal.tseg1))
            return BTRERR_BAUDRATE;
        if((bitrate->btr.nominal.tseg2 < BTR_NOMINAL_TSEG2_MIN) || (BTR_NOMINAL_TSEG2_MAX < bitrate->btr.nominal.tseg2))
            return BTRERR_BAUDRATE;
        if((bitrate->btr.nominal.sjw < BTR_NOMINAL_SJW_MIN) || (BTR_NOMINAL_SJW_MAX < bitrate->btr.nominal.sjw))
            return BTRERR_BAUDRATE;
#if (OPTION_CAN_2_0_ONLY == 0)
        if(fdoe && (brse ||             //   bit-rate switching enabled
                   (bitrate->btr.data.brp && bitrate->btr.data.tseg1 && bitrate->btr.data.tseg2 && bitrate->btr.data.sjw))) {
            if((bitrate->btr.data.brp < BTR_DATA_BRP_MIN) || (BTR_DATA_BRP_MAX < bitrate->btr.data.brp))
                return BTRERR_BAUDRATE;
            if((bitrate->btr.data.tseg1 < BTR_DATA_TSEG1_MIN) || (BTR_DATA_TSEG1_MAX < bitrate->btr.data.tseg1))
                return BTRERR_BAUDRATE;
            if((bitrate->btr.data.tseg2 < BTR_DATA_TSEG2_MIN) || (BTR_DATA_TSEG2_MAX < bitrate->btr.data.tseg2))
                return BTRERR_BAUDRATE;
            if((bitrate->btr.data.sjw < BTR_DATA_SJW_MIN) || (BTR_DATA_SJW_MAX < bitrate->btr.data.sjw))
                return BTRERR_BAUDRATE;
            data = true;                //   if we have all data register set, but not the brse flag!
        }
#endif
        memcpy(&temporary, bitrate, sizeof(btr_bitrate_t));
    }
    /* nominal bit-rate:
     *
     * (1) speed = freq / (brp * (1 + tseg1 + tseg2))
     *
     * (2) sp = (1 + tseg1) / (1 + tseg1 + tseg2)
     */
    speed->nominal.speed = (float)(temporary.btr.frequency)
                         / (float)(temporary.btr.nominal.brp * (1u + temporary.btr.nominal.tseg1 + temporary.btr.nominal.tseg2));
    speed->nominal.samplepoint = (float)(1u + temporary.btr.nominal.tseg1)
                               / (float)(1u + temporary.btr.nominal.tseg1 + temporary.btr.nominal.tseg2);
#if (OPTION_CAN_2_0_ONLY == 0)
    speed->nominal.fdoe = fdoe;

    /* data bit-rate (CAN FD only):
     *
     * (1) speed = freq / (brp * (1 + tseg1 + tseg2))
     *
     * (2) sp = (1 + tseg1) / (1 + tseg1 + tseg2)
     */
    if(data) {
        speed->data.speed = (float)(temporary.btr.frequency)
                          / (float)(temporary.btr.data.brp * (1u + temporary.btr.data.tseg1 + temporary.btr.data.tseg2));
        speed->data.samplepoint = (float)(1u + temporary.btr.data.tseg1)
                                / (float)(1u + temporary.btr.data.tseg1 + temporary.btr.data.tseg2);
        speed->data.brse = brse;
    }
    else {
        speed->data.speed = speed->nominal.speed; // or 0.0?
        speed->data.samplepoint = speed->nominal.samplepoint; // or 0.0?
        speed->data.brse = false;
    }
#endif
    return BTRERR_NOERROR;
}

int btr_speed2bitrate(const btr_speed_t *speed, btr_bitrate_t *bitrate)
{
    if(!bitrate || !speed)              // check for null-pointer
        return BTRERR_NULLPTR;

    /* note: there could be serveral settings to match the speed! */

    return BTRERR_NOTSUPP;              // sorry, not realized yet!
}

int btr_index2bitrate(const btr_index_t index, btr_bitrate_t *bitrate)
{
    btr_sja1000_t btr0btr1 = 0x0000u;   // SJA1000 registers
    int rc = BTRERR_FATAL;              // return value

    if(!bitrate)                        // check for null-pointer
        return BTRERR_NULLPTR;

    /* first convert the index into SJA1000 registers */
    if((rc = btr_index2sja1000(index, &btr0btr1)) != BTRERR_NOERROR)
        return rc;
    /* then convert the SJA1000 registers into bit-rate settings */
    if((rc = btr_sja10002bitrate(btr0btr1, bitrate)) != BTRERR_NOERROR)
        return rc;

    return BTRERR_NOERROR;
}

int btr_bitrate2index(const btr_bitrate_t *bitrate, btr_index_t *index)
{
    btr_sja1000_t btr0btr1;             // SJA1000 register
    int rc = BTRERR_FATAL;              // return value
    int i;

    if(!bitrate || !index)              // check for null-pointer
        return BTRERR_NULLPTR;

    /* first convert bit-rate into SJA1000 register BTR0 and BTR1 */
    if((rc = btr_bitrate2sja1000(bitrate, &btr0btr1)) != BTRERR_NOERROR)
        return rc;
    /* then look in the table of predefined bit-timing indexes */
    for(i = 0; i < BTR_SJA1000_MAX_INDEX; i++) {
        if(btr0btr1 == sja1000_btr0btr1[i]) {
            *index = (btr_index_t)(i * (-1));
            return BTRERR_NOERROR;
        }
    }
    /* bad luck, nothing found:( */
    return BTRERR_BAUDRATE;
}

int btr_string2bitrate(const btr_string_t string, btr_bitrate_t *bitrate, bool *brse)
{
    if(!bitrate || !string || !brse)    // check for null-pointer
        return BTRERR_NULLPTR;

    return scan_bitrate(string, bitrate, brse);
}

int btr_bitrate2string(const btr_bitrate_t *bitrate, bool brse, btr_string_t string)
{
    btr_bitrate_t temporary;            // bit rate settings
    bool data = brse;                   // to convert also data bit-rate settings
    int rc = BTRERR_FATAL;              // return value

    if(!bitrate || !string)             // check for null-pointer
        return BTRERR_NULLPTR;

    if(bitrate->index <= 0) {           // CAN 2.0 bit-rate index
        if((rc = btr_index2bitrate(bitrate->index, &temporary)) != BTRERR_NOERROR)
            return rc;
    }
    else {                              // CAN FD bit-rate settings
        if((bitrate->btr.nominal.brp < BTR_NOMINAL_BRP_MIN) || (BTR_NOMINAL_BRP_MAX < bitrate->btr.nominal.brp))
            return BTRERR_BAUDRATE;
        if((bitrate->btr.nominal.tseg1 < BTR_NOMINAL_TSEG1_MIN) || (BTR_NOMINAL_TSEG1_MAX < bitrate->btr.nominal.tseg1))
            return BTRERR_BAUDRATE;
        if((bitrate->btr.nominal.tseg2 < BTR_NOMINAL_TSEG2_MIN) || (BTR_NOMINAL_TSEG2_MAX < bitrate->btr.nominal.tseg2))
            return BTRERR_BAUDRATE;
        if((bitrate->btr.nominal.sjw < BTR_NOMINAL_SJW_MIN) || (BTR_NOMINAL_SJW_MAX < bitrate->btr.nominal.sjw))
            return BTRERR_BAUDRATE;
#if (OPTION_CAN_2_0_ONLY == 0)
        if(brse ||                      //   bit-rate switching enabled
          (bitrate->btr.data.brp && bitrate->btr.data.tseg1 && bitrate->btr.data.tseg2 && bitrate->btr.data.sjw)) {
            if((bitrate->btr.data.brp < BTR_DATA_BRP_MIN) || (BTR_DATA_BRP_MAX < bitrate->btr.data.brp))
                return BTRERR_BAUDRATE;
            if((bitrate->btr.data.tseg1 < BTR_DATA_TSEG1_MIN) || (BTR_DATA_TSEG1_MAX < bitrate->btr.data.tseg1))
                return BTRERR_BAUDRATE;
            if((bitrate->btr.data.tseg2 < BTR_DATA_TSEG2_MIN) || (BTR_DATA_TSEG2_MAX < bitrate->btr.data.tseg2))
                return BTRERR_BAUDRATE;
            if((bitrate->btr.data.sjw < BTR_DATA_SJW_MIN) || (BTR_DATA_SJW_MAX < bitrate->btr.data.sjw))
                return BTRERR_BAUDRATE;
            data = true;                //   if we have all data register set, but not the brse flag!
        }
#endif
        memcpy(&temporary, bitrate, sizeof(btr_bitrate_t));
    }
#if (OPTION_CAN_2_0_ONLY == 0)
    if(!data) {                         // overwrite data settings with nominal settings when not brse flag
        temporary.btr.data.brp = temporary.btr.nominal.brp;
        temporary.btr.data.tseg1 = temporary.btr.nominal.tseg1;
        temporary.btr.data.tseg2 = temporary.btr.nominal.tseg2;
        temporary.btr.data.sjw = temporary.btr.nominal.sjw;
    }
#endif
    return print_bitrate(&temporary, data, string);
}

int btr_sja10002bitrate(const btr_sja1000_t btr0btr1, btr_bitrate_t *bitrate)
{
    if(!bitrate)                        // check for null-pointer
        return BTRERR_NULLPTR;

    memset(bitrate, 0, sizeof(btr_bitrate_t));

    /* convert SJA1000 register BTR0 and BTR1 (don't forget the +1) */
    bitrate->btr.frequency = BTR_FREQ_SJA1000;
    bitrate->btr.nominal.sjw = BTR_SJW(btr0btr1) + 1u;
    bitrate->btr.nominal.brp = BTR_BRP(btr0btr1) + 1u;
    bitrate->btr.nominal.sam = BTR_SAM(btr0btr1); // 0 = sample once, 1 = three times!
    bitrate->btr.nominal.tseg2 = BTR_TSEG2(btr0btr1) + 1u;
    bitrate->btr.nominal.tseg1 = BTR_TSEG1(btr0btr1) + 1u;

    return BTRERR_NOERROR;
}

int btr_bitrate2sja1000(const btr_bitrate_t *bitrate, btr_sja1000_t *btr0btr1)
{
    if(!bitrate || !btr0btr1)           // check for null-pointer
        return BTRERR_NULLPTR;

    if(bitrate->btr.frequency != BTR_FREQ_SJA1000)
        return BTRERR_BAUDRATE;
    if((bitrate->btr.nominal.sjw < BTR_SJA1000_SJW_MIN) || (BTR_SJA1000_SJW_MAX < bitrate->btr.nominal.sjw))
        return BTRERR_BAUDRATE;
    if((bitrate->btr.nominal.brp < BTR_SJA1000_BRP_MIN) || (BTR_SJA1000_BRP_MAX < bitrate->btr.nominal.brp))
        return BTRERR_BAUDRATE;
    if(/*(bitrate->btr.nominal.sam < BTR_SJA1000_SAM_MIN) ||*/ (BTR_SJA1000_SAM_MAX < bitrate->btr.nominal.sam))
        return BTRERR_BAUDRATE;
    if((bitrate->btr.nominal.tseg2 < BTR_SJA1000_TSEG2_MIN) || (BTR_SJA1000_TSEG2_MAX < bitrate->btr.nominal.tseg2))
        return BTRERR_BAUDRATE;
    if((bitrate->btr.nominal.tseg1 < BTR_SJA1000_TSEG1_MIN) || (BTR_SJA1000_TSEG1_MAX < bitrate->btr.nominal.tseg1))
        return BTRERR_BAUDRATE;
#if (0)
    if(bitrate->btr.data.brp || bitrate->btr.data.tseg1 || bitrate->btr.data.tseg2 || bitrate->btr.data.sjw)
        return BTRERR_BAUDRATE;
#endif
    /* make SJA1000 register BTR0 and BTR1 (don't forget the -1) */
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

    if(!btr0btr1)                       // check for null-pointer
        return BTRERR_NULLPTR;

    /* get SJA1000 register BTR0 and BTR1 from table */
    if(BTR_INDEX(index) < BTR_SJA1000_MAX_INDEX) {
        *btr0btr1 = sja1000_btr0btr1[BTR_INDEX(index)];
        rc = BTRERR_NOERROR;
    }
    else
        rc = BTRERR_BAUDRATE;

    return rc;
}

/*  -----------  local functions  ----------------------------------------
 */

static int print_bitrate(const btr_bitrate_t *bitrate, bool brse, btr_string_t string)
{
    assert(bitrate && string);          // just to make sure

    /* note: all fields have been checked for their limits before */

    if(bitrate->btr.frequency == BTR_FREQ_SJA1000) { // CAN 2.0
        if(sprintf(string, "f_clock=%i,nom_brp=%u,nom_tseg1=%u,nom_tseg2=%u,nom_sjw=%u,nom_sam=%u",
                   bitrate->btr.frequency,
                   bitrate->btr.nominal.brp,
                   bitrate->btr.nominal.tseg1,
                   bitrate->btr.nominal.tseg2,
                   bitrate->btr.nominal.sjw,
                   bitrate->btr.nominal.sam) < 0)
                   return BTRERR_BAUDRATE;
    }
#if (OPTION_CAN_2_0_ONLY == 0)
    else if(!brse) {                    // CAN FD: long frames only
        if(sprintf(string, "f_clock=%i,nom_brp=%u,nom_tseg1=%u,nom_tseg2=%u,nom_sjw=%u",
                   bitrate->btr.frequency,
                   bitrate->btr.nominal.brp,
                   bitrate->btr.nominal.tseg1,
                   bitrate->btr.nominal.tseg2,
                   bitrate->btr.nominal.sjw) < 0)
            return BTRERR_BAUDRATE;
    }
    else {                              // CAN FD: long and fast frames
        if(sprintf(string, "f_clock=%i,nom_brp=%u,nom_tseg1=%u,nom_tseg2=%u,nom_sjw=%u,"
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
    }
#else
    else {  // FIXME!
        if(sprintf(string, "f_clock=%i,nom_brp=%u,nom_tseg1=%u,nom_tseg2=%u,nom_sjw=%u",
                   bitrate->btr.frequency,
                   bitrate->btr.nominal.brp,
                   bitrate->btr.nominal.tseg1,
                   bitrate->btr.nominal.tseg2,
                   bitrate->btr.nominal.sjw) < 0)
            return BTRERR_BAUDRATE;
    }
#endif
    return BTRERR_NOERROR;
}

static int scan_bitrate(const btr_string_t string, btr_bitrate_t *bitrate, bool *brse)
{
    btr_bitrate_t temporary;            // bit rate settings
    bool data = false;                  // data bit-rate settings

    assert(bitrate && string && brse);  // just to make sure

    memset(&temporary, 0, sizeof(btr_bitrate_t));

    char str[BTR_STRING_LENGTH], *ptr;  // local variables
    char *key, *value;
    uint32_t tmp = 0u;
#if (0)
    uint32_t nom_speed = 0;
    uint32_t data_speed = 0;
    float nom_sample_point = 0.0;
    float data_sample_point = 0.0;
    float tmp_float = 0.0;
#endif
    if(strlen(string) >= BTR_STRING_LENGTH)
        return BTRERR_BAUDRATE;
    strncpy(str, string, BTR_STRING_LENGTH);
    ptr = str;

    while(*ptr != '\0') {               // lexical analysis:
        tmp = 0;
#if (0)
        tmp_float = 0.0;
#endif
        // skip blanks and scan: <key> '='
        if(!(key = skip_blanks(ptr)))
            return BTRERR_BAUDRATE;
        if(!(ptr = scan_key(key)))
            return BTRERR_BAUDRATE;
        // skip blanks and scan: <value> [',']
        if(!(value = skip_blanks(ptr)))
            return BTRERR_BAUDRATE;
        if(!(ptr = scan_value(value)))
            return BTRERR_BAUDRATE;
        // evaluate <key> '=' <value> [',']
        if(strlen(value) == 0 || strlen(value) > 9)
            return BTRERR_BAUDRATE;
        // convert <value> = [0-9]+ and less or equal '999999999'
        if(strchr(value, '.') == NULL)
            tmp = (uint32_t)strtol(value, NULL, 10);
#if (0)
        else
            tmp_float = (float)strtof(value, NULL) / 100.0f;
#else
        else
            return BTRERR_BAUDRATE;
#endif
        // f_clock: (80000000, 60000000, 40000000, 30000000, 24000000, 20000000)
        if(!strcasecmp(key, "f_clock")) {
#ifndef CANBTR_PEAK_FREQUENCIES
            if((BTR_FREQUENCY_MIN <= tmp) && (tmp <= BTR_FREQUENCY_MAX))
                temporary.btr.frequency = (int32_t)tmp;
            else
                return BTRERR_BAUDRATE;
#else
            switch(tmp) {
            case 80000000u: temporary.btr.frequency = BTR_FREQ_80MHz; break;
            case 60000000u: temporary.btr.frequency = BTR_FREQ_60MHz; break;
            case 40000000u: temporary.btr.frequency = BTR_FREQ_40MHz; break;
            case 30000000u: temporary.btr.frequency = BTR_FREQ_30MHz; break;
            case 24000000u: temporary.btr.frequency = BTR_FREQ_24MHz; break;
            case 20000000u: temporary.btr.frequency = BTR_FREQ_20MHz; break;
            default: return BTRERR_BAUDRATE;
            }
#endif
        }
        // f_clock_mhz: (80, 60, 40, 30, 24, 20)
        else if(!strcasecmp(key, "f_clock_mhz")) {
#ifndef CANBTR_PEAK_FREQUENCIES
            if((BTR_FREQUENCY_MHZ_MIN <= tmp) && (tmp <= BTR_FREQUENCY_MHZ_MAX))
                temporary.btr.frequency = (int32_t)tmp * (int32_t)1000000;
            else
                return BTRERR_BAUDRATE;
#else
            switch(tmp) {
            case 80u: temporary.btr.frequency = BTR_FREQ_80MHz; break;
            case 60u: temporary.btr.frequency = BTR_FREQ_60MHz; break;
            case 40u: temporary.btr.frequency = BTR_FREQ_40MHz; break;
            case 30u: temporary.btr.frequency = BTR_FREQ_30MHz; break;
            case 24u: temporary.btr.frequency = BTR_FREQ_24MHz; break;
            case 20u: temporary.btr.frequency = BTR_FREQ_20MHz; break;
            default: return BTRERR_BAUDRATE;
            }
#endif
        }
        // nom_brp: 1..1024
        else if(!strcasecmp(key, "nom_brp")) {
            if((BTR_NOMINAL_BRP_MIN <= tmp) && (tmp <= BTR_NOMINAL_BRP_MAX))
                temporary.btr.nominal.brp = (uint16_t)tmp;
            else
                return BTRERR_BAUDRATE;
        }
        // nom_tseg1: 1..256
        else if(!strcasecmp(key, "nom_tseg1")) {
            if((BTR_NOMINAL_TSEG1_MIN <= tmp) && (tmp <= BTR_NOMINAL_TSEG1_MAX))
                temporary.btr.nominal.tseg1 = (uint16_t)tmp;
            else
                return BTRERR_BAUDRATE;
        }
        // nom_tseg2: 1..128
        else if(!strcasecmp(key, "nom_tseg2")) {
            if((BTR_NOMINAL_TSEG2_MIN <= tmp) && (tmp <= BTR_NOMINAL_TSEG2_MAX))
                temporary.btr.nominal.tseg2 = (uint16_t)tmp;
            else
                return BTRERR_BAUDRATE;
        }
        // nom_sjw: 1..128
        else if(!strcasecmp(key, "nom_sjw")) {
            if((BTR_NOMINAL_SJW_MIN <= tmp) && (tmp <= BTR_NOMINAL_SJW_MAX))
                temporary.btr.nominal.sjw = (uint16_t)tmp;
            else
                return BTRERR_BAUDRATE;
        }
        // nom_sam: (none)
        else if(!strcasecmp(key, "nom_sam")) {
#if (0)
            if((BTR_NOMINAL_SAM_MIN <= tmp) && (tmp <= BTR_NOMINAL_SAM_MAX))
                temporary.btr.nominal.sam = (uint8_t)tmp;
            else
                return BTRERR_BAUDRATE;
#else
            // FIXME: SJA1000 = {0, 1} vs. Kvaser = {1, 3}
            temporary.btr.nominal.sam = (uint8_t)tmp;
#endif
        }
#if (OPTION_CAN_2_0_ONLY == 0)
        // data_brp: 1..1024
        else if(!strcasecmp(key, "data_brp")) {
            if((BTR_DATA_BRP_MIN <= tmp) && (tmp <= BTR_DATA_BRP_MAX))
                temporary.btr.data.brp = (uint16_t)tmp;
            else
                return BTRERR_BAUDRATE;
            data = true;
        }
        // data_tseg1: 1..32
        else if(!strcasecmp(key, "data_tseg1")) {
            if((BTR_DATA_TSEG1_MIN <= tmp) && (tmp <= BTR_DATA_TSEG1_MAX))
                temporary.btr.data.tseg1 = (uint16_t)tmp;
            else
                return BTRERR_BAUDRATE;
            data = true;
        }
        // data_tseg2: 1..16
        else if(!strcasecmp(key, "data_tseg2")) {
            if((BTR_DATA_TSEG2_MIN <= tmp) && (tmp <= BTR_DATA_TSEG2_MAX))
                temporary.btr.data.tseg2 = (uint16_t)tmp;
            else
                return BTRERR_BAUDRATE;
            data = true;
        }
        // data_sjw: 1..16
        else if(!strcasecmp(key, "data_sjw")) {
            if((BTR_DATA_SJW_MIN <= tmp) && (tmp <= BTR_DATA_SJW_MAX))
                temporary.btr.data.sjw = (uint16_t)tmp;
            else
                return BTRERR_BAUDRATE;
            data = true;
        }
        // data_ssp_offset: (none)
        else if(!strcasecmp(key, "data_ssp_offset")) {
            // not used
        }
#if (0)
        // nom_speed: 1000..2000000
        else if (!strcasecmp(key, "nom_speed")) {
            if (tmp < 1000ul)
                tmp *= 1000ul;
            if ((BTR_NOMINAL_SPEED_MIN <= tmp) && (tmp <= BTR_NOMINAL_SPEED_MAX))
                nom_speed = (uint32_t)tmp;
            else
                return BTRERR_BAUDRATE;
        }
        // nom_sp: 0.05..1.00
        else if (!strcasecmp(key, "nom_sp")) {
            if (tmp_float == 0.0)
                tmp_float = (float)tmp / 100.0f;
            if ((BTR_NOMINAL_SP_MIN <= tmp_float) && (tmp_float <= BTR_NOMINAL_SP_MAX))
                nom_sample_point = tmp_float;
            else
                return BTRERR_BAUDRATE;
        }
        // data_speed: 1000..12000000
        else if (!strcasecmp(key, "data_speed")) {
            if (tmp < 100ul)
                tmp *= 1000000ul;
            if (tmp < 1000ul)
                tmp *= 1000ul;
            if ((BTR_DATA_SPEED_MIN <= tmp) && (tmp <= BTR_DATA_SPEED_MAX))
                data_speed = (uint32_t)tmp;
            else
                return BTRERR_BAUDRATE;
        }
        // data_sp: 0.1..1.0
        else if (!strcasecmp(key, "data_sp")) {
            if (tmp_float == 0.0)
                tmp_float = (float)tmp / 100.0f;
            if ((BTR_DATA_SP_MIN <= tmp_float) && (tmp_float <= BTR_DATA_SP_MAX))
                data_sample_point = tmp_float;
            else
                return BTRERR_BAUDRATE;
        }
#endif
#endif
        else {
            // unknown key
            return BTRERR_BAUDRATE;
        }
    }
#if (0)
    if (temporary.btr.frequency) {
        /* f_clock=<freq>,nom_speed=<nom_speed>,nom_sp=<nom_sample_point>[,data_speed=<data_speed>,data_sp=<data_sample_point>] */
        if ((nom_speed && (nom_sample_point != 0.0)) &&
            !((temporary.btr.nominal.brp || temporary.btr.nominal.tseg1 || temporary.btr.nominal.tseg2 || temporary.btr.nominal.sjw || temporary.btr.nominal.sam) ||
            (temporary.btr.data.brp || temporary.btr.data.tseg1 || temporary.btr.data.tseg2 || temporary.btr.data.sjw))) {
            if (!btr_find_bit_timing_nominal(nom_speed, nom_sample_point, temporary.btr.frequency, btr_nominal, BTR_OPTION_TQ_FEW))
                return BTRERR_BAUDRATE;

            if (data_speed && (data_sample_point != 0.0)) {
                if (!btr_find_bit_timing_data(data_speed, data_sample_point, temporary.btr.frequency, btr_data, BTR_OPTION_TQ_FEW))
                    return BTRERR_BAUDRATE;
            }
            else if (data_speed || (data_sample_point != 0.0))
                return BTRERR_BAUDRATE;
        }
        else if (nom_speed || (nom_sample_point != 0.0))
            return BTRERR_BAUDRATE;

        /* f_clock=<freq>,nom_brp=<nom_brp>,nom_tseg1=<nom_tseg1>,nom_tseg2=<nom_tseg2>,nom_sjw=<nom_sjw>[,nom_sam=<nom_sam>]
                        [,data_brp=<data_brp>,data_tseg1=<data_tseg1>,data_tseg2=<data_tseg2>,data_sjw=<data_sjw>] */
        if ((temporary.btr.nominal.brp && temporary.btr.nominal.tseg1 && temporary.btr.nominal.tseg2 && temporary.btr.nominal.sjw) &&
            ((temporary.btr.data.brp && temporary.btr.data.tseg1 && temporary.btr.data.tseg2 && temporary.btr.data.sjw) ||
            (!temporary.btr.data.brp && !temporary.btr.data.tseg1 && !temporary.btr.data.tseg2 && !temporary.btr.data.sjw)))
            return 1;
    }
#endif
    /* note: without any further verification */
    memcpy(bitrate, &temporary, sizeof(btr_bitrate_t));
    *brse = data;

    return BTRERR_NOERROR;
}

static char *scan_key(char *str)
{
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

static char *scan_value(char *str)
{
    char *ptr = str;
    int   dot = 0;

    assert(str);

    while (('0' <= *ptr && *ptr <= '9') || (*ptr == '.')) {
        if ((*ptr == '.') && (++dot > 1))
            return ptr;
        ptr++;
    }
    while (*ptr == ' ')
        *ptr++ = '\0';
    if ((*ptr != ',') && (*ptr != '\0'))
        return NULL;
    if (*ptr != '\0')
        *ptr++ = '\0';
    return ptr;
}

static char *skip_blanks(char *str)
{
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
