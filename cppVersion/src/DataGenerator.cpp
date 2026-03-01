#include "DataGenerator.h"

DataGenerator::DataGenerator(unsigned int seed)
    : rng(seed), sizeDist(1024, 2048), byteDist(0, 255) {}

std::vector<Record> DataGenerator::generateRecords(size_t count) {
    std::vector<Record> records;
    records.reserve(count);
    
    for (size_t i = 0; i < count; ++i) {
        size_t size = sizeDist(rng);
        records.emplace_back(static_cast<int>(i), size);
        
        for (size_t j = 0; j < size; ++j) {
            records.back().data[j] = static_cast<char>(byteDist(rng));
        }
    }
    
    return records;
}

Record DataGenerator::generateRecord(int id) {
    size_t size = sizeDist(rng);
    Record record(id, size);
    
    for (size_t i = 0; i < size; ++i) {
        record.data[i] = static_cast<char>(byteDist(rng));
    }
    
    return record;
}
