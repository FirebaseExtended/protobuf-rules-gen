load("@com_google_protobuf//:protobuf.bzl", "proto_gen")

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

proto_gen(
    name = "firebase_rules_options_genproto_rules",
    srcs = [
        "firebase_rules_options.proto",
    ],
    protoc = "@com_google_protobuf//:protoc",
    visibility = ["//visibility:public"],
    deps = [":well_known_protos"],
)

proto_gen(
    name = "well_known_protos",
    srcs = ["@com_google_protobuf//:well_known_protos"],
    protoc = "@com_google_protobuf//:protoc",
    visibility = ["//visibility:public"],
)
