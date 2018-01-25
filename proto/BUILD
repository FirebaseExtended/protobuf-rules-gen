proto_library(
    name = "rules_example1_proto",
    srcs = ["example.proto"],
    deps = [":firebase_rules_options_proto"],
)

proto_library(
    name = "firebase_rules_options_proto",
    srcs = [
        "firebase_rules_options.proto",
    ],
    visibility = ["//visibility:public"],
    deps = ["@com_google_protobuf//:descriptor_proto"],
)

filegroup(
    name = "firebase_rules_options_proto_file",
    srcs = [
        "firebase_rules_options.proto",
    ],
    visibility = ["//visibility:public"],
)
