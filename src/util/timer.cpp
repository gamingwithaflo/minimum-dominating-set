#include <chrono>
#include "timer.h"

//Constructor to intialize t_start with current time.
timer::timer() {
	t_start = std::chrono::system_clock::now();
}

// Function to calculate elapsed time in seconds (could be changed)
long long timer::count() const {
	auto elapsed = std::chrono::system_clock::now() - t_start;
	return std::chrono::duration_cast<std::chrono::seconds>(elapsed).count();
}

//use case: start timer by creating timer t;
//then run t.count() to find out how long has elapsed.