#pragma once

#include <string>
#include <vector>
#include <memory>
#include "sstable.h"

namespace sstable {

/**
 * @brief Compaction manages the process of merging SSTables to maintain efficiency.
 * 
 * Compaction is responsible for:
 * 1. Selecting SSTables to compact
 * 2. Merging SSTables while removing duplicate keys and tombstones
 * 3. Creating new SSTables at the appropriate level
 */
class Compaction {
public:
    /**
     * @brief Construct a new Compaction object
     * 
     * @param base_path Base directory for SSTable files
     */
    explicit Compaction(const std::string& base_path);

    /**
     * @brief Compact a set of SSTables
     * 
     * @param input_tables SSTables to compact
     * @param output_level Level for the new SSTable
     * @return std::unique_ptr<SSTable> The new compacted SSTable
     */
    std::unique_ptr<SSTable> Compact(
        const std::vector<std::unique_ptr<SSTable>>& input_tables,
        int output_level);

    /**
     * @brief Check if compaction is needed for a set of SSTables
     * 
     * @param tables SSTables to check
     * @param level Current level of the SSTables
     * @return true if compaction is needed
     * @return false if compaction is not needed
     */
    bool ShouldCompact(const std::vector<std::unique_ptr<SSTable>>& tables,
                      int level) const;

    /**
     * @brief Get the maximum size for SSTables at a given level
     * 
     * @param level The level to check
     * @return size_t Maximum size in bytes
     */
    static size_t GetMaxSizeForLevel(int level);

private:
    struct KeyValue {
        std::string key;
        std::string value;
        bool is_tombstone;
        size_t sequence_number;
    };

    std::vector<KeyValue> MergeTables(
        const std::vector<std::unique_ptr<SSTable>>& tables);
    void RemoveDuplicates(std::vector<KeyValue>* entries);
    std::string GenerateOutputPath(int level) const;

    std::string base_path_;
    static constexpr size_t kBaseLevelSize = 2 * 1024 * 1024; // 2MB
    static constexpr double kLevelSizeMultiplier = 10.0;
};

} // namespace sstable 