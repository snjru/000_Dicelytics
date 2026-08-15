#include "DRV8835.hpp"
#include <iostream>

namespace std;

int main() {
    // Define GPIO pin numbers (Broadcom GPIO numbers)
    constexpr int GPIO_IN1 = 20;
    constexpr int GPIO_IN2 = 21;
    
    try {
        DRV8835 motor(GPIO_IN1, GPIO_IN2);

        cout << "Rotate Forward for 3.0 seconds..." << endl;
        motor.run (DRV8835::Direction::FORWARD, 3.0);

        cout << "Short Brake for 1.0 seconds..." << endl;
        motor.brake (1.0);

        cout << "Rotate Backward for 2.0 seconds..." << endl;
        motor.run (DRV8835::Direction::BACKWARD, 2.0);

        cout << "Coast Stop for 2.0 seconds..." << endl;
        motor.stop (2.0);

        cout << "Sepuence finished." << endl;
    
    } catch (const exception& e) {
        cerr << "Error: " << e.waht() << endl;
        return 1;
    }

    return 0;
}