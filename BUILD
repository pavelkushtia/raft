package(default_visibility = ["//visibility:public"])

cc_library(
    name = "sstable_lib",
    srcs = [
        "sstable/src/memtable.cpp",
        "sstable/src/sstable.cpp",
        "sstable/src/compaction.cpp",
        "sstable/src/lsm_tree.cpp",
        "sstable/src/bloom_filter.cpp",
        "sstable/src/skip_list.cpp",
    ],
    hdrs = [
        "sstable/include/memtable.h",
        "sstable/include/sstable.h",
        "sstable/include/compaction.h",
        "sstable/include/lsm_tree.h",
        "sstable/include/bloom_filter.h",
        "sstable/include/skip_list.h",
    ],
    includes = ["sstable/include"],
    copts = ["-std=c++17"],
)

cc_test(
    name = "memtable_test",
    srcs = ["sstable/tests/memtable_test.cpp"],
    deps = [
        ":sstable_lib",
        "@com_google_googletest//:gtest_main",
    ],
    copts = ["-std=c++17"],
)

cc_test(
    name = "sstable_test",
    srcs = ["sstable/tests/sstable_test.cpp"],
    deps = [
        ":sstable_lib",
        "@com_google_googletest//:gtest_main",
    ],
    copts = ["-std=c++17"],
)

cc_test(
    name = "compaction_test",
    srcs = ["sstable/tests/compaction_test.cpp"],
    deps = [
        ":sstable_lib",
        "@com_google_googletest//:gtest_main",
    ],
    copts = ["-std=c++17"],
)

cc_test(
    name = "lsm_tree_test",
    srcs = ["sstable/tests/lsm_tree_test.cpp"],
    deps = [
        ":sstable_lib",
        "@com_google_googletest//:gtest_main",
    ],
    copts = ["-std=c++17"],
)

cc_test(
    name = "skip_list_test",
    srcs = ["sstable/tests/skip_list_test.cpp"],
    deps = [
        ":sstable_lib",
        "@com_google_googletest//:gtest_main",
    ],
    copts = ["-std=c++17"],
)

cc_binary(
    name = "sstable_example",
    srcs = ["sstable/examples/main.cpp"],
    deps = [":sstable_lib"],
    copts = ["-std=c++17"],
) 