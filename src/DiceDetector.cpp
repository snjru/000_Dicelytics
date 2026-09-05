#include "DiceDetector.hpp"

DiceDetector::DiceDetector(int targetHeight)
	: m_targetHeight(targetHeight) {
}

DiceDetector::~DiceDetector() {
}

cv::Mat DiceDetector::resize(const cv::Mat& inputImg) const {
	if (inputImg.empty()) {
		return cv::Mat();
	}

	int currentHeight = inputImg.rows;
	int currentWidth = inputImg.cols;

	double scale = static_cast<double>(m_targetHeight) / currentHeight;
	
	int newHeight = m_targetHeight;
	int newWidth = static_cast<int>(currentWidth * scale);

	cv::Mat resizedImg;
	cv::resize(inputImg, resizedImg, cv::Size(newWidth, newHeight));

	return resizedImg;
}

cv::Mat DiceDetector::preprocess(const cv::Mat& inputImg) const {
	if (inputImg.empty()) {
		return cv::Mat();
	}

	cv::Mat grayImg, blurImg;
	cv::cvtColor(inputImg, grayImg, cv::COLOR_BGR2GRAY);
	cv::GaussianBlur(grayImg, blurImg, cv::Size(5,5), 0);

	return blurImg;
}