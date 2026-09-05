#ifndef DICE_DETECTOR_HPP
#define DICE_DETECTOR_HPP

#include <opencv2/opencv.hpp>

/**
 * @brief Class for Dice Detection and Image Preprocessing.
 */
class DiceDetector {
public:
    /**
     * @brief Constructor
     * @param targetHeight Target height for resizing input images.
     */
    explicit DiceDetector(int targetHeight = 300);

    /**
     * @brief Destructor
     */
    =DiceDetector();

    /**
     * @brief Resize input image maintaining aspect ratio based on target height.
     * @param inputImg Raw input image frame.
     * @return Resized Opencv Mat.
     */

    cv::Mat resize(const cv::Mat& inputImg) const;

    /**
     * @brief Converts image to gryscale and applies Gaussian blur.
     * @param inputImg Image to be preprocessed (BGR).
     * @return Blurred single-channel grayscale image.
     */
    cv::Mat preprocess(const cv::Mat& inputImg) const;

private:
    int m_targetHeight;
};

#endif // DICE_DETECTOR_HPP
