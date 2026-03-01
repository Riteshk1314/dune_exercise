#include "ChunkedFileStrategy.h"
#include <fstream>
#include <filesystem>
#include <stdexcept>
#include <algorithm>

namespace fs = std::filesystem;

ChunkedFileStrategy::ChunkedFileStrategy(const std::string& dir, size_t recordsPerChunk)
    : recordsPerChunk(recordsPerChunk) {
    baseDir = dir;
    indexFile = dir + "/chunked_index.idx";
    fs::create_directories(dir);
}

std::string ChunkedFileStrategy::getChunkFileName(int chunkId) const {
    return baseDir + "/chunk_" + std::to_string(chunkId) + ".dat";
}

void ChunkedFileStrategy::write(const std::vector<Record>& records) {
    index.clear();
    index.resize(records.size());  // direct indexing by record ID
    recordOrder.clear();
    recordOrder.reserve(records.size());
    
    int currentChunk = -1;
    std::ofstream out;
    size_t recordsInCurrentChunk = 0;
    size_t currentOffset = 0;  // track manually, tellp() was slow
    
    constexpr size_t bufferSize = 1024 * 1024;
    std::vector<char> buffer(bufferSize);
    
    for (const auto& record : records) {
        if (recordsInCurrentChunk == 0) {
            if (out.is_open()) out.close();
            currentChunk++;
            currentOffset = 0;
            out.open(getChunkFileName(currentChunk), std::ios::binary);
            if (!out) throw std::runtime_error("Failed to open chunk file");
            out.rdbuf()->pubsetbuf(buffer.data(), bufferSize);
        }
        
        out.write(record.data.data(), record.data.size());
        
        // store chunk number in the recordId field
        index[record.id] = IndexEntry(currentChunk, currentOffset, record.data.size());
        recordOrder.push_back(record.id);
        currentOffset += record.data.size();
        
        recordsInCurrentChunk++;
        if (recordsInCurrentChunk >= recordsPerChunk)
            recordsInCurrentChunk = 0;
    }
    
    totalChunks = currentChunk + 1;
    if (out.is_open()) out.close();
    writeIndex();
}

std::vector<Record> ChunkedFileStrategy::readSequential() {
    readIndex();
    std::vector<Record> records;
    records.reserve(recordOrder.size());
    
    std::ifstream currentFile;
    int currentChunkId = -1;
    
    constexpr size_t bufferSize = 1024 * 1024;
    std::vector<char> iobuf(bufferSize);
    
    for (int recordId : recordOrder) {
        const auto& entry  = index[recordId];
        int chunkId = entry.recordId; // chunkId stored here
        
        if (chunkId != currentChunkId) {
            if (currentFile.is_open()) currentFile.close();
            currentFile.open(getChunkFileName(chunkId), std::ios::binary);
            if (!currentFile) throw std::runtime_error("Failed to open chunk file");
            currentFile.rdbuf()->pubsetbuf(iobuf.data(), bufferSize);
            currentChunkId = chunkId;
        }
        
        // records within a chunk are already sequential, no seek needed
        Record record(recordId, entry.size);
        currentFile.read(record.data.data(), entry.size);
        records.push_back(std::move(record));
    }
    
    return records;
}

std::vector<Record> ChunkedFileStrategy::readRandom(const std::vector<int>& indices) {
    readIndex();
    
    // sort by (chunk, offset) to minimize file switches
    std::vector<std::pair<int, size_t>> sorted;
    sorted.reserve(indices.size());
    for (size_t i = 0; i < indices.size(); ++i)
        sorted.emplace_back(indices[i], i);
    
    std::sort(sorted.begin(), sorted.end(),
              [this](const auto& a, const auto& b) {
                  const auto& ea = index[a.first];
                  const auto& eb = index[b.first];
                  if (ea.recordId != eb.recordId) return ea.recordId < eb.recordId;
                  return ea.offset < eb.offset;
              });
    
    std::vector<Record> records(indices.size());
    std::ifstream currentFile;
    int currentChunkId = -1;
    
    for (const auto& [idx, origPos] : sorted) {
        const auto& entry = index[idx];
        int chunkId = entry.recordId;
        
        if (chunkId != currentChunkId) {
            if (currentFile.is_open()) currentFile.close();
            currentFile.open(getChunkFileName(chunkId), std::ios::binary);
            if (!currentFile) throw std::runtime_error("Failed to open chunk file");
            currentChunkId = chunkId;
        }
        
        Record record(idx, entry.size);
        currentFile.seekg(entry.offset);
        currentFile.read(record.data.data(), entry.size);
        records[origPos] = std::move(record);
    }
    
    return records;
}

void ChunkedFileStrategy::writeIndex() {
    std::ofstream out(indexFile, std::ios::binary);
    if (!out) throw std::runtime_error("Failed to open index file");
    
    size_t count = recordOrder.size();
    out.write(reinterpret_cast<const char*>(&count),       sizeof(count));
    out.write(reinterpret_cast<const char*>(&totalChunks), sizeof(totalChunks));
    
    out.write(reinterpret_cast<const char*>(recordOrder.data()), count * sizeof(int));
    out.write(reinterpret_cast<const char*>(index.data()),       index.size() * sizeof(IndexEntry));
}

void ChunkedFileStrategy::readIndex() {
    std::ifstream in(indexFile, std::ios::binary);
    if (!in) throw std::runtime_error("Failed to open index file");
    
    size_t count;
    in.read(reinterpret_cast<char*>(&count),       sizeof(count));
    in.read(reinterpret_cast<char*>(&totalChunks), sizeof(totalChunks));
    
    recordOrder.resize(count);
    in.read(reinterpret_cast<char*>(recordOrder.data()), count * sizeof(int));
    
    index.resize(count);
    in.read(reinterpret_cast<char*>(index.data()), count * sizeof(IndexEntry));
}

void ChunkedFileStrategy::cleanUp() {
    for (size_t i = 0; i < totalChunks; ++i) {
        fs::remove(getChunkFileName(i));
    }
    fs::remove(indexFile);
}

size_t ChunkedFileStrategy::getDiskSpaceUsed() const {
    size_t total = 0;
    
    for (size_t i = 0; i < totalChunks; ++i) {
        std::string filename = getChunkFileName(i);
        if (fs::exists(filename)) {
            total += fs::file_size(filename);
        }
    }
    
    if (fs::exists(indexFile)) {
        total += fs::file_size(indexFile);
    }
    
    return total;
}

size_t ChunkedFileStrategy::getNumFiles() const {
    return totalChunks + 1; // chunks + index
}
