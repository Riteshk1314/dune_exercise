#include "SingleFileStrategy.h"
#include <fstream>
#include <filesystem>
#include <stdexcept>
#include <algorithm>

namespace fs = std::filesystem;

SingleFileStrategy::SingleFileStrategy(const std::string& dir) {
    baseDir = dir;
    dataFile = dir + "/single_data.dat";
    indexFile = dir + "/single_index.idx";
    fs::create_directories(dir);
}

void SingleFileStrategy::write(const std::vector<Record>& records) {
    std::ofstream out(dataFile, std::ios::binary);
    if (!out) throw std::runtime_error("cant open data file");
    
    // 4MB buffer - tried smaller values but this was fastest on my machine
    constexpr size_t bufferSize = 4 * 1024 * 1024;
    std::vector<char> buffer(bufferSize);
    out.rdbuf()->pubsetbuf(buffer.data(), bufferSize);
    
    index.clear();
    index.reserve(records.size());
    
    size_t currentOffset = 0;
    for (const auto& record : records) {
        out.write(record.data.data(), record.data.size());
        index.emplace_back(record.id, currentOffset, record.data.size());
        currentOffset += record.data.size();
    }
    
    out.close();
    writeIndex();
}

std::vector<Record> SingleFileStrategy::readSequential() {
    readIndex();
    std::ifstream in(dataFile, std::ios::binary);
    if (!in) throw std::runtime_error("Failed to open data file for reading");
    
    constexpr size_t bufferSize = 4 * 1024 * 1024;
    std::vector<char> buffer(bufferSize);
    in.rdbuf()->pubsetbuf(buffer.data(), bufferSize);
    
    std::vector<Record> records;
    records.reserve(index.size());
    
    for (const auto& entry : index) {
        Record record(entry.recordId, entry.size);
        in.read(record.data.data(), entry.size);
        records.push_back(std::move(record));
    }
    
    return records;
}

std::vector<Record> SingleFileStrategy::readRandom(const std::vector<int>& indices) {
    readIndex();
    std::ifstream in(dataFile, std::ios::binary);
    if (!in) throw std::runtime_error("Failed to open data file for reading");
    
    std::vector<std::pair<int, size_t>> sorted;
    sorted.reserve(indices.size());
    for (size_t i = 0; i < indices.size(); ++i) {
        sorted.emplace_back(indices[i], i);
    }
    std::sort(sorted.begin(), sorted.end(),
              [this](const auto& a, const auto& b) {
                  return index[a.first].offset < index[b.first].offset;
              });
    
    std::vector<Record> records(indices.size());
    for (const auto& [idx, origPos] : sorted) {
        const auto& entry = index[idx];
        Record record(entry.recordId, entry.size);
        in.seekg(entry.offset);
        in.read(record.data.data(), entry.size);
        records[origPos] = std::move(record);
    }
    
    return records;
}

void SingleFileStrategy::writeIndex() {
    std::ofstream out(indexFile, std::ios::binary);
    if (!out) throw std::runtime_error("Failed to open index file for writing");
    
    size_t count = index.size();
    out.write(reinterpret_cast<const char*>(&count), sizeof(count));
    out.write(reinterpret_cast<const char*>(index.data()), count * sizeof(IndexEntry));
}

void SingleFileStrategy::readIndex() {
    std::ifstream in(indexFile, std::ios::binary);
    if (!in) throw std::runtime_error("Failed to open index file for reading");
    
    size_t count;
    in.read(reinterpret_cast<char*>(&count), sizeof(count));
    
    index.resize(count);
    in.read(reinterpret_cast<char*>(index.data()), count * sizeof(IndexEntry));
}

void SingleFileStrategy::cleanUp() {
    fs::remove(dataFile);
    fs::remove(indexFile);
}

size_t SingleFileStrategy::getDiskSpaceUsed() const {
    size_t total = 0;
    
    if (fs::exists(dataFile)) {
        total += fs::file_size(dataFile);
    }
    if (fs::exists(indexFile)) {
        total += fs::file_size(indexFile);
    }
    
    return total;
}
