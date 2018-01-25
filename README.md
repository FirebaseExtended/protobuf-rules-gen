# Firebase Rules Protobuf Validation

[//]: # (Build status here...)

### Quick Note

This is an *experimental* plugin for Security Rules, which means you should
__always__ validate these by hand before you decide to deploy these to a
production environment.

## Introduction

This is an experimental `protoc` plugin that generates [Firebase Rules for Cloud
Firestore](https://firebase.google.com/docs/firestore/security/overview) based
on Google's Protocol Buffer format.

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
function is_PersonMessage(resource) {
  return ((resource.keys().hasAll(['name']) && resource.size() == 1)) ||
          (resource.keys().hasAll(['name','email']) && resource.size() == 2)) ||
          (resource.keys().hasAll(['name','phone']) && resource.size() == 2)) ||
          (resource.keys().hasAll(['name','phone','email']) && resource.size() == 3))) &&
          ((resource.name is string)) &&
          ((resource.email == null) || (resource.email is string)) &&
          ((resource.phone == null) || (is_Person_PhoneNumberMessage(resource.phone)));
}
function is_Person_PhoneNumberMessage(resource) {
  return ((resource.keys().hasAll([]) && resource.size() == 0)) ||
          (resource.keys().hasAll(['number']) && resource.size() == 1)) ||
          (resource.keys().hasAll(['type']) && resource.size() == 1)) ||
          (resource.keys().hasAll(['type','number']) && resource.size() == 2))) &&
          ((resource.number == null) || (resource.number is string)) &&
          ((resource.type == null) || (is_Person_PhoneTypeEnum(resource.type)));
}
function is_Person_PhoneTypeEnum(resource) {
  return resource == "MOBILE" ||
          resource == "HOME" ||
          resource == "WORK";
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

```
TODO(rockwood): Fill this out once there is a release and prebuilt binaries
```

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
    option (google.firebase.rules.firebase_rules_enum).numberic_values = true;
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

  // TODO(rockwood): Support timestamps

  // This message must have either a phone or an email.
  option (google.firebase.rules.firebase_rules_message).validate =
      "resource.keys().hasAny(['email', 'phone'])";
}
```

This would generate the following functions.

```javascript
// @@START_GENERATED_FUNCTIONS@@
function istutorial_PersonMessage(resource) {
  return ((resource.keys().hasAll([]) && resource.size() == 0)) ||
          (resource.keys().hasAll(['name']) && resource.size() == 1)) ||
          (resource.keys().hasAll(['email']) && resource.size() == 1)) ||
          (resource.keys().hasAll(['phone']) && resource.size() == 1)) ||
          (resource.keys().hasAll(['starredWebsites']) && resource.size() == 1)) ||
          (resource.keys().hasAll(['email','name']) && resource.size() == 2)) ||
          (resource.keys().hasAll(['phone','name']) && resource.size() == 2)) ||
          (resource.keys().hasAll(['starredWebsites','name']) && resource.size() == 2)) ||
          (resource.keys().hasAll(['phone','email']) && resource.size() == 2)) ||
          (resource.keys().hasAll(['starredWebsites','email']) && resource.size() == 2)) ||
          (resource.keys().hasAll(['starredWebsites','phone']) && resource.size() == 2)) ||
          (resource.keys().hasAll(['phone','email','name']) && resource.size() == 3)) ||
          (resource.keys().hasAll(['starredWebsites','email','name']) && resource.size() == 3)) ||
          (resource.keys().hasAll(['starredWebsites','phone','name']) && resource.size() == 3)) ||
          (resource.keys().hasAll(['starredWebsites','phone','email']) && resource.size() == 3)) ||
          (resource.keys().hasAll(['starredWebsites','phone','email','name']) && resource.size() == 4))) &&
          ((resource.name == null) || (resource.name is string)) &&
          ((resource.email == null) || (resource.email is string && (resource.email.matches('.*@domain\.com')))) &&
          ((resource.phone == null) || (istutorial_Person_PhoneNumberMessage(resource.phone))) &&
          ((resource.starredWebsites == null) || (resource.starredWebsites is list)) &&
          (resource.keys().hasAny(['email', 'phone']));
}
function istutorial_Person_PhoneNumberMessage(resource) {
  return ((resource.keys().hasAll([]))) &&
          ((resource.number == null) || (resource.number is string)) &&
          ((resource.type == null) || (istutorial_Person_PhoneTypeEnum(resource.type)));
}
function istutorial_Person_PhoneTypeEnum(resource) {
  return resource == 0 ||
          resource == 1 ||
          resource == 2;
}
// @@END_GENERATED_FUNCTIONS@@

// Start your rules...
```

## Building

1) Install [Bazel](http://www.bazel.io/docs/install.html).

2) Build with `bazel build //...`

3) A sample invocation of the plugin, `protoc-gen-firebase_rules`, is available
in `example_usage.sh`. This script can be run from the command line.

## Releasing

1) Build the `proto-gen-firebase_rules` binary via `bazel build //...`

2) Ensure all the tests pass via `bazel test //...`

3) Build a binary for each platform (windows, linux, and darwin).

4) Tag a GitHub release and attach each prebuilt binary to the release.

## Authors

`proto-gen-firebase-rules` was initiated with ❤️️ by [Tyler
Rockwood](https://github.com/rockwotj).

## Disclaimer

This is not an official Google product (experimental or otherwise), it is just
code that happens to be owned by Google.
