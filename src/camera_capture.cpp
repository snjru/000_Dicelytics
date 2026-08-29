#include "camera_capture.h"

#include <cstdlib>
#include <sstream>
#include <iostream>

cv::Mat captureGrayscaleImage(const std::string& outputPath,
                                int width = 1280,
                                int height = 960,
                                int timeoutMs = 1000)
{
    // Build the rpicam-still command.
    // -n : no preview window
    // -t : timeout before capture (ms)
    std::ostringstream cmd;
    cmd << "rpicam-still -n"
        << " -o " << outputPath
        << " --width " << width
        << " --height " << height
        << " -t " << timeoutMs;

    int ret = std::system(cmd.str().c_str());
    if (ret != 0) {
        std::cerr << "[captureGrayscaleImage] rpicam-still failed (exit code " << ret << ")" << std::endl;
        return cv::Mat();
    }

    // Load directly as grayscale
    cv::Mat gray = cv::imread(outputPath, cv::IMREAD_GRAYSCALE);
    if (gray.empty()) {
        std::cerr << "[captureGrayscaleImage] Failed to load image from " << outputPath << std::endl;
    }

    return gray;
}