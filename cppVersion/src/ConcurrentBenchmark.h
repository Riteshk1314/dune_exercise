#pragma once
#include "StorageStrategy.h"
#include "BenchmarkMetrics.h"
#include <vector>
#include <thread>

class ConcurrentBenchmark {
public:
    static BenchmarkMetrics testConcurrentWrites(
        const std::vector<Record>& records,
        size_t numThreads = 4
    );
    
private:
    static void writeChunk(
        const std::vector<Record>& records,
        size_t startIdx,
        size_t endIdx,
        const std::string& outputDir,
        int threadId
    );
};
