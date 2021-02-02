#
#	SerialCAN - CAN-over-Serial-Line Interfaces
#
#	Copyright (C) 2007,2016-2021  Uwe Vogt, UV Software, Berlin (info@uv-software.com)
#
#	This file is part of SerialCAN.
#
#	SerialCAN is free software: you can redistribute it and/or modify
#	it under the terms of the GNU Lesser General Public License as published by
#	the Free Software Foundation, either version 3 of the License, or
#	(at your option) any later version.
#
#	SerialCAN is distributed in the hope that it will be useful,
#	but WITHOUT ANY WARRANTY; without even the implied warranty of
#	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#	GNU Lesser General Public License for more details.
#
#	You should have received a copy of the GNU Lesser General Public License
#	along with SerialCAN.  If not, see <http://www.gnu.org/licenses/>.
#

all:
	@./build_no.sh
	@echo "Building SerialCAN (build "$(shell git log -1 --pretty=format:%h)")..."
	$(MAKE) -C Libraries/SerialCAN $@
	$(MAKE) -C Libraries/CANAPI $@
	$(MAKE) -C Libraries/SLCAN $@
	$(MAKE) -C Utilities/can_test $@
	$(MAKE) -C Utilities/can_moni $@
	$(MAKE) -C Trial $@

clean:
	$(MAKE) -C Libraries/SerialCAN $@
	$(MAKE) -C Libraries/CANAPI $@
	$(MAKE) -C Libraries/SLCAN $@
	$(MAKE) -C Utilities/can_test $@
	$(MAKE) -C Utilities/can_moni $@
	$(MAKE) -C Trial $@

install:
	$(MAKE) -C Libraries/SerialCAN $@
	$(MAKE) -C Libraries/CANAPI $@
	$(MAKE) -C Libraries/SLCAN $@
#	$(MAKE) -C Utilities/can_test $@
#	$(MAKE) -C Utilities/can_moni $@

build_no:
	@./build_no.sh
	@cat Sources/build_no.h
