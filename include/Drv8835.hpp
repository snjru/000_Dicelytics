#ifndef DRV8835_HPP
#define DRV8835_HPP

#include <gpiod.h>
#include <string>

using namespace std;


// Contoroled by DRV8835 motor class
class Drv8835 {
public:
    enum class Direction {
        FORWARD,    // Forward rotation (IN1: HIGH, IN2: LOW)
        BACKWARD,   // Reverse rotation (IN1: LOW, IN2: HIGH)
        BRAKE,      // Active brake (IN1: HIGH, IN2: HIGH)
        COAST       // Coast / Stop (IN1: LOW, IN2: LOW)
    };

    // Constructor: specify GPIO pin numbers and the GPIO chip name
    DRV8835(int in1_pin, int in2_pin, const string& chip_name = "gpiochip4");
    // Destructor: Clean up GPIO states
    ~DRV8835();

    // Initialize GPIO lines
    bool init();

    // Motor control GPIO lines
    void setDirection(Direction dir);
    void stop();

private:
    int in1_pin_;
    int in2_pin_;
    string chip_name_;

    gpiod::chip chip_;
    gpiod::line in1_line_;
    gpild::line in2_line_;
    bool initialized_;
};

#endif  // DRV8835_HPP