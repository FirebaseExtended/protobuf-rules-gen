protobuf_commit = "099d99759101c295244c24d8954ec85b8ac65ce3"

protobuf_sha256 = "c0ab1b088e220c1d56446f34001f0178e590270efdef1c46a77da4b9faa9d7b0"

WELL_KNOWN_PROTOS = [
    "google/protobuf/any.proto",
    "google/protobuf/api.proto",
    "google/protobuf/compiler/plugin.proto",
    "google/protobuf/descriptor.proto",
    "google/protobuf/duration.proto",
    "google/protobuf/empty.proto",
    "google/protobuf/field_mask.proto",
    "google/protobuf/source_context.proto",
    "google/protobuf/struct.proto",
    "google/protobuf/timestamp.proto",
    "google/protobuf/type.proto",
    "google/protobuf/wrappers.proto",
]

def protobuf_rules_gen_repositories():
    native.http_archive(
        name = "com_google_protobuf",
        sha256 = protobuf_sha256,
        strip_prefix = "protobuf-" + protobuf_commit,
        url = "https://github.com/google/protobuf/archive/" + protobuf_commit + ".tar.gz",
    )

    native.new_http_archive(
        name = "com_google_protobuf_well_known_protos",
        build_file_content = """
load("@com_google_protobuf//:protobuf.bzl","proto_gen")
proto_gen(
    name = "protos",
    srcs = %s,
    protoc = "@com_google_protobuf//:protoc",
    visibility = ["//visibility:public"],
)""" % (WELL_KNOWN_PROTOS,),
        sha256 = protobuf_sha256,
        # The well-known protos are below `src` but for protobuf imports to
        # work correctly they should be at the root, so we strip `src`.
        strip_prefix = "protobuf-%s/src" % (protobuf_commit,),
        urls = ["https://github.com/google/protobuf/archive/%s.tar.gz" % (protobuf_commit,)],
    )
