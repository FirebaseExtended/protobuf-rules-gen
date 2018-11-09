# Copyright 2017 Google LLC
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

cc_library(
    name = "generator",
    srcs = ["generator.cc"],
    hdrs = ["generator.h"],
    visibility = ["//visibility:public"],
    deps = [
        ":firebase_rules_options",
        "@com_google_protobuf//:protobuf",
        "@com_google_protobuf//:protoc_lib",
    ],
)

cc_binary(
    name = "protoc-gen-firebase_rules",
    srcs = ["main.cc"],
    linkstatic = 1,
    visibility = ["//visibility:public"],
    deps = [
        ":generator",
        "@com_google_protobuf//:protobuf",
        "@com_google_protobuf//:protoc_lib",
    ],
)

cc_proto_library(
    name = "firebase_rules_options",
    visibility = ["//visibility:public"],
    deps = ["//proto:firebase_rules_options_proto"],
)
