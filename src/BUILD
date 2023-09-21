
cc_library(
    name = "lk-line",
    srcs = ["line.cpp",],
    hdrs = ["line.h"],
)

cc_library(
    name = "lk-errors",
    srcs = ["errors.cpp"],
    hdrs = ["errors.h"],
)

cc_library(
    name = "lk-taxscan",
    srcs = ["taxscan.cpp", "taxscan_types.cpp"],
    hdrs = ["taxscan.h"],
    deps = [":lk-line", ":lk-errors"],
)

cc_binary(
    name = "lorikeet",
    srcs = ["main.cpp"],
    deps = [
        ":lk-taxscan",
    ],
)

cc_test(
    name = "test",
    srcs = ["test.cpp", "line_test.cpp", "taxscan_test.cpp"],
    deps = ["lk-line", "lk-taxscan", "@googletest//:gtest_main"],
)