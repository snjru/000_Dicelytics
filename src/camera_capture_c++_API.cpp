#include "pi_camera.h"

#include <iostream>
#include <memory>
#include <sys/mman.h>
#include <unistd.h>

#include <libcamera/libcamera.h>

using namespace libcamera;

cv::Mat captureGrayscaleDirect(int width, int height) {
    // 1. Start CameraManager and select the first Raspberry Pi camera
    std::unique_ptr<CameraManager> cm = std::make_unique<CameraManager>();
    cm->start();

    if (cm->cameras().empty()) {
        std::cerr << "[captureGrayscaleDirect] No Raspberry Pi camera found." << std::endl;
        cm->stop();
        return cv::Mat();
    }

    std::shared_ptr<Camera> camera = cm->cameras()[0];
    if (camera->acquire() < 0) {
        std::cerr << "[captureGrayscaleDirect] Failed to acquire camera." << std::endl;
        cm->stop();
        return cv::Mat();
    }

    // 2. Configure camera to capture YUV420 format (Y channel = grayscale)
    std::unique_ptr<CameraConfiguration> config = camera->generateConfiguration({ StreamRole::StillCapture });
    StreamConfiguration &streamConfig = config->at(0);
    streamConfig.pixelFormat = formats::YUV420;
    streamConfig.size.width = width;
    streamConfig.size.height = height;

    config->validate();
    if (camera->configure(config.get()) < 0) {
        std::cerr << "[captureGrayscaleDirect] Failed to configure camera." << std::endl;
        camera->release();
        cm->stop();
        return cv::Mat();
    }

    // 3. Allocate buffers and map hardware memory (mmap)
    Stream *stream = streamConfig.stream();
    FrameBufferAllocator allocator(camera);
    if (allocator.allocate(stream) < 0) {
        std::cerr << "[captureGrayscaleDirect] Failed to allocate buffers." << std::endl;
        camera->release();
        cm->stop();
        return cv::Mat();
    }

    const auto &buffers = allocator.buffers(stream);
    const auto &plane = buffers[0]->planes()[0];
    void* mappedAddr = mmap(NULL, plane.length, PROT_READ | PROT_WRITE, MAP_SHARED, plane.fd.get(), 0);

    if (mappedAddr == MAP_FAILED) {
        std::cerr << "[captureGrayscaleDirect] Failed to mmap buffer." << std::endl;
        camera->release();
        cm->stop();
        return cv::Mat();
    }

    // 4. Create request, queue it, and start camera
    std::unique_ptr<Request> request = camera->createRequest();
    request->addBuffer(stream, buffers[0].get());

    camera->start();
    camera->queueRequest(request.get());

    // Wait briefly for the frame to be captured (100ms)
    usleep(100000);

    // 5. Wrap the Y-plane (grayscale) memory directly into cv::Mat
    cv::Mat gray(height, width, CV_8UC1, mappedAddr);
    
    // Deep-copy to retain matrix data after cleaning up libcamera memory
    cv::Mat result = gray.clone();

    // 6. Cleanup resources
    camera->stop();
    munmap(mappedAddr, plane.length);
    camera->release();
    cm->stop();

    return result;
}