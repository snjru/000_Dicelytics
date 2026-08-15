#ifndef DRV8835_HPP
#define DRV8835_HPP

#include <pigpio.h>
#include <chrono>
#include <thread>
#include <stdexcept>

class Drv8835Motor {
public:
    enum class Direction {
        FORWARD,
        BACKWARD
    };

    // Constructor: Assign GPIO pins for IN1 and IN2
    DRV8835(int in1Pin, int in2Pin);

    // Destructor: Clean up GPIO states
    ~DRV8835();

    // Core control functions
    void run(Direction dir, double seconds);
    void brake(double seconds);
    void stop(double seconds);

private:
    int in1Pin_;
    int in2Pin_;

    // Helper to sleep in seconds (accepts floating-point numbers)
    void sleepSecounds(double seconds);
};

#endif