#include "sstable.h"
#include <fstream>
#include <algorithm>
#include <filesystem>

namespace sstable {

SSTable::SSTable(const std::string& path,
                 const std::vector<std::pair<std::string, std::string>>& entries,
                 int level)
    : path_(path),
      level_(level),
      size_(0),
      bloom_filter_(std::make_unique<BloomFilter>(entries.size() * 10, 3)) {
    WriteToDisk(entries);
    size_ = std::filesystem::file_size(path_);
}

SSTable::SSTable(const std::string& path)
    : path_(path),
      level_(0),
      size_(0) {
    ReadFromDisk();
    size_ = std::filesystem::file_size(path_);
}

void SSTable::WriteToDisk(const std::vector<std::pair<std::string, std::string>>& entries) {
    std::ofstream file(path_, std::ios::binary);
    if (!file) {
        throw std::runtime_error("Failed to open file for writing: " + path_);
    }

    // Write header
    const uint32_t magic = 0x53535442; // "SSTB"
    const uint32_t version = 1;
    const uint64_t num_entries = entries.size();
    
    file.write(reinterpret_cast<const char*>(&magic), sizeof(magic));
    file.write(reinterpret_cast<const char*>(&version), sizeof(version));
    file.write(reinterpret_cast<const char*>(&num_entries), sizeof(num_entries));

    // Write entries
    size_t offset = sizeof(magic) + sizeof(version) + sizeof(num_entries);
    for (const auto& entry : entries) {
        const auto& [key, value] = entry;
        uint32_t key_len = key.size();
        uint32_t value_len = value.size();

        file.write(reinterpret_cast<const char*>(&key_len), sizeof(key_len));
        file.write(reinterpret_cast<const char*>(&value_len), sizeof(value_len));
        file.write(key.data(), key_len);
        file.write(value.data(), value_len);

        bloom_filter_->Add(key);
        index_.push_back({key, static_cast<uint32_t>(offset), key_len + value_len + 8});
        offset += key_len + value_len + 8;
    }

    // Write bloom filter
    std::string bloom_data = bloom_filter_->Serialize();
    uint32_t bloom_size = bloom_data.size();
    file.write(reinterpret_cast<const char*>(&bloom_size), sizeof(bloom_size));
    file.write(bloom_data.data(), bloom_size);

    if (!index_.empty()) {
        smallest_key_ = index_.front().key;
        largest_key_ = index_.back().key;
    }
}

void SSTable::ReadFromDisk() {
    std::ifstream file(path_, std::ios::binary);
    if (!file) {
        throw std::runtime_error("Failed to open file for reading: " + path_);
    }

    // Read header
    uint32_t magic, version;
    uint64_t num_entries;
    
    file.read(reinterpret_cast<char*>(&magic), sizeof(magic));
    file.read(reinterpret_cast<char*>(&version), sizeof(version));
    file.read(reinterpret_cast<char*>(&num_entries), sizeof(num_entries));

    if (magic != 0x53535442) {
        throw std::runtime_error("Invalid SSTable file: " + path_);
    }

    // Read entries
    size_t offset = sizeof(magic) + sizeof(version) + sizeof(num_entries);
    for (uint64_t i = 0; i < num_entries; ++i) {
        uint32_t key_len, value_len;
        file.read(reinterpret_cast<char*>(&key_len), sizeof(key_len));
        file.read(reinterpret_cast<char*>(&value_len), sizeof(value_len));

        std::string key(key_len, '\0');
        std::string value(value_len, '\0');
        file.read(&key[0], key_len);
        file.read(&value[0], value_len);

        index_.push_back({key, static_cast<uint32_t>(offset), key_len + value_len + 8});
        offset += key_len + value_len + 8;
    }

    // Read bloom filter
    uint32_t bloom_size;
    file.read(reinterpret_cast<char*>(&bloom_size), sizeof(bloom_size));
    std::string bloom_data(bloom_size, '\0');
    file.read(&bloom_data[0], bloom_size);
    bloom_filter_ = BloomFilter::Deserialize(bloom_data);

    if (!index_.empty()) {
        smallest_key_ = index_.front().key;
        largest_key_ = index_.back().key;
    }
}

bool SSTable::Get(const std::string& key, std::string* value) const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!bloom_filter_->MightContain(key)) {
        return false;
    }
    return BinarySearch(key, value);
}

bool SSTable::BinarySearch(const std::string& key, std::string* value) const {
    std::ifstream file(path_, std::ios::binary);
    if (!file) {
        return false;
    }

    auto it = std::lower_bound(index_.begin(), index_.end(), key,
        [](const IndexEntry& entry, const std::string& k) {
            return entry.key < k;
        });

    if (it == index_.end() || it->key != key) {
        return false;
    }

    file.seekg(it->offset);
    uint32_t key_len, value_len;
    file.read(reinterpret_cast<char*>(&key_len), sizeof(key_len));
    file.read(reinterpret_cast<char*>(&value_len), sizeof(value_len));

    // Skip key
    file.seekg(key_len, std::ios::cur);

    // Read value
    value->resize(value_len);
    file.read(&(*value)[0], value_len);

    return true;
}

std::vector<std::pair<std::string, std::string>> SSTable::GetRange(
    const std::string& start_key,
    const std::string& end_key) const {
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<std::pair<std::string, std::string>> result;

    auto start_it = std::lower_bound(index_.begin(), index_.end(), start_key,
        [](const IndexEntry& entry, const std::string& k) {
            return entry.key < k;
        });

    auto end_it = std::upper_bound(index_.begin(), index_.end(), end_key,
        [](const std::string& k, const IndexEntry& entry) {
            return k < entry.key;
        });

    std::ifstream file(path_, std::ios::binary);
    if (!file) {
        return result;
    }

    for (auto it = start_it; it != end_it; ++it) {
        file.seekg(it->offset);
        uint32_t key_len, value_len;
        file.read(reinterpret_cast<char*>(&key_len), sizeof(key_len));
        file.read(reinterpret_cast<char*>(&value_len), sizeof(value_len));

        std::string key(key_len, '\0');
        std::string value(value_len, '\0');
        file.read(&key[0], key_len);
        file.read(&value[0], value_len);

        result.emplace_back(std::move(key), std::move(value));
    }

    return result;
}

} // namespace sstable 