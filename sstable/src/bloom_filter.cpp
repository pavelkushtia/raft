#include "bloom_filter.h"
#include <functional>
#include <sstream>

namespace sstable {

BloomFilter::BloomFilter(size_t size, size_t num_hashes)
    : bits_((size + 63) / 64),  // Round up to nearest 64-bit block
      num_hashes_(num_hashes) {
    InitializeHashFunctions();
}

void BloomFilter::InitializeHashFunctions() {
    // Use different seeds for each hash function
    for (size_t i = 0; i < num_hashes_; ++i) {
        hash_functions_.push_back(
            [i](const std::string& key) {
                size_t hash = 5381;
                for (char c : key) {
                    hash = ((hash << 5) + hash) + c + i;
                }
                return hash;
            }
        );
    }
}

void BloomFilter::Add(const std::string& key) {
    for (size_t i = 0; i < num_hashes_; ++i) {
        size_t hash = hash_functions_[i](key);
        size_t bit_index = hash % (bits_.size() * 64);
        size_t block_index = bit_index / 64;
        size_t bit_offset = bit_index % 64;
        bits_[block_index].set(bit_offset);
    }
}

bool BloomFilter::MightContain(const std::string& key) const {
    for (size_t i = 0; i < num_hashes_; ++i) {
        size_t hash = hash_functions_[i](key);
        size_t bit_index = hash % (bits_.size() * 64);
        size_t block_index = bit_index / 64;
        size_t bit_offset = bit_index % 64;
        if (!bits_[block_index].test(bit_offset)) {
            return false;
        }
    }
    return true;
}

std::string BloomFilter::Serialize() const {
    std::stringstream ss;
    ss.write(reinterpret_cast<const char*>(&num_hashes_), sizeof(num_hashes_));
    size_t num_blocks = bits_.size();
    ss.write(reinterpret_cast<const char*>(&num_blocks), sizeof(num_blocks));
    for (const auto& block : bits_) {
        uint64_t value = block.to_ullong();
        ss.write(reinterpret_cast<const char*>(&value), sizeof(value));
    }
    return ss.str();
}

std::unique_ptr<BloomFilter> BloomFilter::Deserialize(const std::string& data) {
    std::stringstream ss(data);
    size_t num_hashes, num_blocks;
    ss.read(reinterpret_cast<char*>(&num_hashes), sizeof(num_hashes));
    ss.read(reinterpret_cast<char*>(&num_blocks), sizeof(num_blocks));
    
    auto filter = std::make_unique<BloomFilter>(num_blocks * 64, num_hashes);
    for (size_t i = 0; i < num_blocks; ++i) {
        uint64_t value;
        ss.read(reinterpret_cast<char*>(&value), sizeof(value));
        filter->bits_[i] = std::bitset<64>(value);
    }
    return filter;
}

} // namespace sstable 