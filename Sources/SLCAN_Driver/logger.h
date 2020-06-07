/*
 *  Software for Industrial Communication, Motion Control, and Automation
 *
 *  Copyright (C) 2016,2020  Uwe Vogt, UV Software, Berlin (info@uv-software.com)
 *
 *  This module is part of the SourceMedley repository.
 *
 *  This module is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This module is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this module.  If not, see <http://www.gnu.org/licenses/>.
 */
/** @file        logger.h
 *
 *  @brief       Writing log messages into an ASCII file.
 *
 *  @author      $Author: haumea $
 *
 *  @version     $Rev: 668 $
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

/** @brief TBD
 *  @param[in]   pathname - ...
 *  @returns     0 if successful, or a negative value on error
 */
extern int log_init(const char *pathname/* = NULL, flags = 0*/);


/** @brief TBD
 *  @returns     0 if successful, or a negative value on error
 */
extern int log_exit(void);


/** @brief TBD
 *  @param[in]   buffer - pointer to a buffer ...
 *  @param[in]   nbytes - number of bytes to be logged
 *  @returns     0 if successful, or a negative value on error
 */
extern int log_sync(const uint8_t *buffer, size_t nbytes);


/** @brief TBD
 *  @param[in]   buffer - pointer to a buffer ...
 *  @param[in]   nbytes - number of bytes to be logged
 *  @returns     0 if successful, or a negative value on error
 */
extern int log_async(const uint8_t *buffer, size_t nbytes);


/** @brief TBD
 *  @param[in]   format - format string (see printf)
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
