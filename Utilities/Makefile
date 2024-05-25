#
#	CAN Utilities for generic CAN Interfaces (CAN API V3)
#
#	Copyright (c) 2005-2010 Uwe Vogt, UV Software, Friedrichshafen
#	Copyright (c) 2012-2024 Uwe Vogt, UV Software, Berlin (info@mac-can.com)
#
#	These utilities are free software: you can redistribute it and/or modify
#	it under the terms of the GNU General Public License as published by
#	the Free Software Foundation, either version 3 of the License, or
#	(at your option) any later version.
#
#	These utilities is distributed in the hope that it will be useful,
#	but WITHOUT ANY WARRANTY; without even the implied warranty of
#	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#	GNU General Public License for more details.
#
#	You should have received a copy of the GNU General Public License
#	along with these utilities.  If not, see <https://www.gnu.org/licenses/>.
#
all:
	@echo "\033[1mBuilding my beloved CAN Utilities...\033[0m"
	$(MAKE) -C can_test $@
	$(MAKE) -C can_moni $@

clean:
	$(MAKE) -C can_test $@
	$(MAKE) -C can_moni $@

pristine:
	$(MAKE) -C can_test $@
	$(MAKE) -C can_moni $@

install:
#	$(MAKE) -C can_test $@
#	$(MAKE) -C can_moni $@
