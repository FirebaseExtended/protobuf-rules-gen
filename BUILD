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
        "$(location //example:example.rules)",
        "$(location //example/testdata:golden.rules)",
        "$(locations :testdata)",
    ],
    data = [
        "//example/testdata:golden.rules",
        "//example:example.rules",
        ":testdata",
        "//firebase_rules_generator:protoc-gen-firebase_rules",
        "//proto:firebase_rules_options_proto_file",
        "@com_google_protobuf//:descriptor_proto",
        "@com_google_protobuf//:protoc",
        "@com_google_protobuf//:timestamp_proto",
    ],
)

platform(
    name = "windows-amd64",
    constraint_values = [
        "@platforms//os:windows",
        "@platforms//cpu:x86_64",
    ],
)
platform(
    name = "windows-arm64",
    constraint_values = [
        "@platforms//os:windows",
        "@platforms//cpu:arm64",
    ],
)
platform(
    name = "darwin-amd64",
    constraint_values = [
        "@platforms//os:macos",
        "@platforms//cpu:x86_64",
    ],
)
platform(
    name = "darwin-arm64",
    constraint_values = [
        "@platforms//os:macos",
        "@platforms//cpu:arm64",
    ],
)
platform(
    name = "linux-amd64",
    constraint_values = [
        "@platforms//os:linux",
        "@platforms//cpu:x86_64",
    ],
)
platform(
    name = "linux-arm64",
    constraint_values = [
        "@platforms//os:linux",
        "@platforms//cpu:arm64",
    ],
)
