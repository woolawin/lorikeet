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
