#pragma once

#include <string>
#include <vector>
#include <bitset>
#include <functional>

namespace sstable {

/**
 * @brief BloomFilter is a space-efficient probabilistic data structure for membership testing.
 * 
 * It can tell you definitively if an element is not in the set, or if it might be in the set.
 * False positives are possible, but false negatives are not.
 */
class BloomFilter {
public:
    /**
     * @brief Construct a new Bloom Filter
     * 
     * @param size Number of bits in the filter
     * @param num_hashes Number of hash functions to use
     */
    BloomFilter(size_t size, size_t num_hashes);

    /**
     * @brief Add an element to the filter
     * 
     * @param key The element to add
     */
    void Add(const std::string& key);

    /**
     * @brief Check if an element might be in the filter
     * 
     * @param key The element to check
     * @return true if the element might be in the filter
     * @return false if the element is definitely not in the filter
     */
    bool MightContain(const std::string& key) const;

    /**
     * @brief Serialize the filter to a string
     * 
     * @return std::string Serialized filter
     */
    std::string Serialize() const;

    /**
     * @brief Deserialize a filter from a string
     * 
     * @param data Serialized filter
     * @return std::unique_ptr<BloomFilter> Deserialized filter
     */
    static std::unique_ptr<BloomFilter> Deserialize(const std::string& data);

private:
    std::vector<std::bitset<64>> bits_;
    size_t num_hashes_;
    std::vector<std::function<size_t(const std::string&)>> hash_functions_;

    void InitializeHashFunctions();
    size_t Hash(const std::string& key, size_t seed) const;
};

} // namespace sstable 