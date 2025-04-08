#include "lsm_tree.h"
#include <iostream>
#include <chrono>
#include <random>
#include <thread>

using namespace sstable;

void RunBenchmark(LSMTree& db, int num_operations) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> key_dist(0, 1000000);
    std::uniform_int_distribution<> op_dist(0, 2);
    
    auto start = std::chrono::high_resolution_clock::now();
    
    for (int i = 0; i < num_operations; ++i) {
        std::string key = "key" + std::to_string(key_dist(gen));
        std::string value = "value" + std::to_string(i);
        
        int op = op_dist(gen);
        switch (op) {
            case 0: // Put
                db.Put(key, value);
                break;
            case 1: // Get
                std::string result;
                db.Get(key, &result);
                break;
            case 2: // Delete
                db.Delete(key);
                break;
        }
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    std::cout << "Completed " << num_operations << " operations in "
              << duration.count() << " ms" << std::endl;
}

int main() {
    try {
        // Create LSM Tree with 64MB MemTable
        LSMTree db("/tmp/sstable", 64 * 1024 * 1024);
        
        // Basic operations
        std::cout << "Testing basic operations..." << std::endl;
        
        // Put some values
        db.Put("key1", "value1");
        db.Put("key2", "value2");
        db.Put("key3", "value3");
        
        // Get values
        std::string value;
        if (db.Get("key1", &value)) {
            std::cout << "Got key1: " << value << std::endl;
        }
        
        // Delete a key
        db.Delete("key2");
        if (!db.Get("key2", &value)) {
            std::cout << "key2 was successfully deleted" << std::endl;
        }
        
        // Range query
        auto range = db.GetRange("key1", "key3");
        std::cout << "Range query results:" << std::endl;
        for (const auto& [k, v] : range) {
            std::cout << k << ": " << v << std::endl;
        }
        
        // Run benchmark
        std::cout << "\nRunning benchmark..." << std::endl;
        RunBenchmark(db, 10000);
        
        // Force flush and compaction
        std::cout << "\nFlushing MemTable and compacting..." << std::endl;
        db.FlushMemTable();
        
        // Verify data persistence
        std::cout << "\nVerifying data persistence..." << std::endl;
        if (db.Get("key1", &value)) {
            std::cout << "key1 still exists after flush: " << value << std::endl;
        }
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
} 