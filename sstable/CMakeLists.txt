cmake_minimum_required(VERSION 3.10)
project(sstable VERSION 1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find GTest package
find_package(GTest REQUIRED)

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/include)

# Add source files
set(SOURCES
    src/memtable.cpp
    src/sstable.cpp
    src/compaction.cpp
    src/lsm_tree.cpp
)

# Add header files
set(HEADERS
    include/memtable.h
    include/sstable.h
    include/compaction.h
    include/lsm_tree.h
)

# Create library
add_library(sstable STATIC ${SOURCES} ${HEADERS})

# Add tests
add_executable(memtable_test tests/memtable_test.cpp)
add_executable(sstable_test tests/sstable_test.cpp)
add_executable(compaction_test tests/compaction_test.cpp)
add_executable(lsm_tree_test tests/lsm_tree_test.cpp)

# Link tests with GTest and our library
target_link_libraries(memtable_test GTest::GTest GTest::Main sstable)
target_link_libraries(sstable_test GTest::GTest GTest::Main sstable)
target_link_libraries(compaction_test GTest::GTest GTest::Main sstable)
target_link_libraries(lsm_tree_test GTest::GTest GTest::Main sstable)

# Add example
add_executable(sstable_example examples/main.cpp)
target_link_libraries(sstable_example sstable)

# Enable testing
enable_testing()
add_test(NAME memtable_test COMMAND memtable_test)
add_test(NAME sstable_test COMMAND sstable_test)
add_test(NAME compaction_test COMMAND compaction_test)
add_test(NAME lsm_tree_test COMMAND lsm_tree_test) 