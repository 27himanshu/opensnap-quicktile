# Define constants:
SRC_DIR=src
BUILD_DIR=build
BIN_DIR=bin
EXECUTABLE=opensnap-quicktile
OUTPUT=$(BIN_DIR)/$(EXECUTABLE)

# GNU C++ compiler config:
CC=g++
CFLAGS=-Wall -std=c++11
LDFLAGS =

# Sources and objects
CPP_SOURCES = $(wildcard $(SRC_DIR)/*.cpp)
OBJECTS = $(patsubst $(SRC_DIR)/%.cpp, $(BUILD_DIR)/%.o, $(CPP_SOURCES))

# Libraries used
LIBS=x11 gtk+-3.0 dbus-cxx-1.0 log4cpp

# Use LIBS with pkg-conf to get the appropriate flags
CFLAGS += `pkg-config --cflags $(LIBS)`
LDFLAGS += `pkg-config --libs $(LIBS)`

# Convenience method to ensure that directories exist before trying to use them
dir_guard=@mkdir -p $(@D)

# Make everything
all: $(OUTPUT)

# Make everything, but set debug flags when compiling
debug: CFLAGS += -DDEBUG=DEBUG -DLOG4CPP_FIX_ERROR_COLLISION=1 -g
debug: all

# Make objects
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(dir_guard)
	$(CC) $(CFLAGS) -c -o $@ $<

# Make binary
$(OUTPUT): $(OBJECTS)
	$(dir_guard)
	$(CC) $(LDFLAGS) -o $@ $^

# Clean up generated files
clean:
	rm -rf $(BUILD_DIR) $(BIN_DIR)
