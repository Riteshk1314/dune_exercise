#include "DataGenerator.h"
#include "SingleFileStrategy.h"
#include "ChunkedFileStrategy.h"
#include "IndividualFileStrategy.h"
#include "BenchmarkTimer.h"
#include "BenchmarkMetrics.h"
#include "DataValidator.h"
#include <iostream>
#include <iomanip>
#include <random>
#include <algorithm>
#include <numeric>

// keep same seed as generator so results are reproducible
std::vector<int> generateRandomIndices(size_t count, size_t max, unsigned int seed = 24) {
    std::vector<int> indices;
    indices.reserve(count);
    
    std::mt19937 rng(seed);
    std::uniform_int_distribution<int> dist(0, max - 1);
    
    for (size_t i = 0; i < count; ++i) {
        indices.push_back(dist(rng));
    }
    
    return indices;
}

BenchmarkMetrics runBenchmark(StorageStrategy* strategy, const std::vector<Record>& records, size_t totalDataSize) {
    BenchmarkMetrics result;
    result.strategy = strategy->getName();
    result.totalDataSize = totalDataSize;
    BenchmarkTimer timer;
    
    // std::cout << "DEBUG: starting " << strategy->getName() << std::endl;
    std::cout << "  Testing " << strategy->getName() << " strategy..." << std::endl;
    
    std::cout << "    Writing..." << std::flush;
    timer.start();
    strategy->write(records);
    timer.stop();
    result.writeTime = timer.getElapsedSeconds();
    std::cout << " Done (" << result.writeTime << "s)" << std::endl;
    
    result.diskSpaceUsed = strategy->getDiskSpaceUsed();
    result.numFiles = strategy->getNumFiles();
    
    std::cout << "    Sequential read..." << std::flush;
    timer.start();
    auto seqRecords = strategy->readSequential();
    timer.stop();
    result.seqReadTime = timer.getElapsedSeconds();
    std::cout << " Done (" << result.seqReadTime << "s)" << std::endl;
    
    result.dataVerified = DataValidator::verifyRecords(records, seqRecords);
    if (!result.dataVerified) {
        std::cerr << "    WARNING: sequential read verification failed!" << std::endl;
    }
    
    auto randomIndices = generateRandomIndices(1000, records.size());
    std::cout << "    Random read..." << std::flush;
    timer.start();
    auto randRecords = strategy->readRandom(randomIndices);
    timer.stop();
    result.randReadTime = timer.getElapsedSeconds();
    std::cout << " Done (" << result.randReadTime << "s)" << std::endl;
    
    if (!DataValidator::verifySubset(records, randRecords, randomIndices)) {
        std::cerr << "    WARNING: random read verification failed!" << std::endl;
        result.dataVerified = false;
    }
    
    strategy->cleanUp();
    return result;
}

void printResults(const std::vector<BenchmarkMetrics>& results) {
    std::cout << "\n========================================" << std::endl;
    std::cout << "BENCHMARK RESULTS" << std::endl;
    std::cout << "========================================\n" << std::endl;
    
    
    std::cout << std::left << std::setw(15) << "Strategy"
              << std::right << std::setw(12) << "Write (s)"
              << std::setw(15) << "Write (MB/s)"
              << std::setw(12) << "SeqRead (s)"
              << std::setw(15) << "SeqRead (MB/s)"
              << std::setw(12) << "RandRead (s)"
              << std::setw(13) << "Verified" << std::endl;
    std::cout << std::string(94, '-') << std::endl;
    
    for (const auto& result : results) {
        std::cout << std::left << std::setw(15) << result.strategy
                  << std::right << std::fixed << std::setprecision(3)
                  << std::setw(12) << result.writeTime
                  << std::setw(15) << result.writeThroughput()
                  << std::setw(12) << result.seqReadTime
                  << std::setw(15) << result.seqReadThroughput()
                  << std::setw(12) << result.randReadTime
                  << std::setw(13) << (result.dataVerified ? "YES" : "NO") << std::endl;
    }
    
    std::cout << "\n" << std::left << std::setw(15) << "Strategy"
              << std::right << std::setw(15) << "Disk Space"
              << std::setw(15) << "Num Files"
              << std::setw(18) << "Bytes/Record" << std::endl;
    std::cout << std::string(63, '-') << std::endl;
    
    for (const auto& result : results) {
        double bytesPerRecord = static_cast<double>(result.diskSpaceUsed) / 100000.0;
        std::cout << std::left << std::setw(15) << result.strategy
                  << std::right << std::fixed << std::setprecision(2)
                  << std::setw(15) << (result.diskSpaceUsed / 1024.0 / 1024.0) << " MB"
                  << std::setw(12) << result.numFiles
                  << std::setw(18) << bytesPerRecord << std::endl;
    }
    
    std::cout << "\n========================================\n" << std::endl;
}

int main() {
    const size_t NUM_RECORDS = 100000;
    const unsigned int SEED = 24;
    
    std::cout << "DUNE Fine-Grained Storage Benchmark" << std::endl;
    std::cout << "====================================" << std::endl;
    std::cout << "Generating " << NUM_RECORDS << " records..." << std::endl;
    
    DataGenerator generator(SEED);
    auto records = generator.generateRecords(NUM_RECORDS);
    
    size_t totalDataSize = 0;
    for (const auto& r : records) totalDataSize += r.data.size();
    
    std::cout << "Generation complete (" << std::fixed << std::setprecision(2)
              << (totalDataSize / 1024.0 / 1024.0)
              << " MB). Starting benchmarks...\n" << std::endl;
    
    std::vector<BenchmarkMetrics> results;
    
    {
        SingleFileStrategy strategy("data_single");
        results.push_back(runBenchmark(&strategy, records, totalDataSize));
    }
    
    {
        ChunkedFileStrategy strategy("data_chunked", 1000);
        results.push_back(runBenchmark(&strategy, records, totalDataSize));
    }
    
    {
        IndividualFileStrategy strategy("data_individual");
        results.push_back(runBenchmark(&strategy, records, totalDataSize));
    }
    
    printResults(results);
    
    std::cout << "Benchmark complete!" << std::endl;
    
    return 0;
}
