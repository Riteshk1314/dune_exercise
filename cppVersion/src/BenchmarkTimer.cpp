#include "BenchmarkTimer.h"

void BenchmarkTimer::start() {
    startTime = std::chrono::high_resolution_clock::now();
    running = true;
}

void BenchmarkTimer::stop() {
    endTime = std::chrono::high_resolution_clock::now();
    running = false;
}

double BenchmarkTimer::getElapsedSeconds() const {
    return std::chrono::duration<double>(endTime - startTime).count();
}

double BenchmarkTimer::getElapsedMilliseconds() const {
    return getElapsedSeconds() * 1000.0;
}
