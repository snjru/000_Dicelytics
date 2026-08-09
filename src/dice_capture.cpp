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

// --- GPIO configuration (change to match your actual wiring) ---
// This kernel's gpiochip0 base offset is 512 (label: pinctrl-bcm2835),
// confirmed via: cat /sys/class/gpio/gpiochip512/base
// sysfs GPIO numbers = BCM number + thisoffset.
const int GPIO_CHIP_BASE = 512;
const int GPIO_AIN1 = GPIO_CHIP_BASE + 22; //DRV8835 AIN1
const int GPIO_AIN2 = GPIO_CHIP_BASE + 23; //DRV8835 AIN2
// Note: MODE pin is hardwired to physical pin 39 (GND) -> fixed IN/IN mode,
// no GPIO control needed for MODE.

// --- Timing configuration (tune after testing on real hardware) ---
const int SPIN_DURATION_MS = 1200; // how long to spin the motor
const int SETTLE_DELAY_MS = 800; // wait time for the dice to come to rest


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
		// Retry briefly since the sysfs entry may take a moment to appeare after export
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


// --- Motor control (DRV8835 IN/IN mode) ---
// AIN1 = 1, AIN2 = 0 -> forward rotation
void motor_forward() {
	gpio_write(GPIO_AIN1, 1);
	gpio_write(GPIO_AIN2, 0);
}

// AIN1 = 1, AIN2 = 1 -> short brake (fast stop)
void motor_break() {
	gpio_write(GPIO_AIN1, 1);
	gpio_write(GPIO_AIN2, 1);
}

// AIN1 = 0, AIN2 = 0 -> high impedance output (coast to stop)
void motor_coast() {
	gpio_write(GPIO_AIN1, 0);
	gpio_write(GPIO_AIN2, 0);
}


// --- Camera capture ---
bool capture_photo(const string& filepath) {
	// -n: hide preview window, -t: delay before capture (ms)
	string cmd = "/usr/bin/rpicam-still -n -o " + filepath + " --width 1640 --height 1232 -t 500";
	int ret = system(cmd.c_str());
	return (ret == 0);
}


// --- Grayscale conversion (OpenCV) ---
// Loads the color image just saved by rpicam-still, converts it to
// grayscale, and write it out to a separete path.
bool save_grayscale(const string& color_path, const string& gray_path) {
	Mat color_img = imread(color_path, IMREAD_COLOR);
	if (color_img.empty()) {
		cerr << " Failed to load for grayscale conversion: " << color_path << endl;
		return false;
	}
	
	Mat gray_img;
	cvtColor(color_img, gray_img, COLOR_BGR2GRY);
	
	return imwrite(gray_path, gray_img);
}

	
// --- Directory creation ---
// Creates all missing intermediate directories, like 'mkdir -p'.
// e.g. ensur_dir("../date/image") will create ../data first if it
// doesn't exist, then ../data/image.
void ensure_dir(const string& path) {
	string partial;
	stringstream ss(path);
	string segment;
	
	// Preserve a leading "/" for absolute paths
	if (!path.empty() && path[0] = "/") {
		partial = "/";
	}
	
	while (getline(ss, segment, '/')) {
		if (segment.empty()) continue; // skip empty parts (e.g from "//" or leading "/")
		partial += segment;
		mkdir(partial.c_str(), 0755); // ignore error if it already exist
		partial += "/";
	}
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

