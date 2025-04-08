#pragma once

#include <string>
#include <vector>
#include <memory>
#include <map>
#include <mutex>
#include "memtable.h"
#include "sstable.h"
#include "compaction.h"

namespace sstable {

/**
 * @brief LSMTree implements a Log-Structured Merge Tree storage engine.
 * 
 * The LSM Tree consists of:
 * 1. An in-memory MemTable for recent writes
 * 2. Multiple levels of SSTables on disk
 * 3. A compaction manager to maintain efficiency
 */
class LSMTree {
public:
    /**
     * @brief Construct a new LSMTree object
     * 
     * @param base_path Directory where SSTables will be stored
     * @param memtable_size Maximum size of the MemTable in bytes
     */
    explicit LSMTree(const std::string& base_path,
                    size_t memtable_size = 64 * 1024 * 1024); // Default 64MB

    /**
     * @brief Insert a key-value pair
     * 
     * @param key The key to insert
     * @param value The value to insert
     * @return true if the insertion was successful
     */
    bool Put(const std::string& key, const std::string& value);

    /**
     * @brief Get the value associated with a key
     * 
     * @param key The key to look up
     * @param value Output parameter for the value
     * @return true if the key was found
     * @return false if the key was not found
     */
    bool Get(const std::string& key, std::string* value);

    /**
     * @brief Delete a key
     * 
     * @param key The key to delete
     * @return true if the deletion was successful
     */
    bool Delete(const std::string& key);

    /**
     * @brief Get all key-value pairs in a range
     * 
     * @param start_key Start of the range (inclusive)
     * @param end_key End of the range (inclusive)
     * @return std::vector<std::pair<std::string, std::string>> Vector of key-value pairs
     */
    std::vector<std::pair<std::string, std::string>> GetRange(
        const std::string& start_key,
        const std::string& end_key);

    /**
     * @brief Flush the current MemTable to disk
     * 
     * This is called automatically when the MemTable is full,
     * but can also be called manually.
     */
    void FlushMemTable();

    /**
     * @brief Perform compaction if needed
     * 
     * This is called automatically after flushing the MemTable,
     * but can also be called manually.
     */
    void MaybeCompact();

private:
    void LoadExistingSSTables();
    void AddSSTable(std::unique_ptr<SSTable> table);
    void RemoveSSTable(const std::string& path);
    bool CheckMemTableFull();
    void SwitchMemTable();

    std::string base_path_;
    std::unique_ptr<MemTable> memtable_;
    std::unique_ptr<MemTable> immutable_memtable_;
    std::map<int, std::vector<std::unique_ptr<SSTable>>> levels_;
    std::unique_ptr<Compaction> compaction_;
    mutable std::mutex mutex_;
};

} // namespace sstable 