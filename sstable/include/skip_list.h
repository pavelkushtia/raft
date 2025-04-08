#pragma once

#include <string>
#include <memory>
#include <random>
#include <vector>

namespace sstable {

/**
 * @brief SkipList is a probabilistic data structure that allows for efficient
 * search, insertion, and deletion operations in O(log n) time.
 * 
 * The SkipList is used by the MemTable to store key-value pairs in sorted order.
 */
class SkipList {
public:
    /**
     * @brief Construct a new Skip List
     */
    SkipList();

    /**
     * @brief Insert a key-value pair
     * 
     * @param key The key to insert
     * @param value The value to insert
     * @return true if the insertion was successful
     */
    bool Insert(const std::string& key, const std::string& value);

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
     * @brief Delete a key
     * 
     * @param key The key to delete
     * @return true if the deletion was successful
     */
    bool Delete(const std::string& key);

    /**
     * @brief Get all key-value pairs in sorted order
     * 
     * @return std::vector<std::pair<std::string, std::string>> Vector of key-value pairs
     */
    std::vector<std::pair<std::string, std::string>> GetAllEntries() const;

private:
    struct Node {
        std::string key;
        std::string value;
        std::vector<std::shared_ptr<Node>> forward;

        Node(const std::string& k, const std::string& v, int level)
            : key(k), value(v), forward(level + 1) {}
    };

    static constexpr int kMaxLevel = 16;
    static constexpr float kProbability = 0.5;

    int RandomLevel();
    std::shared_ptr<Node> FindGreaterOrEqual(
        const std::string& key,
        std::vector<std::shared_ptr<Node>>* prev) const;

    std::shared_ptr<Node> head_;
    int max_level_;
    std::mt19937 rng_;
    std::uniform_real_distribution<float> dist_;
};

} // namespace sstable 