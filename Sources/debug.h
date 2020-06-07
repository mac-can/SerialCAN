/*
 *  Controler Area Network - Lawicel SLCAN Protocol (CAN over Serial Line)
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
#ifndef DEBUG_H_INCLUDED
#define DEBUG_H_INCLUDED

#include "logger.h"

#if (OPTION_SLCAN_DEBUG_LEVEL > 0) || (OPTION_SERIAL_DEBUG_LEVEL > 0)
#define LOGGER_INIT(fn)  log_init(fn)
#define LOGGER_EXIT()  log_exit()
#if defined(_WIN32) || defined(_WIN64)
#define LOGGER_KILL()  log_kill()
#endif
#else
#define LOGGER_INIT(fn)  while (0)
#define LOGGER_EXIT()  while (0)
#if defined(_WIN32) || defined(_WIN64)
#define LOGGER_KILL()  while (0)
#endif
#endif

#endif /* DEBUG_H_INCLUDED */
