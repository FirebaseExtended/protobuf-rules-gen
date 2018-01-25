#!/bin/bash -eux
#
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

if [ ! -f "WORKSPACE" ]; then
  echo "Not in a Bazel root directory (WORKSPACE file does not exist), aborted!" 
  exit 1
fi

# Build the plugin and protobuf compiler
bazel build @com_google_protobuf//:protoc
bazel build //firebase_rules_generator:protoc-gen-firebase_rules 

# Create output directories
mkdir -p bazel-genfiles/java
mkdir -p bazel-genfiles/javascript
mkdir -p bazel-genfiles/typescript
mkdir -p bazel-genfiles/objc
mkdir -p bazel-genfiles/swift

DIR_NAME=$(basename `pwd`)

# Generate models and firestore rules helpers
./bazel-bin/external/com_google_protobuf/protoc \
  --proto_path=./ \
  --proto_path=./bazel-$DIR_NAME/external/com_google_protobuf/src/ \
  --plugin=protoc-gen-firebase_rules=bazel-bin/firebase_rules_generator/protoc-gen-firebase_rules \
  --firebase_rules_out=bazel-genfiles/ \
  --java_out=bazel-genfiles/java/ \
  --objc_out=bazel-genfiles/objc/ \
  --js_out=bazel-genfiles/javascript/ \
  proto/example.proto

if hash node 2>/dev/null; then
  # Install JS deps
  npm install protobufjs
  # Generate typescript models
  ./node_modules/.bin/pbjs -t static-module -w commonjs -o bazel-genfiles/typescript/example.js proto/example.proto
  ./node_modules/.bin/pbts --out bazel-genfiles/typescript/example.d.ts bazel-genfiles/typescript/example.js
fi

# Generate swift models if on a mac
if [[ "$OSTYPE" == "darwin"* ]]; then
  pushd bazel-out/_tmp
  if [ ! -d "swift-protobuf" ]; then
    git clone https://github.com/apple/swift-protobuf.git
  fi
  cd swift-protobuf
  git checkout tags/1.0.2
  swift build -c release -Xswiftc -static-stdlib
  popd
  # Generate the swift models
  ./bazel-bin/external/com_google_protobuf/protoc \
    --proto_path=./ \
    --proto_path=./bazel-$DIR_NAME/external/com_google_protobuf/src/ \
    --plugin=protoc-gen-swift=bazel-out/_tmp/swift-protobuf/.build/release/protoc-gen-swift \
    --swift_out=bazel-genfiles/swift/ \
    proto/example.proto
fi

echo "Checkout bazel-genfiles/ for output model files and rules functions"
