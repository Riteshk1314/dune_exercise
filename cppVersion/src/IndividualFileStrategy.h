#pragma once
#include "StorageStrategy.h"
#include <vector>

// One file per record. Splits into subdirs to avoid huge flat directories.
class IndividualFileStrategy : public StorageStrategy {
public:
    IndividualFileStrategy(const std::string& dir);
    
    void write(const std::vector<Record>& records) override;
    std::vector<Record> readSequential() override;
    std::vector<Record> readRandom(const std::vector<int>& indices) override;
    void cleanUp() override;
    std::string getName() const override { return "Individual"; }
    
    size_t getDiskSpaceUsed() const override;
    size_t getNumFiles() const override { return totalRecords; }
    
private:
    size_t totalRecords;
    std::vector<size_t> recordSizes; // cached at write time
    
    std::string getRecordFileName(int recordId) const;
    void ensureSubdirectories(size_t numRecords);
};
