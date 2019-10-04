load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

protobuf_version = "3.6.1.3"

protobuf_sha256 = "73fdad358857e120fd0fa19e071a96e15c0f23bb25f85d3f7009abfd4f264a2a"


def protobuf_rules_gen_repositories():
    if "com_google_protobuf" not in native.existing_rules():
        http_archive(
            name = "com_google_protobuf",
            sha256 = protobuf_sha256,
            strip_prefix = "protobuf-" + protobuf_version,
            url = "https://github.com/google/protobuf/archive/v" + protobuf_version + ".tar.gz",
        )
