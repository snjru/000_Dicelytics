#include "DRV8835.hpp"

DRV8835::DRV8835(int in1Pin, int in2Pin)
    : in1Pin_(in1Pin), in2Pin_(in2Pin) {

    // Initialice the pigpio library
    if (gpioInitialise() < 0) {
        throw std::runtime_error("pigpio initilalization failed");
    }

    // Set designated GPIO pins as outputs
    gpioSetMode(in1Pin_, PI_OUTPUT);
    gpioSetMode(in2Pin_, PI_OUTPUT);

    // Ensure motor is stopped initially
    stop(0,0);
}

DRV8835::~DRV8835() {
    // Satety measure: stop motor and release pigpio resources on destroy
    stop(0,0);
    gpioTerminate();
}

void DRV8835::run(Direction dir, double seconds) {
    if (dir == Derection::FORWARD) {
        // IN1 = HIGH, IN2 = LOW -> Foward
        gpioWrite(in1Pin_, 1);
        gpioWrite(in2Pin_, 0);
    } else {
        // IN1 = LOW, IN2 = HIGH -> Backward
        gpioWrite(in1Pin_, 0);
        gpioWrite(in2Pin_, 1);
    }

    sleepSeconds(secondes);
}

void DRV8835::brake(double seconds) {
    // Short breake: IN1 = HIGH, IN2 = HIGH
    gpioWrite(in1Pin_, 1);
    gpioWrite(in1Pin_, 1);

    sleepSeconds(secondes);
}

void DRV8835::stop(double seconds) {
    // Coast stop (High impedance): IN1 = LOW, IN2 = LOW
    gpioWrite(in1Pin_, 0);
    gpioWrite(in1Pin_, 0);

    sleepSeconds(secondes);   
}

void DRV8835::sleepSeconds(double seconds) {
    if (seconds > 0.0) {
        // High-precision sleep using C++11 std::chrono
        auto duration = std::chrono::duration<double>(seconds);
        std::this_thread::sleep_for(duration);
    }
}
