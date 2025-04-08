#include "compaction.h"
#include "sstable.h"
#include <gtest/gtest.h>
#include <string>
#include <filesystem>
#include <vector>

using namespace sstable;

class CompactionTest : public ::testing::Test {
protected:
    void SetUp() override {
        test_dir_ = "/tmp/compaction_test";
        std::filesystem::create_directories(test_dir_);
        compaction_ = std::make_unique<Compaction>(test_dir_);
    }
    
    void TearDown() override {
        std::filesystem::remove_all(test_dir_);
    }
    
    std::string test_dir_;
    std::unique_ptr<Compaction> compaction_;
};

TEST_F(CompactionTest, BasicCompaction) {
    // Create input SSTables
    std::vector<std::unique_ptr<SSTable>> input_tables;
    
    // First SSTable
    std::vector<std::pair<std::string, std::string>> entries1 = {
        {"key1", "value1"},
        {"key3", "value3"}
    };
    input_tables.push_back(std::make_unique<SSTable>(
        test_dir_ + "/table1.sst",
        entries1,
        0));
    
    // Second SSTable
    std::vector<std::pair<std::string, std::string>> entries2 = {
        {"key2", "value2"},
        {"key4", "value4"}
    };
    input_tables.push_back(std::make_unique<SSTable>(
        test_dir_ + "/table2.sst",
        entries2,
        0));
    
    // Compact to level 1
    auto new_table = compaction_->Compact(input_tables, 1);
    
    // Verify the compacted table
    std::string value;
    EXPECT_TRUE(new_table->Get("key1", &value));
    EXPECT_EQ(value, "value1");
    EXPECT_TRUE(new_table->Get("key2", &value));
    EXPECT_EQ(value, "value2");
    EXPECT_TRUE(new_table->Get("key3", &value));
    EXPECT_EQ(value, "value3");
    EXPECT_TRUE(new_table->Get("key4", &value));
    EXPECT_EQ(value, "value4");
}

TEST_F(CompactionTest, DuplicateKeys) {
    // Create input SSTables with duplicate keys
    std::vector<std::unique_ptr<SSTable>> input_tables;
    
    // First SSTable
    std::vector<std::pair<std::string, std::string>> entries1 = {
        {"key1", "value1"},
        {"key2", "value2"}
    };
    input_tables.push_back(std::make_unique<SSTable>(
        test_dir_ + "/table1.sst",
        entries1,
        0));
    
    // Second SSTable with duplicate key
    std::vector<std::pair<std::string, std::string>> entries2 = {
        {"key2", "value2_new"},
        {"key3", "value3"}
    };
    input_tables.push_back(std::make_unique<SSTable>(
        test_dir_ + "/table2.sst",
        entries2,
        0));
    
    // Compact to level 1
    auto new_table = compaction_->Compact(input_tables, 1);
    
    // Verify the compacted table
    std::string value;
    EXPECT_TRUE(new_table->Get("key1", &value));
    EXPECT_EQ(value, "value1");
    EXPECT_TRUE(new_table->Get("key2", &value));
    EXPECT_EQ(value, "value2_new"); // Should have the newer value
    EXPECT_TRUE(new_table->Get("key3", &value));
    EXPECT_EQ(value, "value3");
}

TEST_F(CompactionTest, TombstoneHandling) {
    // Create input SSTables with tombstones
    std::vector<std::unique_ptr<SSTable>> input_tables;
    
    // First SSTable
    std::vector<std::pair<std::string, std::string>> entries1 = {
        {"key1", "value1"},
        {"key2", "value2"}
    };
    input_tables.push_back(std::make_unique<SSTable>(
        test_dir_ + "/table1.sst",
        entries1,
        0));
    
    // Second SSTable with tombstone
    std::vector<std::pair<std::string, std::string>> entries2 = {
        {"key2", ""}, // Tombstone
        {"key3", "value3"}
    };
    input_tables.push_back(std::make_unique<SSTable>(
        test_dir_ + "/table2.sst",
        entries2,
        0));
    
    // Compact to level 1
    auto new_table = compaction_->Compact(input_tables, 1);
    
    // Verify the compacted table
    std::string value;
    EXPECT_TRUE(new_table->Get("key1", &value));
    EXPECT_EQ(value, "value1");
    EXPECT_FALSE(new_table->Get("key2", &value)); // Should be deleted
    EXPECT_TRUE(new_table->Get("key3", &value));
    EXPECT_EQ(value, "value3");
}

TEST_F(CompactionTest, ShouldCompact) {
    // Create a large SSTable
    std::vector<std::pair<std::string, std::string>> entries;
    for (int i = 0; i < 1000; ++i) {
        entries.emplace_back("key" + std::to_string(i), "value" + std::to_string(i));
    }
    
    std::vector<std::unique_ptr<SSTable>> tables;
    tables.push_back(std::make_unique<SSTable>(
        test_dir_ + "/table.sst",
        entries,
        0));
    
    // Check if compaction is needed
    EXPECT_TRUE(compaction_->ShouldCompact(tables, 0));
}

TEST_F(CompactionTest, LevelSizeCalculation) {
    EXPECT_EQ(Compaction::GetMaxSizeForLevel(0), 2 * 1024 * 1024); // 2MB
    EXPECT_EQ(Compaction::GetMaxSizeForLevel(1), 20 * 1024 * 1024); // 20MB
    EXPECT_EQ(Compaction::GetMaxSizeForLevel(2), 200 * 1024 * 1024); // 200MB
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
} 