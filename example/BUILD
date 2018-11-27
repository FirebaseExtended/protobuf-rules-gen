load("@proto_gen_firebase_rules//:bazel/defs.bzl", "firestore_rules_proto_library", "firestore_rules_binary")

firestore_rules_proto_library(
    name = "schema",
    srcs = ["schema.proto"],
)

firestore_rules_binary(
    name = "example",
    srcs = ["main.rules"],
    deps = [":schema"],
    visibility = ["//visibility:public"],
)

exports_files(["testdata/golden.rules"])