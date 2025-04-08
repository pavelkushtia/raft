#include "memtable.h"
#include <gtest/gtest.h>
#include <string>
#include <thread>
#include <vector>

using namespace sstable;

class MemTableTest : public ::testing::Test {
protected:
    void SetUp() override {
        memtable_ = std::make_unique<MemTable>(1024); // 1KB size limit
    }
    
    std::unique_ptr<MemTable> memtable_;
};

TEST_F(MemTableTest, BasicOperations) {
    EXPECT_TRUE(memtable_->Put("key1", "value1"));
    EXPECT_TRUE(memtable_->Put("key2", "value2"));
    
    std::string value;
    EXPECT_TRUE(memtable_->Get("key1", &value));
    EXPECT_EQ(value, "value1");
    
    EXPECT_TRUE(memtable_->Get("key2", &value));
    EXPECT_EQ(value, "value2");
}

TEST_F(MemTableTest, UpdateValue) {
    EXPECT_TRUE(memtable_->Put("key1", "value1"));
    EXPECT_TRUE(memtable_->Put("key1", "value2"));
    
    std::string value;
    EXPECT_TRUE(memtable_->Get("key1", &value));
    EXPECT_EQ(value, "value2");
}

TEST_F(MemTableTest, Delete) {
    EXPECT_TRUE(memtable_->Put("key1", "value1"));
    EXPECT_TRUE(memtable_->Delete("key1"));
    
    std::string value;
    EXPECT_FALSE(memtable_->Get("key1", &value));
}

TEST_F(MemTableTest, SizeLimit) {
    // Fill up the MemTable
    std::string large_value(512, 'x'); // Half of the size limit
    EXPECT_TRUE(memtable_->Put("key1", large_value));
    EXPECT_TRUE(memtable_->Put("key2", large_value));
    
    // Next insert should fail
    EXPECT_FALSE(memtable_->Put("key3", "value3"));
}

TEST_F(MemTableTest, GetAllEntries) {
    EXPECT_TRUE(memtable_->Put("key3", "value3"));
    EXPECT_TRUE(memtable_->Put("key1", "value1"));
    EXPECT_TRUE(memtable_->Put("key2", "value2"));
    
    auto entries = memtable_->GetAllEntries();
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

TEST_F(MemTableTest, ConcurrentAccess) {
    const int num_threads = 4;
    const int num_operations = 1000;
    std::vector<std::thread> threads;
    
    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back([this, i, num_operations]() {
            for (int j = 0; j < num_operations; ++j) {
                std::string key = "key" + std::to_string(i) + "_" + std::to_string(j);
                std::string value = "value" + std::to_string(i) + "_" + std::to_string(j);
                memtable_->Put(key, value);
                
                std::string retrieved;
                if (memtable_->Get(key, &retrieved)) {
                    EXPECT_EQ(retrieved, value);
                }
            }
        });
    }
    
    for (auto& thread : threads) {
        thread.join();
    }
    
    // Verify all entries are present
    auto entries = memtable_->GetAllEntries();
    EXPECT_EQ(entries.size(), num_threads * num_operations);
}

TEST_F(MemTableTest, TombstoneHandling) {
    EXPECT_TRUE(memtable_->Put("key1", "value1"));
    EXPECT_TRUE(memtable_->Delete("key1"));
    
    std::string value;
    EXPECT_FALSE(memtable_->Get("key1", &value));
    
    auto entries = memtable_->GetAllEntries();
    EXPECT_EQ(entries.size(), 1);
    EXPECT_EQ(entries[0].first, "key1");
    EXPECT_TRUE(entries[0].second.empty()); // Tombstone value
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
} 