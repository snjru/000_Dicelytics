#include <iostream>
#include <string>
#include <windows.h>
#include <commdlg.h>
#include <opencv2/opencv.hpp>
#include "DiceDetector.hpp"

using namespace std;

/**
 * @brief Opens Windows file dialog to select an image file.
 * @return Selected file path string, or empty string if canceled.
 */
string openFileDialog() {
    OPENFILENAME ofn;
    char fileName[MAX_PATH] = "";

    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = NULL;
    ofn.lpstrFilter = "Image Files\0*.jpg;*.png;*.bmp;*.jpeg\0All Files\0*.*\0";
    ofn.lpstrFile = fileName;
    ofn.nMaxFile = MAX_PATH;
    ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST;

    if (GetOpenFileName(&ofn)) {
        return string(fileName);
    }
    return "";
}

/**
 * @brief Test application entry point.
 */
int main() {
    // 1. Select image file via file dialog
    string filepath = openFileDialog();
    if (filepath.empty()) {
        cout << "[Test] No image file was selected." << endl;
        return 0;
    }

    // 2. Load the input image
    cv::Mat inputImg = cv::imread(filepath);
    if (inputImg.empty()) {
        cerr << "[Test Error] Failed to open image: " << filepath << endl;
        return -1;
    }

    cout << "[Test] Image loaded successfully." << endl;
    cout << "  - Original Size: " << inputImg.cols << " x " << inputImg.rows << endl;

    // 3. Instantiate DiceDetector (target height set to 300px)
    const int TARGET_HEIGHT = 300;
    DiceDetector detector(TARGET_HEIGHT);

    // 4. Test resize() member function
    cv::Mat resizedImg = detector.resize(inputImg);
    if (resizedImg.empty()) {
        cerr << "[Test Error] resize() returned an empty image." << endl;
        return -1;
    }
    cout << "  - Resized Size:  " << resizedImg.cols << " x " << resizedImg.rows << endl;

    // 5. Test preprocess() member function (Grayscale + Gaussian Blur)
    cv::Mat blurredImg = detector.preprocess(resizedImg);
    if (blurredImg.empty()) {
        cerr << "[Test Error] preprocess() returned an empty image." << endl;
        return -1;
    }
    cout << "  - Preprocessed:  Successfully converted to Grayscale & Gaussian Blur." << endl;

    // 6. Display results (Convert grayscale to BGR for side-by-side display)
    cv::Mat blurredBGR;
    cv::cvtColor(blurredImg, blurredBGR, cv::COLOR_GRAY2BGR);

    cv::Mat displayCombined;
    cv::hconcat(resizedImg, blurredBGR, displayCombined);

    string windowTitle = "DiceDetector Test (Left: Resized | Right: Preprocessed)";
    cv::imshow(windowTitle, displayCombined);
    
    cout << "[Test] Displaying result window. Press any key on the image window to close." << endl;
    cv::waitKey(0);

    return 0;
}