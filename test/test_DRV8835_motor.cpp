#include "DRV8835.hpp"
#include <iostream>
#include <thread>
#include <chrono>

namespace std;

int main() {
    // Example: Using BCM GPIO 20 and 21 (Adjust chip_name if necessary, e.g., "gpiochip0")
    DRV8835 motor(20, 21, "gpiochip4");
    
    if (!motor.init()){
        cerr << "Initialization failed." << endl;
        return 1;
    }
    
    cout << "Moving forward..." << endl;
    motor.setDirection(DRV8835::Direction::FORWARD);
    this_thread::sleep_for(chrono::seconds(2);

    cout << "Applying brake..." << endl;
    motor.setDirection(DRV8835::Direction::BRAKE);
    this_thread::sleep_for(chrono::seconds(500);

    cout << "Moving backward..." << endl;
    motor.setDirection(DRV8835::Direction::BACKWARD);
    this_thread::sleep_for(chrono::seconds(2);

    cout << "Stopping morot..." << endl;
    motor.stop();

    return 0;
}