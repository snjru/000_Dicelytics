#include "DRV8835.hpp"
#include <iostream>

DRV8835::DRV8835(int in1Pin, int in2Pin, const string& chip_name)
    : in1Pin_(in1Pin), in2Pin_(in2Pin), chip_name_(chip_name), initialized_(false) {}

DRV8835::~DRV8835() {
    if (initialized_) {
        stop(0,0);
        in1_line_.release();
        in2_line_.release();
    }
}

bool DRV8835::init() {
    try {
        // Open the GPIO chip
        chip_ = gpiod::chip(chip_name_);

        // Get the GPIO lines for IN1 and IN2
        in1_line_ = chip_.get_line(in1_pin_);
        in2_line_ = chip_.get_line(in2_pin_);

        // Request lines as output with default initial value 0 (LOW)
        in1_line_.request({"DRV8835_IN1", gpiod::line_request::DIRECTION_OUTPUT, 0}, 0);
        in2_line_.request({"DRV8835_IN2", gpiod::line_request::DIRECTION_OUTPUT, 0}, 0);

        initialized_ = true;
        return true;
    } catch (const exception& e) {
        cerr << "[DRV8835 Error] Failed to initialize libgpiod: " << e.what() << endl;
        return false;
    }
}

void DRV8835::setDirection(Direction dir) {
    if (!initialized_) return;

    switch (dir) {
        case Direction::FORWARD:
            in1_line_.set_value(1);
            in2_line_.set_value(0);
            break;
        case Direction::BACKWARD:
            in1_line_.set_value(0);
            in2_line_.set_value(1);
            break;
        case Direction::BRAKE:
            in1_line_.set_value(1);
            in2_line_.set_value(1);
            break;
        case Direction::COAST:
        default:
            in1_line_.set_value(0);
            in2_line_.set_value(0);
            break;
    }
}

void DRV8835::stop() {
    setDirection(Direction::COAST);
}
