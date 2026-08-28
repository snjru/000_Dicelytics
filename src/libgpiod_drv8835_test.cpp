#include <gpiod.hpp>
#include <iostream>
#include <thread>
#include <chrono>

// Pin assignments
constexpr unsigned int PIN_ENABLE = 17; // Morot enable and brake control
constexpr unsigned int PIN_PHASE  = 27; // DIrection control

// namespace setting
using namespace std;

int main(){
    // Raspberry Pi Zero 2 W uses gpiochip0
    const std::string chip_path = "/dev/gpiochip0";
    
    try {
        // 1. Open the GPIO chip
        ::gpiod::chip chip(chip_path);

        // 2. Configure line settings (Output mode, initial state: LOW)
        ::gpiod::line_settings settings;
        settings.set_direction(::gpiod::line::direction::OUTPUT);
        settings.set_output_value(::gpiod::line::value::INACTIVE);

        
        // 3. Request control of the GPIO lines
        auto request = chip.prepare_request()
            .set_consumer("drv8835-motor-control")
            .add_line_settings(PIN_ENABLE, settings)
            .add_line_settings(PIN_PHASE, settings)
            .do_request();

        cout << "--- Motor Control Started (" << chip_path << ") ---" << endl;

        // Step 1: Forward rotation (2 seconds)
        cout << "1.Forward start" << endl;
        request.set_value(PIN_PHASE, ::gpiod::line::value::INACTIVE); // LOW (Forward direction)
        request.set_value(PIN_ENABLE, ::gpiod::line::value::ACTIVE);  // HIGH (Start rotation)
        this_thread::sleep_for(::chrono::seconds(2));

        // Step 2: Brake stop (2 second)
        cout << "2.Brake stop" << endl;
        request.set_value(PIN_ENABLE, ::gpiod::line::value::INACTIVE); // LOW (Short brake)
        this_thread::sleep_for(::chrono::seconds(2));

        // Step 3: Reverse rotation (2 seconds)
        cout << "3.Reverse start" << endl;
        request.set_value(PIN_PHASE, ::gpiod::line::value::ACTIVE);   // HIGH (Reverse direction)
        ::this_thread::sleep_for(::chrono::milliseconds(10));
        request.set_value(PIN_ENABLE, ::gpiod::line::value::ACTIVE);  // HIGH (Start rotation)
        this_thread::sleep_for(::chrono::seconds(2));

        // Step 4: Final stop
        cout << "4.Stop" << endl;
        request.set_value(PIN_ENABLE, ::gpiod::line::value::INACTIVE); // LOW (Brake and disable)
        this_thread::sleep_for(::chrono::seconds(1));

        cout << "--- Control Completed ---" << endl;

    } catch (const exception& e) {
        cerr << "Error occurred: " << e.what() << endl;
        return 1;
    }

    return 0;
}
