#pragma once
#include "StorageStrategy.h"
#include <vector>

// Splits records into fixed-size chunks, each in its own file.
class ChunkedFileStrategy : public StorageStrategy {
public:
    ChunkedFileStrategy(const std::string& dir, size_t recordsPerChunk = 1000);
    
    void write(const std::vector<Record>& records) override;
    std::vector<Record> readSequential() override;
    std::vector<Record> readRandom(const std::vector<int>& indices) override;
    void cleanUp() override;
    std::string getName() const override { return "Chunked"; }
    
    size_t getDiskSpaceUsed() const override;
    size_t getNumFiles() const override;
    
private:
    size_t recordsPerChunk;
    size_t totalChunks = 0;
    std::string indexFile;
    
    std::vector<IndexEntry> index;
    std::vector<int> recordOrder; // for sequential reads
    
    std::string getChunkFileName(int chunkId) const;
    void writeIndex();
    void readIndex();
};
