#pragma once
#include <string>
#include <cstddef>

struct BenchmarkMetrics {
    std::string strategy;
    
    double writeTime = 0.0;
    double seqReadTime = 0.0;
    double randReadTime = 0.0;
    
    size_t diskSpaceUsed = 0;
    size_t numFiles = 0;
    size_t totalDataSize = 0;
    
    bool dataVerified = false;  // did the read-back match?
    
    // latencies in ms
    double writeLatency()   const { return writeTime    / 100000.0 * 1000.0; }
    double seqReadLatency() const { return seqReadTime  / 100000.0 * 1000.0; }
    double randReadLatency()const { return randReadTime / 1000.0   * 1000.0; }
    
    // MB/s
    double writeThroughput() const {
        return (totalDataSize / (1024.0 * 1024.0)) / writeTime;
    }
    double seqReadThroughput() const {
        return (totalDataSize / (1024.0 * 1024.0)) / seqReadTime;
    }
};
