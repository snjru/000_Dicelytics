#pragma once

#include <opencv2/opench.hpp>
#include <string>

/** 
 * Capture a still image using rpicam-still and load it into an OpenCV Mat as grayscale.
 * 
 * @param outputPath    Temporary file path to save the captured image (e.g. "/tmp/capture.jpg")
 * @param width         Capture width in pixels
 * @param height        Capture height in pixels
 * @param timeoutMs     Camera warm-up/capture timeout in milliseconds
 * @return              Grayscale cv::Mat (CV_8UC1). Empty Mat on filure.
*/

cv::Mat captureGrayscaleImage(const std::string& outputPath,
                                int width = 1280,
                                int height = 960,
                                int timeoutMs = 1000);