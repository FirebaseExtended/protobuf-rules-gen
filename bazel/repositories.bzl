load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

def protobuf_rules_gen_repositories():
    if "com_google_protobuf" not in native.existing_rules():
        http_archive(
            name = "rules_python",
            sha256 = "934c9ceb552e84577b0faf1e5a2f0450314985b4d8712b2b70717dc679fdc01b",
            url = "https://github.com/bazelbuild/rules_python/releases/download/0.3.0/rules_python-0.3.0.tar.gz",
        )
        http_archive(
            name = "bazel_skylib",
            sha256 = "1c531376ac7e5a180e0237938a2536de0c54d93f5c278634818e0efc952dd56c",
            urls = [
                "https://github.com/bazelbuild/bazel-skylib/releases/download/1.0.3/bazel-skylib-1.0.3.tar.gz",
                "https://mirror.bazel.build/github.com/bazelbuild/bazel-skylib/releases/download/1.0.3/bazel-skylib-1.0.3.tar.gz",
            ],
        )
        http_archive(
            name = "zlib",
            build_file = "@com_google_protobuf//:third_party/zlib.BUILD",
            sha256 = "c3e5e9fdd5004dcb542feda5ee4f0ff0744628baf8ed2dd5d66f8ca1197cb1a1",
            strip_prefix = "zlib-1.2.11",
            urls = [
                "https://mirror.bazel.build/zlib.net/zlib-1.2.11.tar.gz",
                "https://zlib.net/zlib-1.2.11.tar.gz",
            ],
        )
        http_archive(
            name = "com_google_protobuf",
            sha256 = "528927e398f4e290001886894dac17c5c6a2e5548f3fb68004cfb01af901b53a",
            strip_prefix = "protobuf-3.17.3",
            url = "https://github.com/google/protobuf/archive/v3.17.3.zip",
        )
