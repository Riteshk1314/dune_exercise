// Not hooked up to main benchmark yet -- just a quick parallel write test.
#include "ConcurrentBenchmark.h"
#include "BenchmarkTimer.h"
#include <fstream>
#include <filesystem>
#include <sstream>
#include <iomanip>

namespace fs = std::filesystem;

void ConcurrentBenchmark::writeChunk(
    const std::vector<Record>& records,
    size_t startIdx,
    size_t endIdx,
    const std::string& outputDir,
    int threadId
) {
    std::ostringstream oss;
    oss << outputDir << "/thread_" << threadId << ".dat";
    std::string filename = oss.str();
    
    std::ofstream out(filename, std::ios::binary);
    if (!out) throw std::runtime_error("Failed to open file for concurrent write");
    
    constexpr size_t bufferSize = 512 * 1024;
    std::vector<char> buffer(bufferSize);
    out.rdbuf()->pubsetbuf(buffer.data(), bufferSize);
    
    for (size_t i = startIdx; i < endIdx; ++i) {
        out.write(records[i].data.data(), records[i].data.size());
    }
}

BenchmarkMetrics ConcurrentBenchmark::testConcurrentWrites(
    const std::vector<Record>& records,
    size_t numThreads
) {
    BenchmarkMetrics result;
    result.strategy = "Concurrent(" + std::to_string(numThreads) + ")";
    
    std::string outputDir = "data_concurrent";
    fs::create_directories(outputDir);
    
    size_t recordsPerThread = records.size() / numThreads;
    std::vector<std::thread> threads;
    
    BenchmarkTimer timer;
    timer.start();
    
    for (size_t i = 0; i < numThreads; ++i) {
        size_t startIdx = i * recordsPerThread;
        size_t endIdx = (i == numThreads - 1) ? records.size() : (i + 1) * recordsPerThread;
        
        threads.emplace_back(writeChunk, std::ref(records), startIdx, endIdx, outputDir, i);
    }
    
    for (auto& thread : threads) {
        thread.join();
    }
    
    timer.stop();
    result.writeTime = timer.getElapsedSeconds();
    
    // Calculate disk space
    result.diskSpaceUsed = 0;
    result.numFiles = numThreads;
    for (size_t i = 0; i < numThreads; ++i) {
        std::string filename = outputDir + "/thread_" + std::to_string(i) + ".dat";
        if (fs::exists(filename)) {
            result.diskSpaceUsed += fs::file_size(filename);
        }
    }
    
    // Cleanup
    fs::remove_all(outputDir);
    
    
    return result;
}
