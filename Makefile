# Compiler and Flags
CXX      := g++
CXXFLAGS := -Wall -O2 -Iinclude
LIBS     := -lgpiodcxx -lgpiod -lrt -lpthread

# Directories
SRC_DIR  := src
TEST_DIR := test
BIN_DIR  := bin

# Sources
COMMON_SRCS := $(wildcard $(SRC_DIR)/*.cpp)
TEST_SRCS   := $(wildcard $(TEST_DIR)/*.cpp)

# Generate target executable names from test files
TARGETS     := $(patsubst $(TEST_DIR)/%.cpp, $(BIN_DIR)/%, $(TEST_SRCS))

# Default target: build all tests
all: $(TARGETS)

# Rule to build each test executable
$(BIN_DIR)/%: $(TEST_DIR)/%.cpp $(COMMON_SRCS)
	@mkdir -p $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $< $(COMMON_SRCS) -o $@ $(LIBS)

# Clean up built binaries
clean:
	rm -rf $(BIN_DIR)

.PHONY: all clean