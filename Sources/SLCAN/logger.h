/*  SPDX-License-Identifier: BSD-2-Clause OR GPL-3.0-or-later */
/*
 *  Software for Industrial Communication, Motion Control and Automation
 *
 *  Copyright (c) 2002-2024 Uwe Vogt, UV Software, Berlin (info@uv-software.com)
 *  All rights reserved.
 *
 *  Module 'logger'
 *
 *  This module is dual-licensed under the BSD 2-Clause "Simplified" License
 *  and under the GNU General Public License v3.0 (or any later version).
 *  You can choose between one of them if you use this module.
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
 *  THIS MODULE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 *  DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 *  FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 *  DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 *  SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 *  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 *  OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *  OF THIS MODULE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *  GNU General Public License v3.0 or later:
 *  This module is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This module is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this module.  If not, see <https://www.gnu.org/licenses/>.
 */
/** @file        logger.h
 *
 *  @brief       Writing log messages into an ASCII file.
 *
 *  @author      $Author: quaoar $
 *
 *  @version     $Rev: 811 $
 *
 *  @defgroup    logger Logging into a file
 *  @{
 */
#ifndef LOGGER_H_INCLUDED
#define LOGGER_H_INCLUDED

#include <stdio.h>
#include <stdint.h>


/*  -----------  options  ------------------------------------------------
 */


/*  -----------  defines  ------------------------------------------------
 */


/*  -----------  types  --------------------------------------------------
 */


/*  -----------  variables  ----------------------------------------------
 */


/*  -----------  prototypes  ---------------------------------------------
 */
#ifdef __cplusplus
extern "C" {
#endif

/** @brief       opens a file for logging.
 * 
 *  @param[in]   pathname - a pathname naming a file, or NULL
 *                          to write to stadard output stream
 * 
 *  @returns     0 if successful, or a negative value on error
 */
extern int log_init(const char *pathname/* = NULL, flags = 0*/);


/** @brief       closes a open log file.
 * 
 *  @returns     0 if successful, or a negative value on error
 */
extern int log_exit(void);


/** @brief       writes binary data as ASCII into the log file.
 *
 *  @remarks     This function can only be used by the log file
 *               owner (e.g. the thread that opened the file).
 * 
 *  @param[in]   buffer - pointer to a buffer containing binary
 *                        data to be written into the log file
 *  @param[in]   nbytes - number of bytes to be logged
 * 
 *  @returns     0 if successful, or a negative value on error
 */
extern int log_sync(const uint8_t *buffer, size_t nbytes);


/** @brief       writes binary data as ASCII into the log file.
 * 
 *  @remarks     This function can be used by threads that do
 *               not own the log file (e.g. callbak handler).
 * 
 *  @param[in]   buffer - pointer to a buffer containing binary
 *                        data to be written into the log file
 *  @param[in]   nbytes - number of bytes to be logged
 * 
 *  @returns     0 if successful, or a negative value on error
 */
extern int log_async(const uint8_t *buffer, size_t nbytes);


/** @brief       writes a formatted string into the log file.
 * 
 *  @remarks     This function can only be used by the log file
 *               owner (e.g. the thread that opened the file).
 * 
 *  @param[in]   format - format string (see printf)
 * 
 *  @returns     0 if successful, or a negative value on error
 */
extern int log_printf(const char *format, ...);


#ifdef __cplusplus
}
#endif
#endif /* LOGGER_H_INCLUDED */

/*  ----------------------------------------------------------------------
 *  Uwe Vogt,  UV Software,  Chausseestrasse 33 A,  10115 Berlin,  Germany
 *  Tel.: +49-30-46799872,  Fax: +49-30-46799873,  Mobile: +49-170-3801903
 *  E-Mail: uwe.vogt@uv-software.de,  Homepage: http://www.uv-software.de/
 */
