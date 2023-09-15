
cc_library(
    name = "lk-taxscan",
    srcs = ["reader.cpp"],
    hdrs = ["taxscan.h"],
)

cc_binary(
    name = "lorikeet",
    srcs = ["main.cpp", "reader.cpp"],
    deps = [
        ":lk-taxscan",
    ],
)

cc_test(
    name = "test",
    srcs = ["test.cpp", "reader_test.cpp"],
    deps = ["lk-taxscan", "@googletest//:gtest_main"],
)