#
#	SerialCAN - CAN-over-Serial-Line Interfaces (for GNU C/C++ compiler)
#
#	Copyright (C) 2007,2016-2021  Uwe Vogt, UV Software, Berlin (info@uv-software.com)
#
#	This library is part of SerialCAN.
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
current_OS := $(shell sh -c 'uname 2>/dev/null || echo Unknown OS')
current_OS := $(patsubst CYGWIN%,Cygwin,$(current_OS))
current_OS := $(patsubst MINGW%,MinGW,$(current_OS))
current_OS := $(patsubst MSYS%,MinGW,$(current_OS))

MAJOR = 0
MINOR = 1
PATCH = 0

HOME_DIR = ../..

SOURCE_DIR = $(HOME_DIR)/Sources
CANAPI_DIR = $(HOME_DIR)/Sources/CANAPI
SERIAL_DIR = $(HOME_DIR)/Sources/SLCAN_Driver

ifeq ($(current_OS),Darwin) # macOS - libUVCANSLC.dylib

LIBRARY = libSerialCAN

VERSION = $(MAJOR).$(MINOR)
TARGET  = $(LIBRARY).$(VERSION).dylib

INSTALL_NAME = $(TARGET)
CURRENT_VERSION = $(MAJOR).$(MINOR).0
COMPATIBILITY_VERSION = $(MAJOR).$(MINOR).0

INSTALL = /usr/local/lib

INCLUDE = /usr/local/include

DEFINES = -DOPTION_CAN_2_0_ONLY=0 \
	-DOPTION_SERIALCAN_DYLIB=1 \
	-DOPTION_CANAPI_DRIVER=1 \
	-DOPTION_CANAPI_RETVALS=0 \
	-DOPTION_CANAPI_COMPANIONS=1 \
	-DOPTION_CANAPI_DEBUG_LEVEL=0 \
	-DOPTION_SERIAL_DEBUG_LEVEL=0 \
	-DOPTION_SLCAN_DEBUG_LEVEL=0

HEADERS = -I$(SOURCE_DIR) \
	-I$(CANAPI_DIR) \
	-I$(SERIAL_DIR)

CFLAGS += -O2 -Wall -Wno-parentheses \
	-fno-strict-aliasing \
	$(DEFINES) \
	$(HEADERS)

CXXFLAGS += -g -Wall -Wextra -pthread \
	$(DEFINES) \
	$(HEADERS)

LIBRARIES =

LDFLAGS  += -nostartfiles -dynamiclib -fvisibility=hidden \
	-install_name $(INSTALL_NAME) \
	-current_version $(CURRENT_VERSION) \
	-compatibility_version $(COMPATIBILITY_VERSION)

endif

ifeq ($(current_OS),Linux) # linux - libuvcan300.so.1

LIBRARY = libserialcan

VERSION = $(MAJOR).$(MINOR)

SONAME  = $(LIBRARY).so.1

TARGET  = $(SONAME).$(VERSION)

INSTALL = /usr/local/lib

INCLUDE = /usr/local/include

DEFINES = -DOPTION_CAN_2_0_ONLY=0 \
	-DOPTION_CANAPI_DRIVER=1 \
	-DOPTION_CANAPI_RETVALS=0 \
	-DOPTION_CANAPI_COMPANIONS=1 \
	-DOPTION_CANAPI_DEBUG_LEVEL=0 \
	-DOPTION_SERIAL_DEBUG_LEVEL=0 \
	-DOPTION_SLCAN_DEBUG_LEVEL=0

HEADERS = -I$(SOURCE_DIR) \
	-I$(CANAPI_DIR) \
	-I$(SERIAL_DIR)

CFLAGS += -fPIC -O2 -Wall -Wno-parentheses \
	-fno-strict-aliasing \
	$(DEFINES) \
	$(HEADERS)

CXXFLAGS += -fPIC -g -Wall -Wextra -pthread \
	$(DEFINES) \
	$(HEADERS)

LIBRARIES =

LDFLAGS  += --shared -Wl,-soname,$(SONAME) -lc

endif

OBJECTS = $(OUTDIR)/SerialCAN.o $(OUTDIR)/can_btr.o \
	$(OUTDIR)/slcan.o $(OUTDIR)/serial.o \
	$(OUTDIR)/buffer.o $(OUTDIR)/queue.o $(OUTDIR)/logger.o

CXX = g++
CC = gcc
LD = g++
RM = rm -f
CP = cp -f

OUTDIR = .objects


.PHONY: info outdir


all: info outdir $(TARGET)

info:
	@echo $(CXX)" on "$(current_OS)
	@echo "Target: "$(TARGET)
	@echo "install: "$(INSTALL)

outdir:
	@mkdir -p $(OUTDIR)

clean:
	$(RM) $(TARGET) $(OUTDIR)/*.o $(OUTDIR)/*.d

install:
	@echo "Copying library file..."
	$(CP) $(TARGET) $(INSTALL)
ifeq ($(current_OS),Darwin)
	$(RM) $(INSTALL)/$(LIBRARY).dylib ; ln -s $(INSTALL)/$(TARGET) $(INSTALL)/$(LIBRARY).dylib
endif
ifeq ($(current_OS),Linux)
	$(RM) $(INSTALL)/$(SONAME) ; ln -s $(INSTALL)/$(TARGET) $(INSTALL)/$(SONAME)
	$(RM) $(INSTALL)/$(LIBRARY).so ; ln -s $(INSTALL)/$(SONAME) $(INSTALL)/$(LIBRARY).so
endif
	@echo "Copying header files..."
	$(CP) $(SOURCE_DIR)/SerialCAN.h $(INCLUDE)
	$(CP) $(CANAPI_DIR)/CANAPI.h $(INCLUDE)
	$(CP) $(CANAPI_DIR)/CANAPI_Types.h $(INCLUDE)
	$(CP) $(CANAPI_DIR)/CANAPI_Defines.h $(INCLUDE)
	$(CP) $(CANAPI_DIR)/CANAPI_SerialCAN.h $(INCLUDE)


$(OUTDIR)/SerialCAN.o: $(SOURCE_DIR)/SerialCAN.cpp
	$(CXX) $(CXXFLAGS) -MMD -MF $*.d -o $@ -c $<

$(OUTDIR)/slcan.o: $(SERIAL_DIR)/slcan.c
	$(CC) $(CFLAGS) -MMD -MF $*.d -o $@ -c $<

$(OUTDIR)/serial.o: $(SERIAL_DIR)/serial.c $(SERIAL_DIR)/serial_p.c
	$(CC) $(CFLAGS) -MMD -MF $*.d -o $@ -c $<

$(OUTDIR)/buffer.o: $(SERIAL_DIR)/buffer.c $(SERIAL_DIR)/buffer_p.c
	$(CC) $(CFLAGS) -MMD -MF $*.d -o $@ -c $<

$(OUTDIR)/queue.o: $(SERIAL_DIR)/queue.c $(SERIAL_DIR)/queue_p.c
	$(CC) $(CFLAGS) -MMD -MF $*.d -o $@ -c $<

$(OUTDIR)/logger.o: $(SERIAL_DIR)/logger.c $(SERIAL_DIR)/logger_p.c
	$(CC) $(CFLAGS) -MMD -MF $*.d -o $@ -c $<

$(OUTDIR)/can_btr.o: $(CANAPI_DIR)/can_btr.c
	$(CC) $(CFLAGS) -MMD -MF $*.d -o $@ -c $<


$(TARGET): $(OBJECTS)
	$(LD) $(LDFLAGS) -o $@ $(OBJECTS) $(LIBRARIES)
	@echo "Target '"$@"' successfully build"
