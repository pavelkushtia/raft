cc_library(
    name = "bloom_filter",
    srcs = ["src/bloom_filter.cpp"],
    hdrs = ["include/bloom_filter.h"],
    includes = ["include"],
    visibility = ["//visibility:public"],
)

cc_library(
    name = "sstable",
    srcs = ["src/sstable.cpp"],
    hdrs = ["include/sstable.h"],
    includes = ["include"],
    visibility = ["//visibility:public"],
    deps = [":bloom_filter"],
)

