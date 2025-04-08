#include "memtable.h"
#include "skip_list.h"
#include <cstring>

namespace sstable {

MemTable::MemTable(size_t max_size)
    : skip_list_(std::make_unique<SkipList>()),
      max_size_(max_size),
      current_size_(0) {}

bool MemTable::Put(const std::string& key, const std::string& value) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (IsFull()) {
        return false;
    }

    size_t entry_size = key.size() + value.size() + 8; // 8 bytes for lengths
    if (current_size_ + entry_size > max_size_) {
        return false;
    }

    if (skip_list_->Insert(key, value)) {
        current_size_ += entry_size;
        return true;
    }
    return false;
}

bool MemTable::Get(const std::string& key, std::string* value) const {
    std::lock_guard<std::mutex> lock(mutex_);
    return skip_list_->Get(key, value);
}

bool MemTable::Delete(const std::string& key) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (IsFull()) {
        return false;
    }

    // For deletion, we insert a tombstone value
    const std::string tombstone = "";
    size_t entry_size = key.size() + 8; // 8 bytes for lengths
    if (current_size_ + entry_size > max_size_) {
        return false;
    }

    if (skip_list_->Insert(key, tombstone)) {
        current_size_ += entry_size;
        return true;
    }
    return false;
}

bool MemTable::IsFull() const {
    return current_size_ >= max_size_;
}

size_t MemTable::GetSize() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return current_size_;
}

std::vector<std::pair<std::string, std::string>> MemTable::GetAllEntries() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return skip_list_->GetAllEntries();
}

} // namespace sstable 