#include "compaction.h"
#include <algorithm>
#include <filesystem>
#include <sstream>

namespace sstable {

Compaction::Compaction(const std::string& base_path)
    : base_path_(base_path) {
    std::filesystem::create_directories(base_path);
}

std::unique_ptr<SSTable> Compaction::Compact(
    const std::vector<std::unique_ptr<SSTable>>& input_tables,
    int output_level) {
    // Merge all entries from input tables
    auto merged_entries = MergeTables(input_tables);
    
    // Remove duplicates and tombstones
    RemoveDuplicates(&merged_entries);
    
    // Create new SSTable
    std::string output_path = GenerateOutputPath(output_level);
    return std::make_unique<SSTable>(output_path, merged_entries, output_level);
}

bool Compaction::ShouldCompact(
    const std::vector<std::unique_ptr<SSTable>>& tables,
    int level) const {
    if (tables.empty()) {
        return false;
    }

    // Check if total size exceeds level's maximum size
    size_t total_size = 0;
    for (const auto& table : tables) {
        total_size += table->GetSize();
    }

    return total_size > GetMaxSizeForLevel(level);
}

size_t Compaction::GetMaxSizeForLevel(int level) {
    return static_cast<size_t>(kBaseLevelSize * std::pow(kLevelSizeMultiplier, level));
}

std::vector<Compaction::KeyValue> Compaction::MergeTables(
    const std::vector<std::unique_ptr<SSTable>>& tables) {
    std::vector<KeyValue> result;
    
    // Collect all entries from all tables
    for (const auto& table : tables) {
        auto entries = table->GetRange(table->GetSmallestKey(), table->GetLargestKey());
        for (const auto& [key, value] : entries) {
            result.push_back({key, value, value.empty(), 0});
        }
    }
    
    // Sort by key
    std::sort(result.begin(), result.end(),
        [](const KeyValue& a, const KeyValue& b) {
            return a.key < b.key;
        });
    
    return result;
}

void Compaction::RemoveDuplicates(std::vector<KeyValue>* entries) {
    if (entries->empty()) {
        return;
    }

    std::vector<KeyValue> result;
    result.push_back(entries->front());
    
    for (size_t i = 1; i < entries->size(); ++i) {
        const auto& current = (*entries)[i];
        const auto& last = result.back();
        
        if (current.key != last.key) {
            result.push_back(current);
        } else if (!current.is_tombstone) {
            // Keep the latest non-tombstone value
            result.back() = current;
        }
    }
    
    // Remove tombstones at the end
    while (!result.empty() && result.back().is_tombstone) {
        result.pop_back();
    }
    
    *entries = std::move(result);
}

std::string Compaction::GenerateOutputPath(int level) const {
    std::stringstream ss;
    ss << base_path_ << "/level-" << level << "/sstable-"
       << std::chrono::system_clock::now().time_since_epoch().count() << ".sst";
    return ss.str();
}

} // namespace sstable 