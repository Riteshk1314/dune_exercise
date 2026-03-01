#pragma once
#include "Record.h"
#include <vector>
#include <cstdint>

class DataValidator {
public:
    static uint32_t computeChecksum(const Record& record);
    
    static bool verifyRecords(const std::vector<Record>& original, 
                             const std::vector<Record>& read);
    
    static bool verifySubset(const std::vector<Record>& original,
                            const std::vector<Record>& read,
                            const std::vector<int>& indices);
};
