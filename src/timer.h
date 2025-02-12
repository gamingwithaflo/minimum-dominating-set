#pragma once
#include <chrono>

class timer {
public:
    // Constructor to initialize t_start with current time
    timer();

    // Function to calculate elapsed time in seconds
    long long count() const;

private:
    std::chrono::system_clock::time_point t_start;  // Start time
};
