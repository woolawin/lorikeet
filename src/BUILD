
cc_library(
    name = "lk-reader",
    srcs = ["reader.cpp"],
    hdrs = ["reader.h"],
)

cc_binary(
    name = "lorikeet",
    srcs = ["main.cpp", "reader.cpp"],
    deps = [
        ":lk-reader",
    ],
)

cc_test(
    name = "test",
    srcs = ["my_test.cpp", "my_test2.cpp"],
    deps = [
        "@googletest//:gtest_main",
        # Add any other dependencies your test needs here
    ],
)