current_OS := $(shell sh -c 'uname 2>/dev/null || echo Unknown OS')
current_OS := $(patsubst CYGWIN%,Cygwin,$(current_OS))
current_OS := $(patsubst MINGW%,MinGW,$(current_OS))
current_OS := $(patsubst MSYS%,MinGW,$(current_OS))

TARGET1 = can_recv
SOURCE1 = $(TARGET1).cpp
DEFINES1 = -DBAUDRATE=CANBTR_INDEX_250K
ifeq ($(current_OS),Darwin) # macOS - libSerial.dylib
LIBRARY1 = SerialCAN
else ifeq ($(current_OS),Linux) # linux - libserialcan.so.1
LIBRARY1 = serialcan
endif

TARGET2 = can_send
SOURCE2 = $(TARGET2).cpp
DEFINES2 = -DBAUDRATE=CANBTR_INDEX_250K
ifeq ($(current_OS),Darwin) # macOS - libUVCANSLC.dylib
LIBRARY2 = UVCANSLC
else ifeq ($(current_OS),Linux) # linux - libuvcanslc.so.1
LIBRARY2 = uvcanslc
endif

CXXFLAGS = -pthread
LDFLAGS =

CXX = g++
LD = g++
RM = rm -f

all: $(TARGET1) $(TARGET2)

$(TARGET1):	$(SOURCE1)
	$(CXX) $(SOURCE1) $(DEFINES1) $(CXXFLAGS) -l $(LIBRARY1) $(LDFLAGS) -o $(TARGET1)

$(TARGET2):	$(SOURCE2)
	$(CXX) $(SOURCE2) $(DEFINES2) $(CXXFLAGS) -l $(LIBRARY2) $(LDFLAGS) -o $(TARGET2)

clean:
	@-$(RM) *.o *~ $(TARGET1) $(TARGET2)
