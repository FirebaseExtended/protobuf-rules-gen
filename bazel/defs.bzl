load("@com_google_protobuf//:protobuf.bzl", "proto_gen")

def _outputs(srcs, ext):
    return [s[:-len(".proto")] + ".pb" + ext for s in srcs]

def firestore_rules_library_impl(ctx):
    files = depset(order="postorder", direct=ctx.files.srcs, transitive=[d.fsrules.files for d in ctx.attr.deps])
    return struct(fsrules=struct(files=files))

def firestore_rules_binary_impl(ctx):
    files = depset(order="postorder", direct=ctx.files.srcs, transitive=[d.fsrules.files for d in ctx.attr.deps])
    args = ctx.actions.args()
    args.add_all(files)
    ctx.actions.run_shell(outputs=[ctx.outputs.bin], inputs=files, command='cat >"%s" "$@"' % ctx.outputs.bin.path, arguments=[args])

firestore_rules_library = rule(
    attrs = {
        "srcs": attr.label_list(
            allow_files = True,
            allow_empty = False,
        ),
        "deps": attr.label_list(providers = ["fsrules"]),
    },
    implementation = firestore_rules_library_impl,
)

firestore_rules_binary = rule(
    attrs = {
        "srcs": attr.label_list(
            allow_files = True,
            allow_empty = False,
        ),
        "deps": attr.label_list(providers = ["fsrules"]),
    },
    outputs = {
        "bin": "%{name}.rules",
    },
    implementation = firestore_rules_binary_impl,
)

def firestore_rules_proto_library(
        name,
        srcs = [],
        deps = [],
        include = None,
        protoc = "@com_google_protobuf//:protoc",
        **kargs):
    """Bazel rule to generate firestore security rules from protobuf schema

    Args:
      name: the name of the firestore_rules_proto_library.
      srcs: the .proto files of the firestore_rules_proto_library.
      deps: a list of dependency labels; must be firestore_rules_proto_library.
      include: a string indicating the include path of the .proto files.
      protoc: the label of the protocol compiler to generate the sources.
      **kargs: other keyword arguments that are passed to ts_library.

    """
    outs = _outputs(srcs, ".cc")

    includes = []
    if include != None:
        includes = [include]

    plugin = "//firebase_rules_generator:protoc-gen-firebase_rules"

    proto_gen(
        name = name + "_genproto_rules",
        srcs = srcs,
        deps = [s + "_genproto_rules" for s in deps]+ ["//proto:firebase_rules_options_genproto_rules"],
        includes = includes,
        protoc = protoc,
        outs = outs,
        visibility = ["//visibility:public"],
        plugin = plugin,
        plugin_language = "protoc-gen-firebase_rules",
        plugin_options = ["bazel"],
        gen_cc = True
    )

    firestore_rules_library(
        name = name,
        srcs = outs,
        deps = deps,
        **kargs
    )
    