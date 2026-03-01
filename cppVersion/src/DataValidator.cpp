#include "DataValidator.h"
#include <iostream>

uint32_t DataValidator::computeChecksum(const Record& record) {
    uint32_t checksum = 0;
    for (size_t i = 0; i < record.data.size(); ++i) {
        checksum = (checksum << 1) ^ static_cast<uint8_t>(record.data[i]);
    }
    return checksum ^ record.id;
}

bool DataValidator::verifyRecords(const std::vector<Record>& original, 
                                  const std::vector<Record>& read) {
    if (original.size() != read.size()) {
        std::cerr << "record count mismatch: wrote " << original.size()
                  << " read back " << read.size() << std::endl;
        return false;
    }
    
    for (size_t i = 0; i < original.size(); ++i) {
        if (original[i].id != read[i].id) {
            std::cerr << "ID mismatch at position " << i
                      << " (expected " << original[i].id
                      << ", got " << read[i].id << ")" << std::endl;
            return false;
        }
        
        if (original[i].data.size() != read[i].data.size()) {
            std::cerr << "size mismatch for record " << original[i].id << std::endl;
            return false;
        }
        
        if (original[i].data != read[i].data) {
            std::cerr << "data mismatch for record " << original[i].id << std::endl;
            return false;
        }
    }
    
    return true;
}

bool DataValidator::verifySubset(const std::vector<Record>& original,
                                const std::vector<Record>& read,
                                const std::vector<int>& indices) {
    if (indices.size() != read.size()) {
        std::cerr << "subset size mismatch" << std::endl;
        return false;
    }
    
    for (size_t i = 0; i < indices.size(); ++i) {
        int idx = indices[i];
        const auto& orig = original[idx];
        const auto& rd   = read[i];
        
        if (orig.id != rd.id || orig.data != rd.data) {
            std::cerr << "data mismatch for record " << idx << std::endl;
            return false;
        }
    }
    
    return true;
}
