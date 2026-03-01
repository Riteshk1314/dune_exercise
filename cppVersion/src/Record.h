#pragma once
#include <vector>
#include <cstdint>

struct Record {
    int id;
    std::vector<char> data;
    
    Record() : id(0) {}
    Record(int id, size_t size) : id(id), data(size) {}
};

// used by SingleFile and Chunked strategies for their on-disk index
struct IndexEntry {
    int recordId;
    size_t offset;
    size_t size;
    
    IndexEntry() : recordId(0), offset(0), size(0) {}
    IndexEntry(int id, size_t off, size_t sz) 
        : recordId(id), offset(off), size(sz) {}
};
