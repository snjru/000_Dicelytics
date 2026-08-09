// dice_capture.cpp
// Dicelytics - Spin the dice via motor, take a photo, and accumulate images
// Usage: ./dice_capture <num_rolls> [output_dir]

#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib>
#include <unistd.h>
#include <chrono>
#include <thread>
#include <sstream>
#include <iomanip>
#include <sys/stat.h>
#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv; 

// --- GPIO configuration ---
// sysfs GPIO numbers = BCM number + GPIO_CHIP_BASE
const int GPIO_CHIP_BASE = 512;

// DRV8835 PHASE/ENABLE Mode Wiring:
// MODE pin is tied to 3.3V (HIGH)
const int GPIO_APHASE  = GPIO_CHIP_BASE + 22; // DRV8835 APHASE (Direction: BCM 22)
const int GPIO_AENABLE = GPIO_CHIP_BASE + 23; // DRV8835 AENABLE (Speed/OnOff: BCM 23)

// --- Timing configuration ---
const int SPIN_DURATION_MS = 1200; // how long to spin the motor
const int SETTLE_DELAY_MS  = 800;  // wait time for the dice to come to rest


// --- GPIO control (sysfs-based) ---
void gpio_export(int pin) {
    ofstream f("/sys/class/gpio/export");
    if (f.is_open()) f << pin;
}

void gpio_unexport(int pin) {
    ofstream f("/sys/class/gpio/unexport");
    if (f.is_open()) f << pin;
}

void gpio_set_direction(int pin, const string& dir) {
    string path = "/sys/class/gpio/gpio" + to_string(pin) + "/direction";
    for (int i = 0; i < 20; i++) {
        ofstream f(path);
        if (f.is_open()) { f << dir; return; }
        this_thread::sleep_for(chrono::milliseconds(50));
    }
    cerr << "Failed to set GPIO direction: " << pin << endl;
}

void gpio_write(int pin, int value) {
    string path = "/sys/class/gpio/gpio" + to_string(pin) + "/value";
    ofstream f(path);
    if (f.is_open()) f << value;
    else cerr << "Failed to write GPIO: " << pin << endl;
}

void gpio_init(int pin) {
    gpio_export(pin);
    gpio_set_direction(pin, "out");
}


// --- Motor control (DRV8835 PHASE/ENABLE mode) ---

// APHASE = 1 (Forward), AENABLE = 1 (Motor On)
void motor_forward() {
    gpio_write(GPIO_APHASE, 1);
    gpio_write(GPIO_AENABLE, 1);
}

// APHASE = 0 (Reverse), AENABLE = 1 (Motor On) -- Included for reference
void motor_reverse() {
    gpio_write(GPIO_APHASE, 0);
    gpio_write(GPIO_AENABLE, 1);
}

// AENABLE = 0 (Motor Off / Coast)
void motor_stop() {
    gpio_write(GPIO_AENABLE, 0);
}


// --- Camera capture ---
bool capture_photo(const string& filepath) {
    string cmd = "/usr/bin/rpicam-still -n -o " + filepath + " --width 1640 --height 1232 -t 500";
    int ret = system(cmd.c_str());
    return (ret == 0);
}


// --- Directory creation ---
void ensure_dir(const string& path) {
    string partial;
    stringstream ss(path);
    string segment;
    
    if (!path.empty() && path[0] == '/') {
        partial = "/";
    }
    
    while (getline(ss, segment, '/')) {
        if (segment.empty()) continue;
        partial += segment;
        mkdir(partial.c_str(), 0755);
        partial += "/";
    }
}


// --- Path Helper ---
string make_filename(const string& output_dir, int index, const string& suffix = "") {
    ostringstream oss;
    oss << output_dir << "/dice_" << setw(4) << setfill('0') << index << suffix << ".jpg";
    return oss.str();
}


// --- Main ---
int main(int argc, char* argv[]) {
    
    if (argc < 2) {
        cerr << "Usage: " << argv[0] << " <num_rolls> [output_dir]" << endl; 
        return 1;
    }
    
    int num_rolls = atoi(argv[1]);
    if (num_rolls <= 0) {
        cerr << "num_rolls must be a positive integer" << endl;
        return 1;
    }
    
    string output_dir = (argc >= 3) ? argv[2] : "../data/image";
    ensure_dir(output_dir);
    
    cout << "Starting " << num_rolls << " roll(s), output dir: " << output_dir << endl;

    // Initialize GPIO
    gpio_init(GPIO_APHASE);
    gpio_init(GPIO_AENABLE);
    motor_stop();
    
    for (int i = 1; i <= num_rolls; i++) {
        cout << "[" << i << "/" << num_rolls << "] Spinning motor..." << endl;
        
        motor_forward();
        this_thread::sleep_for(chrono::milliseconds(SPIN_DURATION_MS));
        
        motor_stop();
        // Wait for the die to settle completely
        this_thread::sleep_for(chrono::milliseconds(SETTLE_DELAY_MS));
        
        string filepath = make_filename(output_dir, i);
        cout << " Capturing: " << filepath << endl;
        
        if (!capture_photo(filepath)) {
            cerr << " Capture failed: " << filepath << endl;
            continue;
        }
        
        // --- OpenCV Image Processing Block ---
        Mat src = imread(filepath, IMREAD_COLOR);
        if (src.empty()) {
            cerr << " OpenCV Error: Failed to load image: " << filepath << endl;
            continue;
        }
        
        Mat gray;
        cvtColor(src, gray, COLOR_BGR2GRAY);
        cout << " OpenCV: Successfully converted to grayscale (Size: " << gray.cols << "x" << gray.rows << ")" << endl;
        
        string gray_filepath = make_filename(output_dir, i, "_gray");
        imwrite(gray_filepath, gray);
    }
    
    // Release GPIO
    motor_stop();
    gpio_unexport(GPIO_APHASE);
    gpio_unexport(GPIO_AENABLE);
    
    cout << "Done. Saved " << num_rolls << " image(s) to " << output_dir << endl;
    return 0;
}

// --- Roll directory naming ---
// Build the path for a single roll's derectory, e.g. "output_dir/roll_001".
// Individual image (color.img, gray.img,and any future processed variants) are saved inside this directory.
string make_roll_dir(const string& output_dir, int index) {
	ostringstream oss;
	oss << output_dir << "/roll_" << setw(4) << setfill('0') << index";
	return oss.str();
}


// --- Main ---

int main(int argc, char* argv[]) {
	
	if (argc < 2) {
		cerr << "Usage: " << argv[0] << " <mum_rolls> [output_dir]" << endl; 
		return 1;
	}
	
	int num_rolls = atoi(argv[1]);
	if (num_rolls <= 0) {
		cerr << "num_rolls must be a positive integer" << endl;
		return 1;
	}
	
	string output_dir = (argc >= 3) ? argv[2] : "../data/image";
	ensure_dir(output_dir);
	
	cout << "Starting " << num_rolls << " roll(s), output dir: " << output_dir << endl;

	// Initialize GPIO
	gpio_init(GPIO_AIN1);
	gpio_init(GPIO_AIN2);
	motor_coast();
	
	
	//////------from here------
	
	for (int i = 1; i <= num_rolls; i++) {
		cout << "[" << i << "/" << num_rolls << "] Spinning motor..." << endl;
		
		motor_forward();
		this_thread::sleep_for(chrono::milliseconds(SPIN_DURATION_MS));
		motor_break();
		this_thread::sleep_for(chrono::milliseconds(SETTLE_DELAY_MS));
		motor_coast();
		// Wait for the die to settle
		this_thread::sleep_for(chrono::milliseconds(SETTLE_DELAY_MS));
		
		string filepath = make_filename(output_dir,i);
		cout << " Capturing: " << filepath << endl;
		
		if (!capture_photo(filepath)) {
			cerr << " Capture failed: " << filepath << endl;
			continue;
		}
		
		// --- OpenCV Image Processing Block ---
		// Read the captured JPEG image as a BGR color Mat
		Mat src = imread(filepath, IMREAD_COLOR);
		if (src.empty()){
			cerr << " OpenCV Error: Failed to load image: " << filepath << endl;
			continue;
		}
		
		// Create a Mat container for the grayscale image
		Mat gray;
		
		// Convert BGR color image to Grayscale
		cvtColor(src, gray, COLOR_BGR2GRAY);
		cout << " OpenCV: Successfully converted to grayscale (Size: " << gray.cols << "x" << gray.rows << ")" << endl;
		
		// Save the grayscale image to a separate file for verification
		string gray_filepath = make_filename(output_dir, i, "_gray");
		imwrite(gray_filepath, gray);
				
	}
	
	// Release GPIO
	motor_coast();
	gpio_unexport(GPIO_AIN1);
	gpio_unexport(GPIO_AIN2);
	
	cout << "Done. Saved " << num_rolls << " image(s) to " << output_dir << endl;
	return 0;
}

