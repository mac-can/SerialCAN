#
#	CAN Monitor for CAN-over-Serial-Line Interfaces (CAN API V3)
#
#	Copyright (c) 2007 by Uwe Vogt, UV Software, Friedrichshafen
#	Copyright (c) 2012-2024  Uwe Vogt, UV Software, Berlin (info@uv-software.com)
#
#	This program is free software: you can redistribute it and/or modify
#	it under the terms of the GNU General Public License as published by
#	the Free Software Foundation, either version 3 of the License, or
#	(at your option) any later version.
#
#	This program is distributed in the hope that it will be useful,
#	but WITHOUT ANY WARRANTY; without even the implied warranty of
#	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#	GNU General Public License for more details.
#
#	You should have received a copy of the GNU General Public License
#	along with this program   If not, see <https://www.gnu.org/licenses/>.
#
current_OS := $(shell sh -c 'uname 2>/dev/null || echo Unknown OS')
current_OS := $(patsubst CYGWIN%,Cygwin,$(current_OS))
current_OS := $(patsubst MINGW%,MinGW,$(current_OS))
current_OS := $(patsubst MSYS%,MinGW,$(current_OS))


ifeq ($(current_OS),$(filter $(current_OS),Linux Darwin))
TARGET  = can_moni
else
TARGET  = can_moni.exe
endif
INSTALL = ~/bin

PROJ_DIR = ../..
HOME_DIR = .
MAIN_DIR = ./Sources

DRIVER_DIR = $(PROJ_DIR)/Sources
CANAPI_DIR = $(PROJ_DIR)/Sources/CANAPI

OBJECTS = $(OUTDIR)/main.o $(OUTDIR)/Options.o $(OUTDIR)/Timer.o \
	$(OUTDIR)/Message.o $(OUTDIR)/can_msg.o

DEFINES = -DOPTION_CANAPI_DRIVER=1 \
	-DOPTION_CANAPI_COMPANIONS=1

HEADERS = -I$(MAIN_DIR) \
	-I$(HOME_DIR) \
	-I$(DRIVER_DIR) \
	-I$(CANAPI_DIR)


ifeq ($(current_OS),Darwin)  # macOS - libSerialCAN.dylib

OBJECTS  += $(BINDIR)/libSerialCAN.a

CFLAGS += -O2 -Wall -Wextra -Wno-parentheses \
	-fno-strict-aliasing \
	$(DEFINES) \
	$(HEADERS)

CXXFLAGS += -O2 -g -Wall -Wextra -pthread \
	$(DEFINES) \
	$(HEADERS)

LDFLAGS  += -rpath /usr/local/lib

ifeq ($(BINARY),UNIVERSAL)
CFLAGS += -arch arm64 -arch x86_64
CXXFLAGS += -arch arm64 -arch x86_64
LDFLAGS += -arch arm64 -arch x86_64
endif

LIBRARIES = -lpthread

CXX = clang++
CC = clang
LD = clang++
endif

ifeq ($(current_OS),$(filter $(current_OS),Linux Cygwin))  # linux - libserialcan.so

OBJECTS  += $(BINDIR)/libserialcan.a

CFLAGS += -O2 -Wall -Wextra -Wno-parentheses \
	-fno-strict-aliasing \
	$(DEFINES) \
	$(HEADERS)

CXXFLAGS += -O2 -g -Wall -Wextra -pthread \
	$(DEFINES) \
	$(HEADERS)

LDFLAGS  +=

LIBRARIES = -lpthread

CXX = g++
CC = gcc
LD = g++
endif

RM = rm -f
CP = cp -f

OUTDIR = .objects
BINDIR = $(PROJ_DIR)/Binaries
INCDIR = $(PROJ_DIR)/Includes

.PHONY: info outdir bindir incdir


all: info outdir bindir incdir $(TARGET)

info:
	@echo $(CXX)" on "$(current_OS)
	@echo "target: "$(TARGET)
	@echo "install: "$(INSTALL)

outdir:
	@mkdir -p $(OUTDIR)

bindir:
	@mkdir -p $(BINDIR)

incdir:
	@mkdir -p $(INCDIR)

clean:
	@-$(RM) $(TARGET) $(OUTDIR)/*.o $(OUTDIR)/*.d

pristine:
	@-$(RM) $(TARGET) $(OUTDIR)/*.o $(OUTDIR)/*.d
	@-$(RM) $(BINDIR)/$(TARGET)

install:
	@echo "Copying binary file..."
	$(CP) $(TARGET) $(INSTALL)


$(OUTDIR)/main.o: $(MAIN_DIR)/main.cpp
	$(CXX) $(CXXFLAGS) -MMD -MF $*.d -o $@ -c $<

$(OUTDIR)/Options.o: $(MAIN_DIR)/Options_p.cpp
	$(CXX) $(CXXFLAGS) -MMD -MF $*.d -o $@ -c $<

$(OUTDIR)/Timer.o: $(MAIN_DIR)/Timer.cpp
	$(CXX) $(CXXFLAGS) -MMD -MF $*.d -o $@ -c $<

$(OUTDIR)/Message.o: $(MAIN_DIR)/Message.cpp
	$(CXX) $(CXXFLAGS) -MMD -MF $*.d -o $@ -c $<

$(OUTDIR)/can_msg.o: $(CANAPI_DIR)/can_msg.c
	$(CC) $(CFLAGS) -MMD -MF $*.d -o $@ -c $<


$(TARGET): $(OBJECTS)
	$(LD) $(LDFLAGS) -o $@ $(OBJECTS) $(LIBRARIES)
	$(CP) $(TARGET) $(BINDIR)
ifeq ($(current_OS),Darwin)
	@lipo -archs $@
endif
	@echo "\033[1mTarget '"$@"' successfully build\033[0m"
