# Compiler and flags
CXX      := g++
CXXFLAGS := -Wall -Wextra -std=c++17 -Iinclude -O2
BUILD_DIR := build

# Default target: builds the main application
all: $(BUILD_DIR)/app

# ------------------------------------------------------------------------------
# Main Application Build Rule
# ------------------------------------------------------------------------------
# Compiles main.cpp and all helper modules located in src/
$(BUILD_DIR)/app: src/main.cpp src/sensor.cpp src/motor.cpp
	@mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $^ -o $@

# ------------------------------------------------------------------------------
# Unit Test Build Rules (tests/)
# ------------------------------------------------------------------------------
# Example: Camera capture unit test
test_camera: tests/test_camera_capture.cpp src/camera.cpp
	@mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $^ -o $(BUILD_DIR)/test_camera

# Run all test targets
test_all: test_camera

# ------------------------------------------------------------------------------
# Cleanup Rule
# ------------------------------------------------------------------------------
# Removes all compiled binaries and object files from build/
clean:
	rm -rf $(BUILD_DIR)/*

.PHONY: all test_all clean