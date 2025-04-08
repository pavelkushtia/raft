#include "lsm_tree.h"
#include <filesystem>
#include <algorithm>

namespace sstable {

LSMTree::LSMTree(const std::string& base_path, size_t memtable_size)
    : base_path_(base_path),
      memtable_(std::make_unique<MemTable>(memtable_size)),
      compaction_(std::make_unique<Compaction>(base_path)) {
    std::filesystem::create_directories(base_path);
    LoadExistingSSTables();
}

bool LSMTree::Put(const std::string& key, const std::string& value) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (CheckMemTableFull()) {
        SwitchMemTable();
    }
    
    return memtable_->Put(key, value);
}

bool LSMTree::Get(const std::string& key, std::string* value) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    // Check MemTable first
    if (memtable_->Get(key, value)) {
        return true;
    }
    
    // Check immutable MemTable if it exists
    if (immutable_memtable_ && immutable_memtable_->Get(key, value)) {
        return true;
    }
    
    // Check SSTables from newest to oldest
    for (auto level_it = levels_.rbegin(); level_it != levels_.rend(); ++level_it) {
        for (auto table_it = level_it->second.rbegin(); table_it != level_it->second.rend(); ++table_it) {
            if ((*table_it)->Get(key, value)) {
                return true;
            }
        }
    }
    
    return false;
}

bool LSMTree::Delete(const std::string& key) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (CheckMemTableFull()) {
        SwitchMemTable();
    }
    
    return memtable_->Delete(key);
}

std::vector<std::pair<std::string, std::string>> LSMTree::GetRange(
    const std::string& start_key,
    const std::string& end_key) {
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<std::pair<std::string, std::string>> result;
    
    // Get entries from MemTable
    auto memtable_entries = memtable_->GetAllEntries();
    result.insert(result.end(), memtable_entries.begin(), memtable_entries.end());
    
    // Get entries from immutable MemTable if it exists
    if (immutable_memtable_) {
        auto immutable_entries = immutable_memtable_->GetAllEntries();
        result.insert(result.end(), immutable_entries.begin(), immutable_entries.end());
    }
    
    // Get entries from SSTables
    for (const auto& [level, tables] : levels_) {
        for (const auto& table : tables) {
            auto table_entries = table->GetRange(start_key, end_key);
            result.insert(result.end(), table_entries.begin(), table_entries.end());
        }
    }
    
    // Sort and remove duplicates
    std::sort(result.begin(), result.end());
    result.erase(std::unique(result.begin(), result.end()), result.end());
    
    return result;
}

void LSMTree::FlushMemTable() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!immutable_memtable_) {
        return;
    }
    
    // Create new SSTable from immutable MemTable
    auto entries = immutable_memtable_->GetAllEntries();
    auto new_table = std::make_unique<SSTable>(
        compaction_->GenerateOutputPath(0),
        entries,
        0);
    
    // Add to level 0
    AddSSTable(std::move(new_table));
    immutable_memtable_.reset();
    
    // Check if compaction is needed
    MaybeCompact();
}

void LSMTree::MaybeCompact() {
    // Check each level for compaction
    for (auto& [level, tables] : levels_) {
        if (compaction_->ShouldCompact(tables, level)) {
            // Select tables to compact
            size_t num_tables = std::min(static_cast<size_t>(10), tables.size());
            std::vector<std::unique_ptr<SSTable>> to_compact;
            to_compact.reserve(num_tables);
            
            for (size_t i = 0; i < num_tables; ++i) {
                to_compact.push_back(std::move(tables[i]));
            }
            tables.erase(tables.begin(), tables.begin() + num_tables);
            
            // Compact and add to next level
            auto new_table = compaction_->Compact(to_compact, level + 1);
            AddSSTable(std::move(new_table));
        }
    }
}

void LSMTree::LoadExistingSSTables() {
    for (const auto& entry : std::filesystem::directory_iterator(base_path_)) {
        if (entry.path().extension() == ".sst") {
            auto table = std::make_unique<SSTable>(entry.path().string());
            AddSSTable(std::move(table));
        }
    }
}

void LSMTree::AddSSTable(std::unique_ptr<SSTable> table) {
    int level = table->GetLevel();
    levels_[level].push_back(std::move(table));
}

void LSMTree::RemoveSSTable(const std::string& path) {
    for (auto& [level, tables] : levels_) {
        auto it = std::find_if(tables.begin(), tables.end(),
            [&path](const std::unique_ptr<SSTable>& table) {
                return table->GetPath() == path;
            });
        
        if (it != tables.end()) {
            tables.erase(it);
            return;
        }
    }
}

bool LSMTree::CheckMemTableFull() {
    return memtable_->IsFull();
}

void LSMTree::SwitchMemTable() {
    immutable_memtable_ = std::move(memtable_);
    memtable_ = std::make_unique<MemTable>(memtable_->GetSize());
}

} // namespace sstable 