workspace(name = "raft")

# This is an empty WORKSPACE file to mark the root of the workspace.
# Dependencies are now managed through MODULE.bazel (Bzlmod).

load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

# GoogleTest
http_archive(
    name = "googletest",
    urls = ["https://github.com/google/googletest/archive/refs/tags/v1.14.0.tar.gz"],
    strip_prefix = "googletest-1.14.0",
)

# Google Benchmark
http_archive(
    name = "com_google_benchmark",
    urls = ["https://github.com/google/benchmark/archive/refs/tags/v1.8.3.tar.gz"],
    strip_prefix = "benchmark-1.8.3",
) 