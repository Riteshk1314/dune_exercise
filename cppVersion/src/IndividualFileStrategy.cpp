#include "IndividualFileStrategy.h"
#include <fstream>
#include <filesystem>
#include <stdexcept>
#include <cstdio>

namespace fs = std::filesystem;

IndividualFileStrategy::IndividualFileStrategy(const std::string& dir)
    : totalRecords(0) {
    baseDir = dir;
    fs::create_directories(dir);
}

std::string IndividualFileStrategy::getRecordFileName(int recordId) const {
    char buf[512];
    int subdir = recordId / 1000;
    // e.g. data_individual/042/record_042123.dat
    std::snprintf(buf, sizeof(buf), "%s/%03d/record_%06d.dat",
                  baseDir.c_str(), subdir, recordId);
    return std::string(buf);
}

void IndividualFileStrategy::ensureSubdirectories(size_t numRecords) {
    size_t numSubdirs = (numRecords + 999) / 1000;
    for (size_t i = 0; i < numSubdirs; ++i) {
        char buf[512];
        std::snprintf(buf, sizeof(buf), "%s/%03zu", baseDir.c_str(), i);
        fs::create_directories(buf);
    }
}

void IndividualFileStrategy::write(const std::vector<Record>& records) {
    totalRecords = records.size();
    recordSizes.resize(records.size());
    
    ensureSubdirectories(totalRecords);
    
    // this is slow but not much we can do - filesystem overhead dominates
    // TODO: maybe try async writes?
    for (const auto& record : records) {
        recordSizes[record.id] = record.data.size();
        
        std::ofstream out(getRecordFileName(record.id), std::ios::binary);
        if (!out) throw std::runtime_error("couldnt create record file");
        out.write(record.data.data(), record.data.size());
    }
}

std::vector<Record> IndividualFileStrategy::readSequential() {
    std::vector<Record> records;
    records.reserve(totalRecords);
    
    for (size_t i = 0; i < totalRecords; ++i) {
        size_t size = recordSizes[i];
        
        std::ifstream in(getRecordFileName(i), std::ios::binary);
        if (!in) throw std::runtime_error("Failed to open record file");
        
        Record record(i, size);
        in.read(record.data.data(), size);
        records.push_back(std::move(record));
    }
    
    return records;
}

std::vector<Record> IndividualFileStrategy::readRandom(const std::vector<int>& indices) {
    std::vector<Record> records;
    records.reserve(indices.size());
    
    for (int idx : indices) {
        size_t size = recordSizes[idx];
        
        std::ifstream in(getRecordFileName(idx), std::ios::binary);
        if (!in) throw std::runtime_error("Failed to open record file");
        
        Record record(idx, size);
        in.read(record.data.data(), size);
        records.push_back(std::move(record));
    }
    
    return records;
}

void IndividualFileStrategy::cleanUp() {
    fs::remove_all(baseDir);
}

size_t IndividualFileStrategy::getDiskSpaceUsed() const {
    size_t total = 0;
    
    if (fs::exists(baseDir)) {
        for (const auto& entry : fs::recursive_directory_iterator(baseDir)) {
            if (entry.is_regular_file()) {
                total += entry.file_size();
            }
        }
    }
    
    return total;
}
