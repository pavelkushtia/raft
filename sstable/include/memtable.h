#pragma once

#include <string>
#include <memory>
#include <vector>
#include <map>
#include <mutex>

namespace sstable {

// Forward declaration
class SkipList;

/**
 * @brief MemTable is an in-memory buffer that stores recent writes before they are flushed to SSTables.
 * 
 * The MemTable uses a skip list for efficient insertion and lookup operations. When the MemTable
 * reaches a certain size threshold, it is flushed to disk as an SSTable.
 */
class MemTable {
public:
    /**
     * @brief Construct a new MemTable object
     * 
     * @param max_size Maximum size in bytes before the MemTable is flushed
     */
    explicit MemTable(size_t max_size = 64 * 1024 * 1024); // Default 64MB

    /**
     * @brief Insert a key-value pair into the MemTable
     * 
     * @param key The key to insert
     * @param value The value to insert
     * @return true if the insertion was successful
     * @return false if the MemTable is full
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
    bool Get(const std::string& key, std::string* value) const;

    /**
     * @brief Delete a key from the MemTable
     * 
     * @param key The key to delete
     * @return true if the deletion was successful
     */
    bool Delete(const std::string& key);

    /**
     * @brief Check if the MemTable is full
     * 
     * @return true if the MemTable has reached its size limit
     */
    bool IsFull() const;

    /**
     * @brief Get the current size of the MemTable in bytes
     * 
     * @return size_t The current size
     */
    size_t GetSize() const;

    /**
     * @brief Get all key-value pairs in the MemTable
     * 
     * @return std::vector<std::pair<std::string, std::string>> Vector of key-value pairs
     */
    std::vector<std::pair<std::string, std::string>> GetAllEntries() const;

private:
    std::unique_ptr<SkipList> skip_list_;
    size_t max_size_;
    size_t current_size_;
    mutable std::mutex mutex_;
};

} // namespace sstable 