#include "lsm_tree.h"
#include <gtest/gtest.h>
#include <string>
#include <filesystem>
#include <vector>
#include <thread>
#include <chrono>

using namespace sstable;

class LSMTreeTest : public ::testing::Test {
protected:
    void SetUp() override {
        test_dir_ = "/tmp/lsm_tree_test";
        std::filesystem::create_directories(test_dir_);
        lsm_tree_ = std::make_unique<LSMTree>(test_dir_, 64 * 1024 * 1024); // 64MB MemTable
    }
    
    void TearDown() override {
        std::filesystem::remove_all(test_dir_);
    }
    
    std::string test_dir_;
    std::unique_ptr<LSMTree> lsm_tree_;
};

TEST_F(LSMTreeTest, BasicOperations) {
    // Test Put and Get
    EXPECT_TRUE(lsm_tree_->Put("key1", "value1"));
    std::string value;
    EXPECT_TRUE(lsm_tree_->Get("key1", &value));
    EXPECT_EQ(value, "value1");
    
    // Test Update
    EXPECT_TRUE(lsm_tree_->Put("key1", "value1_updated"));
    EXPECT_TRUE(lsm_tree_->Get("key1", &value));
    EXPECT_EQ(value, "value1_updated");
    
    // Test Delete
    EXPECT_TRUE(lsm_tree_->Delete("key1"));
    EXPECT_FALSE(lsm_tree_->Get("key1", &value));
}

TEST_F(LSMTreeTest, RangeQuery) {
    // Insert multiple keys
    EXPECT_TRUE(lsm_tree_->Put("key1", "value1"));
    EXPECT_TRUE(lsm_tree_->Put("key2", "value2"));
    EXPECT_TRUE(lsm_tree_->Put("key3", "value3"));
    EXPECT_TRUE(lsm_tree_->Put("key4", "value4"));
    
    // Get range
    auto entries = lsm_tree_->GetRange("key2", "key3");
    EXPECT_EQ(entries.size(), 2);
    EXPECT_EQ(entries[0].first, "key2");
    EXPECT_EQ(entries[0].second, "value2");
    EXPECT_EQ(entries[1].first, "key3");
    EXPECT_EQ(entries[1].second, "value3");
}

TEST_F(LSMTreeTest, MemTableFlush) {
    // Fill MemTable to trigger flush
    for (int i = 0; i < 1000; ++i) {
        EXPECT_TRUE(lsm_tree_->Put("key" + std::to_string(i), 
            std::string(1024, 'x'))); // 1KB values
    }
    
    // Verify data is still accessible after flush
    std::string value;
    EXPECT_TRUE(lsm_tree_->Get("key0", &value));
    EXPECT_EQ(value, std::string(1024, 'x'));
}

TEST_F(LSMTreeTest, Compaction) {
    // Fill multiple levels to trigger compaction
    for (int i = 0; i < 2000; ++i) {
        EXPECT_TRUE(lsm_tree_->Put("key" + std::to_string(i), 
            std::string(1024, 'x'))); // 1KB values
    }
    
    // Verify data is still accessible after compaction
    std::string value;
    EXPECT_TRUE(lsm_tree_->Get("key0", &value));
    EXPECT_EQ(value, std::string(1024, 'x'));
}

TEST_F(LSMTreeTest, ConcurrentAccess) {
    const int num_threads = 4;
    const int num_ops = 1000;
    std::vector<std::thread> threads;
    
    // Function for each thread
    auto worker = [this](int thread_id) {
        for (int i = 0; i < num_ops; ++i) {
            std::string key = "key" + std::to_string(thread_id) + "_" + std::to_string(i);
            std::string value = "value" + std::to_string(thread_id) + "_" + std::to_string(i);
            EXPECT_TRUE(lsm_tree_->Put(key, value));
            
            std::string retrieved_value;
            EXPECT_TRUE(lsm_tree_->Get(key, &retrieved_value));
            EXPECT_EQ(retrieved_value, value);
        }
    };
    
    // Start threads
    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back(worker, i);
    }
    
    // Wait for threads to complete
    for (auto& thread : threads) {
        thread.join();
    }
    
    // Verify all entries are present
    for (int i = 0; i < num_threads; ++i) {
        for (int j = 0; j < num_ops; ++j) {
            std::string key = "key" + std::to_string(i) + "_" + std::to_string(j);
            std::string expected_value = "value" + std::to_string(i) + "_" + std::to_string(j);
            std::string value;
            EXPECT_TRUE(lsm_tree_->Get(key, &value));
            EXPECT_EQ(value, expected_value);
        }
    }
}

TEST_F(LSMTreeTest, Recovery) {
    // Insert some data
    EXPECT_TRUE(lsm_tree_->Put("key1", "value1"));
    EXPECT_TRUE(lsm_tree_->Put("key2", "value2"));
    
    // Create a new LSM Tree instance to simulate recovery
    auto recovered_tree = std::make_unique<LSMTree>(test_dir_, 64 * 1024 * 1024);
    
    // Verify data is still accessible
    std::string value;
    EXPECT_TRUE(recovered_tree->Get("key1", &value));
    EXPECT_EQ(value, "value1");
    EXPECT_TRUE(recovered_tree->Get("key2", &value));
    EXPECT_EQ(value, "value2");
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
} 