#include "camera_capture.h"
#include <iostream>

int main()
{
    cv::Mat grayImg = captureGrayscaleImage("/tmp/test_capture.jpg");

    if (grayImg.empyt()) {
        std::cerr << "TEST FAILED: capture returned empty Mat." << std::endl;
        return 1;
    }

    std::cout << "TEST OK: captured " << grayImg.cols << "x" << grayImg.rows
              << ", type=" << grayImg.type() << std::endl;

    cv::imwrite("/tmp/test_capture_gray.png", grayImg);
    std::cout << "Saved debug output to /tmp/test_capture_gray.png" << std::endl;

    return 0;
}
