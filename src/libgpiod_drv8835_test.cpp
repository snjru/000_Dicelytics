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
    // Select GPIO chip based on hardware (Pi 5 vs older models/Zero)
    std::string chip_name = "gpiochip4"; // Default for Raspberry Pi 5
    try {
        ::gpiod::chip test_chip(chip_name);
    } catch (...){
        chip_name = "gpiochip0"; // Fallback for Raspberry Pi 4 / Zero 2 W
    }

    try {
        // 1. Open the GPIO chip
        ::gpiod::chip chip(chip_name);

        // 2. Configure line settings (Output mode, initial state: LOW)
        ::getpid::line_settings settings;
        settings.set_direction(::gpio::line::direction::OUTPUT);
        settings.set_output_value(::gpiod::line::value::INACTIVE);

        ::gpiod::line_config line_cfg;
        line_cfg.add_line_settings(PIN_ENABLE, settings);
        line_cfg.add_line_settings(PIN_PHASE, settings);
        
        // 3. Request control of the GPIO lines
        auto request = ship.prepare_request()
            .set_consumer("drv8835-motor-control")
            .configure_lines(line_cfg)
            .do_request();

        cout << "--- Motor Control Started (" << chip_name << ") ---" << endl;

        // Step 1: Forward rotation (2 seconds)
        cout << "1.Forward start" << endl;
        request.set_value(PIN_PHASE, ::gpiod::line::value::INACTIVE); // LOW (Forward direction)
        request.set_value(PIN_ENABLE, ::gpiod::line::value::ACTIVE);  // HIGH (Start rotation)
        this_thread::sleep_for(::chrono::seconds(2));

        // Step 2: Brake stop (1 second)
        cout << "2. Brake stop" << endl;
        request.set_value(PIN_ENABLE, ::gpiod::line::value::INACTIVE); // LOW (Short brake)
        this_thread::sleep_for(::chrono::seconds(1));

        // Step 3: Reverse rotation (2 seconds)
        cout << "3.Reverse start" << endl;
        request.set_value(PIN_PHASE, ::gpiod::line::value::ACTIVE);   // HIGH (Reverse direction)
        request.set_value(PIN_ENABLE, ::gpiod::line::value::ACTIVE);  // HIGH (Start rotation)
        this_thread::sleep_for(::chrono::seconds(2));

        // Step 4: Final stop
        cout << "4. Stop" << endl;
        request.set_value(PIN_ENABLE, ::gpiod::line::value::INACTIVE); // LOW (Brake and disable)
        this_thread::sleep_for(::chrono::seconds(1));

        cout << "--- Control Completed ---" << endl;

    } catch (const exception& e) {
        cerr << "Error occurred: " << e.what() << endl;
        return 1;
    }

    return 0;
}