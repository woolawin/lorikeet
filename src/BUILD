
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
    srcs = ["test.cpp", "my_test2.cpp"],
    deps = [
        "@googletest//:gtest_main",
        # Add any other dependencies your test needs here
    ],
)