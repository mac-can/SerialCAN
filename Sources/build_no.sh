#!/bin/sh
echo "/*  -- Do not commit this file --" > build_no.h
echo " *" >> build_no.h
echo " *  SerialCAN - CAN API V3 Driver for CAN over Serial-line Interfaces" >> build_no.h
echo " *" >> build_no.h
echo " *  Copyright (C) 2020  Uwe Vogt, UV Software, Berlin (info@uv-software.com)" >> build_no.h
echo " *" >> build_no.h
echo " *  This file is part of SerialCAN." >> build_no.h
echo " *" >> build_no.h
echo " *  SerialCAN is free software : you can redistribute it and/or modify" >> build_no.h
echo " *  it under the terms of the GNU General Public License as published by" >> build_no.h
echo " *  the Free Software Foundation, either version 3 of the License, or" >> build_no.h
echo " *  (at your option) any later version." >> build_no.h
echo " *" >> build_no.h
echo " *  SerialCAN is distributed in the hope that it will be useful," >> build_no.h
echo " *  but WITHOUT ANY WARRANTY; without even the implied warranty of" >> build_no.h
echo " *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the" >> build_no.h
echo " *  GNU General Public License for more details." >> build_no.h
echo " *" >> build_no.h
echo " *  You should have received a copy of the GNU General Public License" >> build_no.h
echo " *  along with SerialCAN.  If not, see <http://www.gnu.org/licenses/>." >> build_no.h
echo " */" >> build_no.h
echo "#ifndef BUILD_NO_H_INCLUDED" >> build_no.h
echo "#define BUILD_NO_H_INCLUDED" >> build_no.h
echo "#define BUILD_NO 0x"$(git log -1 --pretty=format:%h) >> build_no.h
echo "#define STRINGIFY(X) #X" >> build_no.h
echo "#define TOSTRING(X) STRINGIFY(X)" >> build_no.h
echo "#endif" >> build_no.h
