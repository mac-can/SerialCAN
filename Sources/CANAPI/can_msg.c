/*
 *  CAN Interface API, Version 3 (Message Formatter)
 *
 *  Copyright (C) 2019-2020  Uwe Vogt, UV Software, Berlin (info@uv-software.com)
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
/** @file        can_msg.c
 *
 *  @brief       CAN Message Formatter
 *
 *  @author      $Author: haumea $
 *
 *  @version     $Rev: 951 $
 *
 *  @addtogroup  can_msg
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
#include "can_msg.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>
#include <errno.h>
#include <assert.h>

#include <ctype.h>
#include <time.h>
#if !defined(_WIN32) && !defined(_WIN64)
#include <sys/time.h>
#else
#include <windows.h>
#endif

/*  -----------  defines  ------------------------------------------------
 */

#ifndef DLC2LEN
#define DLC2LEN(x)  dlc_table[x & 0xF]
#endif
#ifndef LEN2DLC
#define LEN2DLC(x)  ((x) > 48) ? 0xF : \
                    ((x) > 32) ? 0xE : \
                    ((x) > 24) ? 0xD : \
                    ((x) > 20) ? 0xC : \
                    ((x) > 16) ? 0xB : \
                    ((x) > 12) ? 0xA : \
                    ((x) > 8) ?  0x9 : (x)
#endif


/*  -----------  types  --------------------------------------------------
 */


/*  -----------  prototypes  ---------------------------------------------
 */

static void format_time(char *string, const msg_message_t *message);
static void format_id(char *string, const msg_message_t *message);
static void format_dlc(char *string, const msg_message_t *message);
static void format_data(char *string, const msg_message_t *message, int ascii, int indent);
static void format_ascii(char *string, const msg_message_t *message);
static void format_data_byte(char *string, unsigned char data);
static void format_data_ascii(char *string, unsigned char data);
static void format_fill_byte(char *string);


/*  -----------  variables  ----------------------------------------------
 */

static struct {                         /* format option: */
    msg_fmt_timestamp_t  time_stamp;    /*   time-stamp {ZERO, ABS, REL} */
    msg_fmt_option_t     time_usec;     /*   time-stamp in usec {OFF, ON} */
    msg_fmt_time_t       time_format;   /*   time format {TIME, SEC, DJD} */
    msg_fmt_number_t     id;            /*   identifier {HEX, DEC, OCT, BIN} */
    msg_fmt_option_t     id_xtd;        /*   extended identifier {OFF, ON} */
    msg_fmt_number_t     dlc;           /*   DLC/length {HEX, DEC, OCT, BIN} */
    msg_fmt_canfd_t      dlc_format;    /*   CAN FD format {DLC, LENGTH} */
    int                  dlc_brackets;  /*   DLC in brackets {'\0', '(', '['} */
    msg_fmt_option_t     flags;         /*   message flags {ON, OFF} */
    msg_fmt_number_t     data;          /*   message data {HEX, DEC, OCT, BIN} */
    msg_fmt_option_t     ascii;         /*   data as ASCII {ON, OFF} */
    int                  ascii_subst;   /*   substitute for non-printables */
    msg_fmt_option_t     channel;       /*   message source {OFF, ON} */
    msg_fmt_option_t     counter;       /*   message counter {ON, OFF} */
    msg_fmt_separator_t  separator;     /*   separator {SPACES, TABS} */
    msg_fmt_wraparound_t wraparound;    /*   wraparound {NO, 8, 16, 32, 64} */
    msg_fmt_option_t     end_of_line;   /*   end-of-line character {ON, OFF} */
    char                 rx_prompt[6+1];/*   prompt for received messages */
    char                 tx_prompt[6+1];/*   prompt for sent messages */
}   msg_option = {
                        .time_stamp = MSG_FMT_TIMESTAMP_ZERO,
                        .time_usec = MSG_FMT_OPTION_OFF,
                        .time_format = MSG_FMT_TIME_SEC,
                        .id = MSG_FMT_NUMBER_HEX,
                        .id_xtd = MSG_FMT_OPTION_OFF,
                        .dlc = MSG_FMT_NUMBER_DEC,
                        .dlc_format = MSG_FMT_CANFD_LENGTH,
                        .dlc_brackets = '\0',
                        .flags = MSG_FMT_OPTION_ON,
                        .data = MSG_FMT_NUMBER_HEX,
                        .ascii = MSG_FMT_OPTION_ON,
                        .ascii_subst = '.',
                        .channel = MSG_FMT_OPTION_OFF,
                        .counter = MSG_FMT_OPTION_ON,
                        .separator = MSG_FMT_SEPARATOR_SPACES,
                        .wraparound = MSG_FMT_WRAPAROUND_NO,
                        .end_of_line = MSG_FMT_OPTION_OFF,
                        .rx_prompt = "",
                        .tx_prompt = ""
};
static msg_format_t msg_format = MSG_FORMAT_DEFAULT;
static char msg_string[MSG_STRING_LENGTH] = "";
static const unsigned char dlc_table[16] = {
    0,1,2,3,4,5,6,7,8,12,16,20,24,32,48,64
};


/*  -----------  functions  ----------------------------------------------
 */

char *msg_format_message(const msg_message_t *message, msg_direction_t direction,
                               msg_counter_t counter, msg_channel_t channel)
{
    char tmp_string[MSG_STRING_LENGTH];

    memset(msg_string, 0, sizeof(msg_string));

    if (message) {
        /* prompt (optional) */
        if (strlen(msg_option.tx_prompt) && (direction == MSG_TX_MESSAGE)) {
            strcat(msg_string, msg_option.tx_prompt);
            strcat(msg_string, (msg_option.separator == MSG_FMT_SEPARATOR_TABS) ? "\t" : " ");
        }
        else if (strlen(msg_option.rx_prompt)) { /* defaults to MSG_DIRECTION_RX_MSG */
            strcat(msg_string, msg_option.rx_prompt);
            strcat(msg_string, (msg_option.separator == MSG_FMT_SEPARATOR_TABS) ? "\t" : " ");
        }
        /* counter (optional) */
        if ((msg_option.counter != MSG_FMT_OPTION_OFF) && ((msg_option.separator == MSG_FMT_SEPARATOR_TABS))) {
            sprintf(tmp_string, "%" PRIu64 "\t", counter);
            strcat(msg_string, tmp_string);
        }
        else if (msg_option.counter != MSG_FMT_OPTION_OFF) { /* defaults to MSG_FMT_SEPARATOR_SPACES */
            sprintf(tmp_string, "%-7" PRIu64 "  ", counter);
            strcat(msg_string, tmp_string);
        }
        /* time-stamp (abs/rel/zero) (hhmmss/sec/DJD).(msec/usec) */
        format_time(tmp_string, message);
        strcat(msg_string, tmp_string);
        strcat(msg_string, (msg_option.separator == MSG_FMT_SEPARATOR_TABS) ? "\t" : "  ");

        /* channel (optional) */
        if ((msg_option.channel != MSG_FMT_OPTION_OFF) && (msg_option.separator == MSG_FMT_SEPARATOR_TABS)) {
            sprintf(tmp_string, "%i\t", channel);
            strcat(msg_string, tmp_string);
        }
        else if (msg_option.channel != MSG_FMT_OPTION_OFF) { /* defaults to MSG_FMT_SEPARATOR_SPACES */
            sprintf(tmp_string, "%-2i  ", channel);
            strcat(msg_string, tmp_string);
        }
        /* identifier (hex/dec/oct) */
        format_id(tmp_string, message);
        strcat(msg_string, tmp_string);
        strcat(msg_string, (msg_option.separator == MSG_FMT_SEPARATOR_TABS) ? "\t" : "  ");

        /* flags (optional) */
        if (msg_option.flags != MSG_FMT_OPTION_OFF) {
            strcat(msg_string, message->xtd ? "X" : "S");
#if (OPTION_CAN_2_0_ONLY == 0)
            if (message->fdf) {
                strcat(msg_string, message->fdf ? "F" : " ");
                strcat(msg_string, message->brs ? "B" : " ");
                strcat(msg_string, message->esi ? "E" : " ");
            }
            else
#endif
                strcat(msg_string, message->rtr ? "R" : " ");
            strcat(msg_string, (msg_option.separator == MSG_FMT_SEPARATOR_TABS) ? "\t" : " ");  /* only one space! */
        }
        /* dlc/length (hex/dec/oct) */
        format_dlc(tmp_string, message);
        strcat(msg_string, tmp_string);

        /* data (hex/dec/oct) plus ascii (optional) */
        if (message->dlc && !message->rtr) {
            strcat(msg_string, (msg_option.separator == MSG_FMT_SEPARATOR_TABS) ? "\t" : "  ");
            format_data(tmp_string, message, (msg_option.ascii == MSG_FMT_OPTION_OFF) ? 0 : 1, (int)strlen(msg_string));
            strcat(msg_string, tmp_string);
        }
        /* end-of-line (optional) */
        if (msg_option.end_of_line) {
            strcat(msg_string, "\n");
        }
    }
    return msg_string;
}

char *msg_format_time(const msg_message_t *message)
{
    memset(msg_string, 0, sizeof(msg_string));

    if (message) {
        /* time-stamp (abs/rel/zero) (hhmmss/sec/DJD).(msec/usec) */
        format_time(msg_string, message);
    }
    return msg_string;
}

char *msg_format_id(const msg_message_t *message)
{
    memset(msg_string, 0, sizeof(msg_string));

    if (message) {
        /* identifier (hex/dec/oct) */
        format_id(msg_string, message);
    }
    return msg_string;
}

char *msg_format_flags(const msg_message_t *message)
{
    memset(msg_string, 0, sizeof(msg_string));

    if (message) {
        /* flags (optional) */
        strcat(msg_string, message->xtd ? "X" : "S");
#if (OPTION_CAN_2_0_ONLY == 0)
        if (message->fdf) {
            strcat(msg_string, message->fdf ? "F" : " ");
            strcat(msg_string, message->brs ? "B" : " ");
            strcat(msg_string, message->esi ? "E" : " ");
        }
        else
#endif
            strcat(msg_string, message->rtr ? "R" : " ");
    }
    return msg_string;
}

char *msg_format_dlc(const msg_message_t *message)
{
    memset(msg_string, 0, sizeof(msg_string));

    if (message) {
        /* dlc/length (hex/dec/oct) */
        format_dlc(msg_string, message);
    }
    return msg_string;
}

char *msg_format_data(const msg_message_t *message)
{
    memset(msg_string, 0, sizeof(msg_string));

    if (message) {
        /* data (hex/dec/oct) */
        if (message->dlc) {
            format_data(msg_string, message, 0, 0);
        }
    }
    return msg_string;
}

char *msg_format_ascii(const msg_message_t *message)
{
    memset(msg_string, 0, sizeof(msg_string));

    if (message) {
        /* data (hex/dec/oct) */
        if (message->dlc) {
            format_ascii(msg_string, message);
        }
    }
    return msg_string;
}


/* message output format {DEFAULT, ...} */
int msg_set_format(msg_format_t format)
{
    int rc = 1;

    switch (format) {
    case MSG_FORMAT_DEFAULT:
        msg_format = format;
        break;
    default:
        rc = 0;
        break;
    }
    return rc;
}

/* formatter option: time-stamp {ZERO, ABS, REL} */
int  msg_set_fmt_time_stamp(msg_fmt_timestamp_t option)
{
    int rc = 1;

    switch (option) {
    case MSG_FMT_TIMESTAMP_ZERO:
    case MSG_FMT_TIMESTAMP_ABSOLUTE:
    case MSG_FMT_TIMESTAMP_RELATIVE:
        msg_option.time_stamp = option;
        break;
    default:
        rc = 0;
        break;
    }
    return rc;
}

/* formatter option: time-stamp in usec {ON, OFF} */
int msg_set_fmt_time_usec(msg_fmt_option_t option)
{
    int rc = 1;

    switch (option) {
    case MSG_FMT_OPTION_OFF:
    case MSG_FMT_OPTION_ON:
        msg_option.time_usec = option;
        break;
    default:
        rc = 0;
        break;
    }
    return rc;
}

/* formatter option: time format {TIME, SEC, DJD} */
int msg_set_fmt_time_format(msg_fmt_time_t option)
{
    int rc = 1;

    switch (option) {
    case MSG_FMT_TIME_HHMMSS:
    case MSG_FMT_TIME_SEC:
    case MSG_FMT_TIME_DJD:
        msg_option.time_format = option;
        break;
    default:
        rc = 0;
        break;
    }
    return rc;
}

/* formatter option: identifier {HEX, DEC, OCT, BIN} */
int msg_set_fmt_id(msg_fmt_number_t option)
{
    int rc = 1;

    switch (option) {
    case MSG_FMT_NUMBER_HEX:
    case MSG_FMT_NUMBER_DEC:
    case MSG_FMT_NUMBER_OCT:
        msg_option.id = option;
        break;
    default:
        rc = 0;
        break;
    }
    return rc;
}

/* formatter option: extended identifier {ON, OFF} */
int msg_set_fmt_id_xtd(msg_fmt_option_t option)
{
    int rc = 1;

    switch (option) {
    case MSG_FMT_OPTION_OFF:
    case MSG_FMT_OPTION_ON:
        msg_option.id_xtd = option;
        break;
    default:
        rc = 0;
        break;
    }
    return rc;
}

/* formatter option: DLC/length {HEX, DEC, OCT, BIN} */
int msg_set_fmt_dlc(msg_fmt_number_t option)
{
    int rc = 1;

    switch (option) {
    case MSG_FMT_NUMBER_HEX:
    case MSG_FMT_NUMBER_DEC:
    case MSG_FMT_NUMBER_OCT:
        msg_option.dlc = option;
        break;
    default:
        rc = 0;
        break;
    }
    return rc;
}

/* formatter option: CAN FD format {DLC, LENGTH} */
int msg_set_fmt_dlc_format(msg_fmt_canfd_t option)
{
    int rc = 1;

    switch (option) {
    case  MSG_FMT_CANFD_DLC:
    case  MSG_FMT_CANFD_LENGTH:
        msg_option.dlc_format = option;
        break;
    default:
        rc = 0;
        break;
    }
    return rc;
}

/* formatter option: DLC in brackets {'\0', '(', '['} */
int msg_set_fmt_dlc_brackets(int option)
{
    int rc = 1;

    switch (option) {
    case '\0':
    case '(':
    case '[':
        msg_option.dlc_brackets = option;
        break;
    default:
        rc = 0;
        break;
    }
    return rc;
}

/* formatter option: message flags {ON, OFF} */
int msg_set_fmt_flags(msg_fmt_option_t option)
{
    int rc = 1;

    switch (option) {
    case MSG_FMT_OPTION_OFF:
    case MSG_FMT_OPTION_ON:
        msg_option.flags = option;
        break;
    default:
        rc = 0;
        break;
    }
    return rc;
}

/* formatter option: message data {HEX, DEC, OCT, BIN} */
int msg_set_fmt_data(msg_fmt_number_t option)
{
    int rc = 1;

    switch (option) {
    case MSG_FMT_NUMBER_HEX:
    case MSG_FMT_NUMBER_DEC:
    case MSG_FMT_NUMBER_OCT:
        msg_option.data = option;
        break;
    default:
        rc = 0;
        break;
    }
    return rc;
}

/* formatter option: data as ASCII {ON, OFF} */
int msg_set_fmt_ascii(msg_fmt_option_t option)
{
    int rc = 1;

    switch (option) {
    case MSG_FMT_OPTION_OFF:
    case MSG_FMT_OPTION_ON:
        msg_option.ascii = option;
        break;
    default:
        rc = 0;
        break;
    }
    return rc;
}

/* formatter option: substitute for non-printables */
int msg_set_fmt_ascii_subst(int option)
{
    int rc = 1;

    if (isprint(option))
        msg_option.ascii_subst = option;
    else
        rc = 0;
    return rc;
}

/* formatter option: message source {ON, OFF} */
int msg_set_fmt_channel(msg_fmt_option_t option)
{
    int rc = 1;

    switch (option) {
    case MSG_FMT_OPTION_OFF:
    case MSG_FMT_OPTION_ON:
        msg_option.channel = option;
        break;
    default:
        rc = 0;
        break;
    }
    return rc;
}

/* formatter option: message counter {ON, OFF} */
int msg_set_fmt_counter(msg_fmt_option_t option)
{
    int rc = 1;

    switch (option) {
    case MSG_FMT_OPTION_OFF:
    case MSG_FMT_OPTION_ON:
        msg_option.counter = option;
        break;
    default:
        rc = 0;
        break;
    }
    return rc;
}

/* formatter option: separator {SPACES, TABS} */
int msg_set_fmt_separator(msg_fmt_separator_t option)
{
    int rc = 1;

    switch (option) {
    case MSG_FMT_SEPARATOR_SPACES:
    case MSG_FMT_SEPARATOR_TABS:
        msg_option.separator = option;
        break;
    default:
        rc = 0;
        break;
    }
    return rc;
}

/* formatter option: wraparound {NO, 8, 16, 32, 64} */
int msg_set_fmt_wraparound(msg_fmt_wraparound_t option)
{
    int rc = 1;

    switch (option) {
    case MSG_FMT_WRAPAROUND_NO:
    case MSG_FMT_WRAPAROUND_8:
    case MSG_FMT_WRAPAROUND_10:
    case MSG_FMT_WRAPAROUND_16:
    case MSG_FMT_WRAPAROUND_32:
    case MSG_FMT_WRAPAROUND_64:
        msg_option.wraparound = option;
        break;
    default:
        rc = 0;
        break;
    }
    return rc;
}

/* formatter option: end-of-line character {ON, OFF} */
int msg_set_fmt_eol(msg_fmt_option_t option)
{
    int rc = 1;

    switch (option) {
    case MSG_FMT_OPTION_OFF:
    case MSG_FMT_OPTION_ON:
        msg_option.end_of_line = option;
        break;
    default:
        rc = 0;
        break;
    }
    return rc;
}

/* formatter option: prompt for received messages */
int msg_set_fmt_rx_prompt(const char *option)
{
    int rc = 1;

    if (strlen(option) <= 6)
        strcpy(msg_option.rx_prompt, option);
    else
        rc = 0;
    return rc;
}

/* formatter option: prompt for sent messages */
int msg_set_fmt_tx_prompt(const char *option)
{
    int rc = 1;

    if (strlen(option) <= 6)
        strcpy(msg_option.tx_prompt, option);
    else
        rc = 0;
    return rc;
}

/*  -----------  local functions  ----------------------------------------
 */

static void format_time(char *string, const msg_message_t *message)
{
    static msg_timestamp_t laststamp = { 0, 0 };
    struct timespec difftime;
    struct tm tm; time_t t;
    char   timestring[25];
    double djd;

    assert(string);
    assert(message);

    switch (msg_option.time_stamp) {
    case MSG_FMT_TIMESTAMP_RELATIVE:
    case MSG_FMT_TIMESTAMP_ZERO:
        if (laststamp.tv_sec == 0) { /* first init */
            laststamp.tv_sec = message->timestamp.tv_sec;
            laststamp.tv_nsec = message->timestamp.tv_nsec;
        }
        difftime.tv_sec = message->timestamp.tv_sec - laststamp.tv_sec;
        difftime.tv_nsec = message->timestamp.tv_nsec - laststamp.tv_nsec;
        if (difftime.tv_nsec < 0) {
            difftime.tv_sec -= 1;
            difftime.tv_nsec += 1000000000;
        }
        if (difftime.tv_sec < 0) {
            difftime.tv_sec = 0;
            difftime.tv_nsec = 0;
        }
        if (msg_option.time_stamp == MSG_FMT_TIMESTAMP_RELATIVE) { /* update for delta calculation */
            laststamp.tv_sec = message->timestamp.tv_sec;
            laststamp.tv_nsec = message->timestamp.tv_nsec;
        }
        t = (time_t)difftime.tv_sec;
        tm = *gmtime(&t);
        break;
    case MSG_FMT_TIMESTAMP_ABSOLUTE:
    default:
        difftime.tv_sec = message->timestamp.tv_sec;
        difftime.tv_nsec = message->timestamp.tv_nsec;
        t = (time_t)message->timestamp.tv_sec;
        tm = *localtime(&t);
        break;
    }
    switch (msg_option.time_format) {
    case MSG_FMT_TIME_HHMMSS:
        strftime(timestring, 24, "%H:%M:%S", &tm); // TODO: tm > 24h (?)
        if (msg_option.time_usec)
            sprintf(string, "%s.%06li", timestring, (long)difftime.tv_nsec / 1000L);
        else/* resolution is 0.1 milliseconds! */
            sprintf(string, "%s.%04li", timestring, (long)difftime.tv_nsec / 100000L);
        break;
    case MSG_FMT_TIME_DJD:
        if (!msg_option.time_usec)  /* round to milliseconds resolution */
            difftime.tv_nsec = ((difftime.tv_nsec + 500000L) / 1000000L) * 1000000L;
        djd = (double)difftime.tv_sec / (double)86400;
        djd += (double)difftime.tv_nsec / (double)86400000000000;
        if (msg_option.time_usec)
            sprintf(string, "%1.12lf", djd);
        else
            sprintf(string, "%1.9lf", djd);
        break;
    case MSG_FMT_TIME_SEC:
    default:
        if (msg_option.time_usec)
            sprintf(string, "%3li.%06li", (long)difftime.tv_sec, (long)difftime.tv_nsec / 1000L);
        else/* resolution is 0.1 milliseconds! */
            sprintf(string, "%3li.%04li", (long)difftime.tv_sec, (long)difftime.tv_nsec / 100000L);
        break;
    }
}

static void format_id(char *string, const msg_message_t *message)
{
    assert(string);
    assert(message);

    string[0] = '\0';
    switch (msg_option.id) {
    case MSG_FMT_NUMBER_DEC:
        if (!msg_option.id_xtd)
            sprintf(string, "%-4" PRIu32, message->id);
        else
            sprintf(string, "%-9" PRIu32, message->id);
        break;
    case MSG_FMT_NUMBER_OCT:
        if (!msg_option.id_xtd)
            sprintf(string, "%04" PRIo32, message->id);
        else
            sprintf(string, "%010" PRIo32, message->id);
        break;
    case MSG_FMT_NUMBER_HEX:
    default:
        if (!msg_option.id_xtd)
            sprintf(string, "%03" PRIX32, message->id);
        else
            sprintf(string, "%08" PRIX32, message->id);
        break;
    }
}

static void format_dlc(char *string, const msg_message_t *message)
{
    assert(string);
    assert(message);

    unsigned char length = (msg_option.dlc_format == MSG_FMT_CANFD_DLC) ? message->dlc : DLC2LEN(message->dlc);
    char pre = '\0', post = '\0';
    int blank = 0;

    string[0] = '\0';
    switch (msg_option.dlc_brackets) {
    case '(': pre = '('; post = ')'; break;
    case '[': pre = '['; post = ']'; break;
    default: break;
    }
    switch (msg_option.dlc) {
    case MSG_FMT_NUMBER_DEC:
        if (pre && post)
            sprintf(string, "%c%u%c", pre, length, post);
        else
            sprintf(string, "%u", length);
        blank = length >= 10 ? 0 : 1;
        break;
    case MSG_FMT_NUMBER_OCT:
        if (pre && post)
            sprintf(string, "%c%02o%c", pre, length, post);
        else
            sprintf(string, "%02o", length);
        blank = length >= 64 ? 0 : 1;
        break;
    case MSG_FMT_NUMBER_HEX:
    default:
        if (pre && post)
            sprintf(string, "%c%X%c", pre, length, post);
        else
            sprintf(string, "%X", length);
        break;
    }
#if (OPTION_CAN_2_0_ONLY == 0)
    if (message->fdf && blank)
        strcat(string, " ");
#else
    (void)blank;  /* to avoid compiler warnings */
#endif
}

static void format_data(char *string, const msg_message_t *message, int ascii, int indent)
{
    assert(string);
    assert(message);

    int length = DLC2LEN(message->dlc);
    int i, j, col, wraparound;
    char datastring[8];

    string[0] = '\0';
#if (OPTION_CAN_2_0_ONLY == 0)
    if (msg_option.wraparound == MSG_FMT_WRAPAROUND_NO)
        wraparound = message->fdf ? (int)MSG_FMT_WRAPAROUND_64 : (int)MSG_FMT_WRAPAROUND_8;
    else
        wraparound = (int)msg_option.wraparound;
#else
    wraparound = (int)MSG_FMT_WRAPAROUND_8;
#endif
    for (i = 0, j = 0, col = 0; i < length; i++) {
        format_data_byte(datastring, message->data[i]);
        strcat(string, datastring);
        if ((i + 1) < length) {
            if ((col + 1) == wraparound) {
                if (ascii) {
                    strcat(string, msg_option.separator == MSG_FMT_SEPARATOR_TABS ? "\t" : "  ");
                    for (col = 0; col < (int)msg_option.wraparound; j++, col++) {
                        format_data_ascii(datastring, message->data[j]);
                        strcat(string, datastring);
                    }
                }
                strcat(string, "\n");
                if (msg_option.separator != MSG_FMT_SEPARATOR_TABS) {
                    for (col = 0; col < indent; col++)
                        strcat(string, " ");
                }
                else
                    strcat(string, "\t");
                col = 0;
            }
            else {
                strcat(string, " ");
                col++;
            }
        }
        else
            col++;
    }
    if (ascii) {
        if ((col < wraparound) && (i != 0)) {
            strcat(string, " ");
            for (; col < wraparound; col++) {
                format_fill_byte(datastring);
                strcat(string, datastring);
                if ((col + 1) != wraparound)
                    strcat(string, " ");
            }
        }
        strcat(string, msg_option.separator == MSG_FMT_SEPARATOR_TABS ? "\t" : "  ");
        for (; j < length; j++) {
            format_data_ascii(datastring, message->data[j]);
            strcat(string, datastring);
        }
    }
}

static void format_ascii(char *string, const msg_message_t *message)
{
    assert(string);
    assert(message);

    int length = DLC2LEN(message->dlc);
    int i, col, wraparound;
    char datastring[8];

    string[0] = '\0';
#if (OPTION_CAN_2_0_ONLY == 0)
    if (msg_option.wraparound == MSG_FMT_WRAPAROUND_NO)
        wraparound = message->fdf ? (int)MSG_FMT_WRAPAROUND_64 : (int)MSG_FMT_WRAPAROUND_8;
    else
        wraparound = (int)msg_option.wraparound;
#else
    wraparound = (int)MSG_FMT_WRAPAROUND_8;
#endif
    for (i = 0, col = 0; i < length; i++) {
        format_data_ascii(datastring, message->data[i]);
        strcat(string, datastring);
        if ((i + 1) < length) {
            if ((col + 1) == wraparound) {
                strcat(string, "\n");
                col = 0;
            }
            else {
                strcat(string, " ");
                col++;
            }
        }
    }
}

static void format_data_byte(char *string, unsigned char data)
{
    assert(string);

    switch (msg_option.data) {
    case MSG_FMT_NUMBER_DEC:
        sprintf(string, "%-3u", data);
        break;
    case MSG_FMT_NUMBER_OCT:
        sprintf(string, "%03o", data);
        break;
    case MSG_FMT_NUMBER_HEX:
    default:
        sprintf(string, "%02X", data);
        break;
    }
}

static void format_fill_byte(char *string)
{
    assert(string);

    switch (msg_option.data) {
    case MSG_FMT_NUMBER_DEC:
        sprintf(string, "   ");
        break;
    case MSG_FMT_NUMBER_OCT:
        sprintf(string, "   ");
        break;
    case MSG_FMT_NUMBER_HEX:
    default:
        sprintf(string, "  ");
        break;
    }
}

static void format_data_ascii(char *string, unsigned char data)
{
    assert(string);

    sprintf(string, "%c", isprint((int)data) ? (char)data : (char)msg_option.ascii_subst);
}

/** @}
 */
/*  ----------------------------------------------------------------------
 *  Uwe Vogt,  UV Software,  Chausseestrasse 33 A,  10115 Berlin,  Germany
 *  Tel.: +49-30-46799872,  Fax: +49-30-46799873,  Mobile: +49-170-3801903
 *  E-Mail: uwe.vogt@uv-software.de,  Homepage: http://www.uv-software.de/
 */
