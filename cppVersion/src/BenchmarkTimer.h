#pragma once
#include <chrono>

class BenchmarkTimer {
public:
    void start();
    void stop();
    double getElapsedSeconds() const;
    double getElapsedMilliseconds() const;
    
private:
    std::chrono::high_resolution_clock::time_point startTime;
    std::chrono::high_resolution_clock::time_point endTime;
    bool running = false;
};
