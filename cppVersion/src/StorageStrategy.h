#pragma once
#include "Record.h"
#include <vector>
#include <string>
#include <cstddef>

class StorageStrategy {
public:
    virtual ~StorageStrategy() = default;
    
    virtual void write(const std::vector<Record>& records) = 0;
    virtual std::vector<Record> readSequential() = 0;
    virtual std::vector<Record> readRandom(const std::vector<int>& indices) = 0;
    virtual void cleanUp() = 0;
    virtual std::string getName() const = 0;
    
    virtual size_t getDiskSpaceUsed() const = 0;
    virtual size_t getNumFiles() const = 0;
    
protected:
    std::string baseDir;
};
