#pragma once

#include <string>
#include <vector>
#include <memory>
#include <fstream>
#include <map>
#include <mutex>
#include "bloom_filter.h"

namespace sstable {

/**
 * @brief SSTable (Sorted String Table) is an immutable file that stores sorted key-value pairs.
 * 
 * SSTables are created when MemTables are flushed to disk. They support efficient point lookups
 * and range scans. Each SSTable includes a bloom filter for quick existence checks.
 */
class SSTable {
public:
    /**
     * @brief Construct a new SSTable from a MemTable
     * 
     * @param path Directory where the SSTable file will be stored
     * @param entries Vector of key-value pairs to store
     * @param level The level in the LSM tree where this SSTable belongs
     */
    SSTable(const std::string& path,
            const std::vector<std::pair<std::string, std::string>>& entries,
            int level);

    /**
     * @brief Load an existing SSTable from disk
     * 
     * @param path Path to the SSTable file
     */
    explicit SSTable(const std::string& path);

    /**
     * @brief Get the value associated with a key
     * 
     * @param key The key to look up
     * @param value Output parameter for the value
     * @return true if the key was found
     * @return false if the key was not found
     */
    bool Get(const std::string& key, std::string* value) const;

    /**
     * @brief Get all key-value pairs in a range
     * 
     * @param start_key Start of the range (inclusive)
     * @param end_key End of the range (inclusive)
     * @return std::vector<std::pair<std::string, std::string>> Vector of key-value pairs
     */
    std::vector<std::pair<std::string, std::string>> GetRange(
        const std::string& start_key,
        const std::string& end_key) const;

    /**
     * @brief Get the file path of this SSTable
     * 
     * @return std::string The file path
     */
    std::string GetPath() const { return path_; }

    /**
     * @brief Get the level of this SSTable in the LSM tree
     * 
     * @return int The level
     */
    int GetLevel() const { return level_; }

    /**
     * @brief Get the size of this SSTable in bytes
     * 
     * @return size_t The size
     */
    size_t GetSize() const { return size_; }

    /**
     * @brief Get the smallest key in this SSTable
     * 
     * @return std::string The smallest key
     */
    std::string GetSmallestKey() const { return smallest_key_; }

    /**
     * @brief Get the largest key in this SSTable
     * 
     * @return std::string The largest key
     */
    std::string GetLargestKey() const { return largest_key_; }

private:
    struct IndexEntry {
        std::string key;
        uint32_t offset;
        uint32_t size;
    };

    void WriteToDisk(const std::vector<std::pair<std::string, std::string>>& entries);
    void ReadFromDisk();
    void BuildIndex();
    bool BinarySearch(const std::string& key, std::string* value) const;

    std::string path_;
    int level_;
    size_t size_;
    std::string smallest_key_;
    std::string largest_key_;
    std::vector<IndexEntry> index_;
    std::unique_ptr<BloomFilter> bloom_filter_;
    mutable std::mutex mutex_;
};

} // namespace sstable 