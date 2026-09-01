#ifndef PI_CAMERA_H
#ifndef PI_CAMERA_H
#define PI_CAMERA_H

#include <opencv2/opencv.hpp>

// Captures a grayscale frame directly from the Raspberry Pi camera using libcamera (Zero-copy).
// Returns an empty cv::Mat on failure.
cv::Mat captureGrayscaleDirect(int width = 1280, int height = 960);

#endif // PI_CAMERA_H