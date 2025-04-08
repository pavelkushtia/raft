# SSTable Implementation in C++

This project implements a Log-Structured Merge Tree (LSM Tree) based storage system with SSTables (Sorted String Tables). It provides efficient write and read operations for key-value pairs with features like compaction, tombstone handling, and persistent storage.

## Overview

SSTables are immutable, sorted files that store key-value pairs. This implementation includes:

- MemTable: In-memory buffer for recent writes
- SSTable: Persistent storage of sorted key-value pairs
- Compaction: Merging SSTables to maintain efficiency
- Tombstone handling: Support for key deletion
- LSM Tree structure: Multiple levels of SSTables

## Project Structure

```
sstable/
├── include/           # Header files
│   ├── memtable.h     # MemTable implementation
│   ├── sstable.h      # SSTable implementation
│   ├── compaction.h   # Compaction strategy
│   └── lsm_tree.h     # LSM Tree implementation
├── src/              # Source files
│   ├── memtable.cpp
│   ├── sstable.cpp
│   ├── compaction.cpp
│   └── lsm_tree.cpp
├── tests/            # Unit tests
│   ├── memtable_test.cpp
│   ├── sstable_test.cpp
│   ├── compaction_test.cpp
│   └── lsm_tree_test.cpp
├── examples/         # Example usage
│   └── main.cpp
└── CMakeLists.txt    # Build configuration
```

## Key Components

### 1. MemTable
- In-memory buffer for recent writes
- Uses a skip list for efficient insertion and lookup
- Flushes to SSTable when size threshold is reached

### 2. SSTable
- Immutable sorted file of key-value pairs
- Binary format for efficient storage
- Supports point lookups and range scans
- Includes bloom filter for quick existence checks

### 3. Compaction
- Merges multiple SSTables into larger ones
- Removes duplicate keys and tombstones
- Maintains sorted order
- Implements size-tiered compaction strategy

### 4. LSM Tree
- Manages multiple levels of SSTables
- Handles write and read operations
- Coordinates compaction across levels
- Maintains metadata for efficient lookups

## Building and Running

### Prerequisites
- C++17 or later
- CMake 3.10 or later
- Google Test (for unit tests)

### Build Instructions
```bash
mkdir build
cd build
cmake ..
make
```

### Running Tests
```bash
cd build
ctest
```

### Example Usage
```cpp
#include "lsm_tree.h"

int main() {
    LSMTree db("/tmp/sstable");
    
    // Write operations
    db.Put("key1", "value1");
    db.Put("key2", "value2");
    
    // Read operations
    std::string value;
    if (db.Get("key1", &value)) {
        std::cout << "Value: " << value << std::endl;
    }
    
    // Delete operation
    db.Delete("key2");
    
    return 0;
}
```

## Implementation Details

### Data Format
SSTables store data in the following format:
```
[Header]
- Magic number (4 bytes)
- Version (4 bytes)
- Number of entries (8 bytes)
- Bloom filter data

[Data Section]
- Key length (4 bytes)
- Value length (4 bytes)
- Key data
- Value data
```

### Performance Considerations
- Write amplification is minimized through careful compaction strategy
- Read amplification is reduced using bloom filters and metadata
- Space amplification is controlled through compaction thresholds

## Testing
The project includes comprehensive unit tests for all components:
- MemTable operations
- SSTable serialization/deserialization
- Compaction strategies
- LSM Tree operations
- Tombstone handling

## Future Improvements
- Implement parallel compaction
- Add compression support
- Support for custom comparators
- Transaction support
- Backup and restore functionality

## License
MIT License
