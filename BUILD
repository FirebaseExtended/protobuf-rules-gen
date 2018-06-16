filegroup(
    name = "testdata",
    srcs = glob([
        "testdata/*.proto",
        "testdata/*.rules",
    ]),
)

py_test(
    name = "integration_test",
    size = "small",
    srcs = ["integration_test.py"],
    args = [
        "$(location //firebase_rules_generator:protoc-gen-firebase_rules)",
        "$(location @com_google_protobuf//:protoc)",
        "$(location //proto:firebase_rules_options_proto_file)",
        "$(location @com_google_protobuf//:descriptor_proto)",
        "$(locations :testdata)",
    ],
    data = [
        ":testdata",
        "//firebase_rules_generator:protoc-gen-firebase_rules",
        "//proto:firebase_rules_options_proto_file",
        "@com_google_protobuf//:descriptor_proto",
        "@com_google_protobuf//:protoc",
        "@com_google_protobuf//:timestamp_proto",
    ],
)
