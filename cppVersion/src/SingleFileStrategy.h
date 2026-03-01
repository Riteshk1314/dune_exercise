#pragma once
#include "StorageStrategy.h"
#include <vector>
#include <string>

// All records go into one binary file + a separate index file.
class SingleFileStrategy : public StorageStrategy {
public:
    SingleFileStrategy(const std::string& dir);
    
    void write(const std::vector<Record>& records) override;
    std::vector<Record> readSequential() override;
    std::vector<Record> readRandom(const std::vector<int>& indices) override;
    void cleanUp() override;
    std::string getName() const override { return "SingleFile"; }
    
    size_t getDiskSpaceUsed() const override;
    size_t getNumFiles() const override { return 2; }
    
private:
    std::string dataFile;
    std::string indexFile;
    std::vector<IndexEntry> index;
    
    void writeIndex();
    void readIndex();
};
