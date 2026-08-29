CXX = g++
CXXFLAGS = -std=c++17 -Wall -Iinclude
OPENCV_FLAGS = $(shell pkg-config --cflags --libs opencv4)

test_camera_capture: src/camera_capture.cpp tests/test_camera_capture.cpp
	$(CXX) $(CXXFLAGS) src/camera_capture.cpp tests/test_camera_capture.cpp -o test_camera_capture $(OPENCV_FLAGS)

clean:
	rm -f test_camera_capture