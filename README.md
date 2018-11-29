# Firebase Rules Protobuf Validation

[![Build
Status](https://travis-ci.org/firebase/protobuf-rules-gen.svg?branch=master)](https://travis-ci.org/firebase/protobuf-rules-gen)

### Quick Note

This is an *experimental* plugin for Security Rules, which means you should
__always__ validate these by hand before you decide to deploy these to a
production environment.

All support is handled through this repo, so if you've got a question or
problem, [file an issue](https://github.com/firebase/protobuf-rules-gen/issues)!

## Introduction

This is an experimental [`protoc`
plugin](https://developers.google.com/protocol-buffers/docs/reference/other)
that generates [Firebase Rules for Cloud
Firestore](https://firebase.google.com/docs/firestore/security/overview) based
on [Google's Protocol Buffer
format](https://developers.google.com/protocol-buffers/).

This allows you to easily validate your data in a platform independent manner.

Here is a quick example:

[//]: # (Keep this up to date with test5 in rules)

```protobuf
syntax = "proto2";
package tutorial;

message Person {
  required string name = 1;
  optional string email = 2;

  enum PhoneType {
    MOBILE = 0;
    HOME = 1;
    WORK = 2;
  }
  message PhoneNumber {
    optional string number = 1;
    optional PhoneType type = 2;
  }

  optional PhoneNumber phone = 3;
}
```

This plugin generates the following [Firebase Rules
function](https://firebase.google.com/docs/firestore/reference/security/#developer_defined)
that can be used to validate your incoming data.

```javascript
// @@START_GENERATED_FUNCTIONS@@
function isPersonMessage(resource) {
  return resource.keys().hasAll(['name']) &&
          (resource.keys().hasOnly(['name','phone','email'])) &&
          ((resource.name is string)) &&
          ((!resource.keys().hasAny(['email'])) || (resource.email is string)) &&
          ((!resource.keys().hasAny(['phone'])) || (isPerson_PhoneNumberMessage(resource.phone)));
}
function isPerson_PhoneNumberMessage(resource) {
  return resource.keys().hasAll([]) &&
          (resource.keys().hasOnly(['type','number'])) &&
          ((!resource.keys().hasAny(['number'])) || (resource.number is string)) &&
          ((!resource.keys().hasAny(['type'])) || (isPerson_PhoneTypeEnum(resource.type)));
}
function isPerson_PhoneTypeEnum(resource) {
  return resource == 0 ||
          resource == 1 ||
          resource == 2;
}
// @@END_GENERATED_FUNCTIONS@@

// Start your rules (these don't get generated!)
service cloud.firestore {
  match /databases/{database}/documents {
    match /users/{userId} {
      allow read: if request.auth.uid == userId;
      allow write: if isPersonMessage(request.resource.data) &&
                      request.auth.uid == userId;
    }
  }
}
```

## Usage

To use this `protoc` plugin once you have your protocol buffers defined, follow
these steps:

1.  Make sure to install the latest version of
    [`protoc`](https://github.com/google/protobuf#protocol-compiler-installation)
2.  Download the latest release from
    [GitHub](https://github.com/firebase/protobuf-rules-gen/releases)
3.  Either put the plugin binary on your `$PATH` or use the
    `--plugin=protoc-gen-firebase_rules=./path/to/protoc-gen-firebase_rules`
    option
4.  Invoke the `protoc` tool using the `--firebase_rules_out=./directory` flag
    to output your `firestore.rules` file with generated functions
5.  If you're importing `"firebase_rules_options.proto"` like the below example
    the `--proto_path=./directory` flag will need to be added to include the
    directory of the `firebase_rules_options.proto` file along with the protobuf
    files from [the `src` directory of the Google Protobuf
    repo](https://github.com/google/protobuf/tree/master/src). An more indepth
    discussion of this can be found [in this
    issue](https://github.com/firebase/protobuf-rules-gen/issues/16).

If you run into trouble feel free to check out our
[`example_usage.sh`](https://github.com/firebase/protobuf-rules-gen/blob/master/example_usage.sh)
script or [file an issue](https://github.com/firebase/protobuf-rules-gen/issues)

## Advanced Usage

[//]: # (https://developers.google.com/protocol-buffers/docs/proto#customoptions)
[//]: # (https://firebase.google.com/docs/firestore/reference/security/)
[//]: # (Keep this up to date with test6 in rules)

```protobuf
syntax = "proto3";
package tutorial;

import "firebase_rules_options.proto";

option (google.firebase.rules.firebase_rules).full_package_names = true;

message Person {
  string name = 1;
  string email = 2 [(google.firebase.rules.firebase_rules_field).validate =
                        "resource.email.matches('.*@domain\\.com')"];

  enum PhoneType {
    option (google.firebase.rules.firebase_rules_enum).string_values = true;
    MOBILE = 0;
    HOME = 1;
    WORK = 2;
  }

  message PhoneNumber {
    string number = 1;
    PhoneType type = 2;
    option (google.firebase.rules.firebase_rules_message).extra_properties =
        true;
  }

  PhoneNumber phone = 3;

  // Currently, we can only check this is a list :(
  repeated string starred_websites = 4;

  // This message must have either a phone or an email.
  option (google.firebase.rules.firebase_rules_message).validate =
      "resource.keys().hasAny(['email', 'phone'])";
}
```

This would generate the following functions.

```javascript
// @@START_GENERATED_FUNCTIONS@@
function istutorial_PersonMessage(resource) {
  return resource.keys().hasAll([]) &&
          (resource.keys().hasOnly(['starredWebsites','phone','email','name'])) &&
          ((!resource.keys().hasAny(['name'])) || (resource.name is string)) &&
          ((!resource.keys().hasAny(['email'])) || (resource.email is string && (resource.email.matches('.*@domain\.com')))) &&
          ((!resource.keys().hasAny(['phone'])) || (istutorial_Person_PhoneNumberMessage(resource.phone))) &&
          ((!resource.keys().hasAny(['starredWebsites'])) || (resource.starredWebsites is list)) &&
          (resource.keys().hasAny(['email', 'phone']));
}
function istutorial_Person_PhoneNumberMessage(resource) {
  return resource.keys().hasAll([]) &&
          ((!resource.keys().hasAny(['number'])) || (resource.number is string)) &&
          ((!resource.keys().hasAny(['type'])) || (istutorial_Person_PhoneTypeEnum(resource.type)));
}
function istutorial_Person_PhoneTypeEnum(resource) {
  return resource == 'MOBILE' ||
          resource == 'HOME' ||
          resource == 'WORK';
}
// @@END_GENERATED_FUNCTIONS@@

// Start your rules...
```

## Standalone usage

1) Install [Bazel](http://www.bazel.io/docs/install.html).

2) Build with `bazel build //...`

3) A sample invocation of the plugin, `protoc-gen-firebase_rules`, is available
in `example_usage.sh`. This script can be run from the command line.

## Using with bazel

It's easy to use protobuf_rules_gen if your project already uses Bazel.

1) Add protobuf_rules_gen to your WORKSPACE:

```python
proto_gen_firebase_rules_commit = "TODO"
http_archive(
    name = "proto_gen_firebase_rules",
    sha256 = "TODO",
    strip_prefix = "protobuf-rules-gen-" + proto_gen_firebase_rules_commit,
    url = "http://github.com/FirebaseExtended/protobuf-rules-gen/archive/" + proto_gen_firebase_rules_commit + ".tar.gz",
)

load("@proto_gen_firebase_rules//bazel:repositories.bzl", "protobuf_rules_gen_repositories")
protobuf_rules_gen_repositories()
```

2) Update your BUILD file:
```python
load("@proto_gen_firebase_rules//bazel:defs.bzl", "firestore_rules_proto_library", "firestore_rules_binary")
```

There are three rules available:
    * firestore_rules_proto_library generates a .rules file from the protobuf
      schema
    * firestore_rules_binary combines multiple .rules files (e.g. the auto
      generated rules with your ACLs that use them)
    * firestore_rules_library wraps up one or more .rules files so that a 
      firestore_rules_binary can depend on it.

See example/BUILD for an example of how to use these rules.


## Releasing

1) Build the `proto-gen-firebase_rules` binary via `bazel build //...`

2) Ensure all the tests pass via `bazel test //...`

3) Build a binary for each platform (windows, linux, and darwin).

4) Tag a GitHub release and attach each prebuilt binary to the release.

## Authors

`protobuf-rules-gen` was initiated with ❤️️ by [Tyler
Rockwood](https://github.com/rockwotj).

## Disclaimer

This is not an official Google product (experimental or otherwise), it is just
code that happens to be owned by Google.
