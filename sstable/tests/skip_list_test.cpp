#include "skip_list.h"
#include <gtest/gtest.h>
#include <string>
#include <vector>
#include <algorithm>

using namespace sstable;

class SkipListTest : public ::testing::Test {
protected:
    void SetUp() override {
        skip_list_ = std::make_unique<SkipList>();
    }
    
    std::unique_ptr<SkipList> skip_list_;
};

TEST_F(SkipListTest, InsertAndGet) {
    EXPECT_TRUE(skip_list_->Insert("key1", "value1"));
    EXPECT_TRUE(skip_list_->Insert("key2", "value2"));
    
    std::string value;
    EXPECT_TRUE(skip_list_->Get("key1", &value));
    EXPECT_EQ(value, "value1");
    
    EXPECT_TRUE(skip_list_->Get("key2", &value));
    EXPECT_EQ(value, "value2");
}

TEST_F(SkipListTest, UpdateValue) {
    EXPECT_TRUE(skip_list_->Insert("key1", "value1"));
    EXPECT_TRUE(skip_list_->Insert("key1", "value2"));
    
    std::string value;
    EXPECT_TRUE(skip_list_->Get("key1", &value));
    EXPECT_EQ(value, "value2");
}

TEST_F(SkipListTest, Delete) {
    EXPECT_TRUE(skip_list_->Insert("key1", "value1"));
    EXPECT_TRUE(skip_list_->Delete("key1"));
    
    std::string value;
    EXPECT_FALSE(skip_list_->Get("key1", &value));
}

TEST_F(SkipListTest, GetAllEntries) {
    EXPECT_TRUE(skip_list_->Insert("key3", "value3"));
    EXPECT_TRUE(skip_list_->Insert("key1", "value1"));
    EXPECT_TRUE(skip_list_->Insert("key2", "value2"));
    
    auto entries = skip_list_->GetAllEntries();
    EXPECT_EQ(entries.size(), 3);
    
    // Check if entries are sorted
    EXPECT_TRUE(std::is_sorted(entries.begin(), entries.end(),
        [](const auto& a, const auto& b) {
            return a.first < b.first;
        }));
    
    // Check values
    EXPECT_EQ(entries[0].first, "key1");
    EXPECT_EQ(entries[0].second, "value1");
    EXPECT_EQ(entries[1].first, "key2");
    EXPECT_EQ(entries[1].second, "value2");
    EXPECT_EQ(entries[2].first, "key3");
    EXPECT_EQ(entries[2].second, "value3");
}

TEST_F(SkipListTest, NonExistentKey) {
    std::string value;
    EXPECT_FALSE(skip_list_->Get("nonexistent", &value));
}

TEST_F(SkipListTest, DeleteNonExistentKey) {
    EXPECT_FALSE(skip_list_->Delete("nonexistent"));
}

TEST_F(SkipListTest, LargeNumberOfEntries) {
    const int num_entries = 1000;
    for (int i = 0; i < num_entries; ++i) {
        std::string key = "key" + std::to_string(i);
        std::string value = "value" + std::to_string(i);
        EXPECT_TRUE(skip_list_->Insert(key, value));
    }
    
    auto entries = skip_list_->GetAllEntries();
    EXPECT_EQ(entries.size(), num_entries);
    
    // Verify all entries are present and sorted
    EXPECT_TRUE(std::is_sorted(entries.begin(), entries.end(),
        [](const auto& a, const auto& b) {
            return a.first < b.first;
        }));
    
    for (int i = 0; i < num_entries; ++i) {
        std::string key = "key" + std::to_string(i);
        std::string value = "value" + std::to_string(i);
        EXPECT_EQ(entries[i].first, key);
        EXPECT_EQ(entries[i].second, value);
    }
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
} 