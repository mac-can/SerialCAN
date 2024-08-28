#
#	Trial Program
#	SerialCAN (SLCAN protocol)
#	Bart Simpson didn't do it
#
current_OS := $(shell sh -c 'uname 2>/dev/null || echo Unknown OS')
current_OS := $(patsubst CYGWIN%,Cygwin,$(current_OS))
current_OS := $(patsubst MINGW%,MinGW,$(current_OS))
current_OS := $(patsubst MSYS%,MinGW,$(current_OS))


ifeq ($(current_OS),$(filter $(current_OS),Linux Darwin))
TARGET  = slc_test
else
TARGET  = slc_test.exe
endif

INSTALL = ~/bin

HOME_DIR = ..
MAIN_DIR = ./Sources

SOURCE_DIR = $(HOME_DIR)/Sources
SERIAL_DIR = $(HOME_DIR)/Sources/SLCAN
CANAPI_DIR = $(HOME_DIR)/Sources/CANAPI
WRAPPER_DIR = $(HOME_DIR)/Sources/Wrapper

OBJECTS = $(OUTDIR)/SerialCAN.o \
	$(OUTDIR)/can_api.o $(OUTDIR)/can_btr.o \
	$(OUTDIR)/slcan.o $(OUTDIR)/serial.o \
	$(OUTDIR)/buffer.o $(OUTDIR)/queue.o \
	$(OUTDIR)/timer.o $(OUTDIR)/logger.o \
	$(OUTDIR)/main.o

DEFINES = -DOPTION_CAN_2_0_ONLY=0 \
	-DOPTION_CANAPI_DRIVER=1 \
	-DOPTION_CANAPI_RETVALS=0 \
	-DOPTION_CANAPI_COMPANIONS=1 \
	-DOPTION_CANAPI_DEBUG_LEVEL=0 \
	-DOPTION_SERIAL_DEBUG_LEVEL=3 \
	-DOPTION_SLCAN_FAKE_COMMANDS=1 \
	-DOPTION_SLCAN_DEBUG_LEVEL=1

HEADERS = -I$(SOURCE_DIR) \
	-I$(SERIAL_DIR) \
	-I$(CANAPI_DIR) \
	-I$(WRAPPER_DIR) \
	-I$(MAIN_DIR)


ifeq ($(current_OS),Darwin)  # macOS - libSerialCAN.dylib

CFLAGS += -O0 -g -Wall -Wextra -Wno-parentheses \
	-fmessage-length=0 -fno-strict-aliasing \
	$(DEFINES) \
	$(HEADERS)

CXXFLAGS += -O0 -g -Wall -Wextra -pthread \
	$(DEFINES) \
	$(HEADERS)

LDFLAGS  += 

ifeq ($(BINARY),UNIVERSAL)
CFLAGS += -arch arm64 -arch x86_64
CXXFLAGS += -arch arm64 -arch x86_64
LDFLAGS += -arch arm64 -arch x86_64
endif

LIBRARIES = -lpthread

CHECKER  = warning,information
IGNORE   = -i serial_w.c -i buffer_w.c -i queue_w.c -i logger_w.c -i can_msg.c -i can_dev.c -i vanilla.c
ifeq ($(HUNTER),BUGS)
CHECKER += --bug-hunting
endif

ifeq ($(XCTEST),FULL)
TESTING =
else
TESTING = -only-testing Testing/Testing/testSmokeTest
endif

CXX = clang++
CC = clang
LD = clang++
endif

ifeq ($(current_OS),$(filter $(current_OS),Linux Cygwin))  # Linux, Cygwin - libserialcan.so

CFLAGS += -O0 -g -Wall -Wextra -Wno-parentheses \
	-fmessage-length=0 -fno-strict-aliasing \
	$(DEFINES) \
	$(HEADERS)

CXXFLAGS += -O0 -g -Wall -Wextra -pthread \
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


.PHONY: info outdir


all: info outdir $(TARGET)

info:
	@echo $(CXX)" on "$(current_OS)
	@echo "target: "$(TARGET)
	@echo "install: "$(INSTALL)

outdir:
	@mkdir -p $(OUTDIR)

check:
ifeq ($(current_OS),Darwin)  # macOS - libSerialCAN.dylib
	cppcheck --enable=$(CHECKER) --suppressions-list=$(HOME_DIR)/suppress.txt \
	-D__APPLE__ $(DEFINES) $(HEADERS) $(IGNORE) $(SOURCE_DIR)
else
	echo "Please use Cppcheck GUI"
endif

clean:
	@-$(RM) $(TARGET) $(OUTDIR)/*.o $(OUTDIR)/*.d

pristine:
	@-$(RM) $(TARGET) $(OUTDIR)/*.o $(OUTDIR)/*.d

install:
	$(CP) $(TARGET) $(INSTALL)

test: info outdir $(TARGET)
	./$(TARGET) INFO EXIT

xctest:
	xcodebuild clean build test -project SerialCAN.xcodeproj -scheme Testing $(TESTING)


$(OUTDIR)/main.o: $(MAIN_DIR)/main.cpp
	$(CXX) $(CXXFLAGS) -MMD -MF $*.d -o $@ -c $<

$(OUTDIR)/SerialCAN.o: $(SOURCE_DIR)/SerialCAN.cpp
	$(CXX) $(CXXFLAGS) -MMD -MF $*.d -o $@ -c $<

$(OUTDIR)/can_api.o: $(WRAPPER_DIR)/can_api.c
	$(CC) $(CFLAGS) -MMD -MF $*.d -o $@ -c $<

$(OUTDIR)/can_btr.o: $(CANAPI_DIR)/can_btr.c
	$(CC) $(CFLAGS) -MMD -MF $*.d -o $@ -c $<

$(OUTDIR)/slcan.o: $(SERIAL_DIR)/slcan.c
	$(CC) $(CFLAGS) -MMD -MF $*.d -o $@ -c $<

$(OUTDIR)/serial.o: $(SERIAL_DIR)/serial.c $(SERIAL_DIR)/serial_p.c
	$(CC) $(CFLAGS) -MMD -MF $*.d -o $@ -c $<

$(OUTDIR)/buffer.o: $(SERIAL_DIR)/buffer.c $(SERIAL_DIR)/buffer_p.c
	$(CC) $(CFLAGS) -MMD -MF $*.d -o $@ -c $<

$(OUTDIR)/queue.o: $(SERIAL_DIR)/queue.c $(SERIAL_DIR)/queue_p.c
	$(CC) $(CFLAGS) -MMD -MF $*.d -o $@ -c $<

$(OUTDIR)/timer.o: $(SERIAL_DIR)/timer.c $(SERIAL_DIR)/timer_p.c
	$(CC) $(CFLAGS) -MMD -MF $*.d -o $@ -c $<

$(OUTDIR)/logger.o: $(SERIAL_DIR)/logger.c $(SERIAL_DIR)/logger_p.c
	$(CC) $(CFLAGS) -MMD -MF $*.d -o $@ -c $<


$(TARGET): $(OBJECTS)
	$(LD) $(LDFLAGS) -o $@ $(OBJECTS) $(LIBRARIES)
ifeq ($(current_OS),Darwin)
	@lipo -archs $@
endif
	@echo "\033[1mTarget '"$@"' successfully build\033[0m"
