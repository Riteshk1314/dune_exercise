#pragma once
#include "Record.h"
#include <vector>
#include <random>

// fixed seed for reproducibility.
class DataGenerator {
public:
    DataGenerator(unsigned int seed = 24);
    
    std::vector<Record> generateRecords(size_t count);
    Record generateRecord(int id);
    
private:
    std::mt19937 rng;
    std::uniform_int_distribution<size_t> sizeDist; // 1024-2048 bytes
    std::uniform_int_distribution<int>    byteDist; // 0-255
};
