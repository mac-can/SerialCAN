current_OS := $(shell sh -c 'uname 2>/dev/null || echo Unknown OS')
current_OS := $(patsubst CYGWIN%,Cygwin,$(current_OS))
current_OS := $(patsubst MINGW%,MinGW,$(current_OS))
current_OS := $(patsubst MSYS%,MinGW,$(current_OS))

BINARIES = ../../Binaries
INCLUDES = -I../../Includes

TARGET1 = can_recv
ifeq ($(current_OS),Darwin) # macOS - libSerial.dylib
SOURCE1 = $(TARGET1).cpp $(BINARIES)/libSerialCAN.a
endif
ifeq ($(current_OS),$(filter $(current_OS),Linux Cygwin)) # linux - libserialcan.so.1
SOURCE1 = $(TARGET1).cpp $(BINARIES)/libserialcan.a
endif
DEFINES1 = -DBAUDRATE=CANBTR_INDEX_250K

TARGET2 = can_send
ifeq ($(current_OS),Darwin) # macOS - libUVCANSLC.dylib
SOURCE2 = $(TARGET2).cpp $(BINARIES)/libUVCANSLC.a
endif
ifeq ($(current_OS),$(filter $(current_OS),Linux Cygwin)) # linux - libuvcanslc.so.1
SOURCE2 = $(TARGET2).cpp $(BINARIES)/libuvcanslc.a
endif
DEFINES2 = -DBAUDRATE=CANBTR_INDEX_250K

CXXFLAGS = -pthread
LDFLAGS =

CXX = g++
LD = g++
RM = rm -f

all: $(TARGET1) $(TARGET2)

$(TARGET1):	$(SOURCE1)
	$(CXX) $(SOURCE1) $(DEFINES1) $(INCLUDES) $(CXXFLAGS) $(LDFLAGS) -o $(TARGET1)

$(TARGET2):	$(SOURCE2)
	$(CXX) $(SOURCE2) $(DEFINES2) $(INCLUDES) $(CXXFLAGS) $(LDFLAGS) -o $(TARGET2)

clean:
	@-$(RM) *.o *~ $(TARGET1) $(TARGET2)
