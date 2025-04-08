#include "sstable.h"
#include <gtest/gtest.h>
#include <string>
#include <filesystem>
#include <vector>

using namespace sstable;

class SSTableTest : public ::testing::Test {
protected:
    void SetUp() override {
        test_dir_ = "/tmp/sstable_test";
        std::filesystem::create_directories(test_dir_);
    }
    
    void TearDown() override {
        std::filesystem::remove_all(test_dir_);
    }
    
    std::string test_dir_;
};

TEST_F(SSTableTest, CreateAndRead) {
    std::vector<std::pair<std::string, std::string>> entries = {
        {"key1", "value1"},
        {"key2", "value2"},
        {"key3", "value3"}
    };
    
    std::string path = test_dir_ + "/test.sst";
    SSTable sstable(path, entries, 0);
    
    std::string value;
    EXPECT_TRUE(sstable.Get("key1", &value));
    EXPECT_EQ(value, "value1");
    
    EXPECT_TRUE(sstable.Get("key2", &value));
    EXPECT_EQ(value, "value2");
    
    EXPECT_TRUE(sstable.Get("key3", &value));
    EXPECT_EQ(value, "value3");
}

TEST_F(SSTableTest, RangeQuery) {
    std::vector<std::pair<std::string, std::string>> entries = {
        {"key1", "value1"},
        {"key2", "value2"},
        {"key3", "value3"},
        {"key4", "value4"},
        {"key5", "value5"}
    };
    
    std::string path = test_dir_ + "/test.sst";
    SSTable sstable(path, entries, 0);
    
    auto range = sstable.GetRange("key2", "key4");
    EXPECT_EQ(range.size(), 3);
    
    EXPECT_EQ(range[0].first, "key2");
    EXPECT_EQ(range[0].second, "value2");
    EXPECT_EQ(range[1].first, "key3");
    EXPECT_EQ(range[1].second, "value3");
    EXPECT_EQ(range[2].first, "key4");
    EXPECT_EQ(range[2].second, "value4");
}

TEST_F(SSTableTest, NonExistentKey) {
    std::vector<std::pair<std::string, std::string>> entries = {
        {"key1", "value1"}
    };
    
    std::string path = test_dir_ + "/test.sst";
    SSTable sstable(path, entries, 0);
    
    std::string value;
    EXPECT_FALSE(sstable.Get("nonexistent", &value));
}

TEST_F(SSTableTest, BloomFilter) {
    std::vector<std::pair<std::string, std::string>> entries;
    for (int i = 0; i < 1000; ++i) {
        entries.emplace_back("key" + std::to_string(i), "value" + std::to_string(i));
    }
    
    std::string path = test_dir_ + "/test.sst";
    SSTable sstable(path, entries, 0);
    
    // Test with a key that definitely doesn't exist
    std::string value;
    EXPECT_FALSE(sstable.Get("nonexistent_key", &value));
}

TEST_F(SSTableTest, LargeEntries) {
    std::vector<std::pair<std::string, std::string>> entries;
    std::string large_value(1024, 'x'); // 1KB value
    
    for (int i = 0; i < 100; ++i) {
        entries.emplace_back("key" + std::to_string(i), large_value);
    }
    
    std::string path = test_dir_ + "/test.sst";
    SSTable sstable(path, entries, 0);
    
    std::string value;
    EXPECT_TRUE(sstable.Get("key50", &value));
    EXPECT_EQ(value, large_value);
}

TEST_F(SSTableTest, Metadata) {
    std::vector<std::pair<std::string, std::string>> entries = {
        {"key1", "value1"},
        {"key2", "value2"}
    };
    
    std::string path = test_dir_ + "/test.sst";
    SSTable sstable(path, entries, 1);
    
    EXPECT_EQ(sstable.GetLevel(), 1);
    EXPECT_GT(sstable.GetSize(), 0);
    EXPECT_EQ(sstable.GetSmallestKey(), "key1");
    EXPECT_EQ(sstable.GetLargestKey(), "key2");
}

TEST_F(SSTableTest, LoadExisting) {
    // Create an SSTable
    std::vector<std::pair<std::string, std::string>> entries = {
        {"key1", "value1"},
        {"key2", "value2"}
    };
    
    std::string path = test_dir_ + "/test.sst";
    {
        SSTable sstable(path, entries, 0);
    }
    
    // Load the existing SSTable
    SSTable loaded_sstable(path);
    
    std::string value;
    EXPECT_TRUE(loaded_sstable.Get("key1", &value));
    EXPECT_EQ(value, "value1");
    
    EXPECT_TRUE(loaded_sstable.Get("key2", &value));
    EXPECT_EQ(value, "value2");
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
} 