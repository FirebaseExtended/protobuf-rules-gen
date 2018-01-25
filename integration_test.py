#!/usr/local/env python
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

import sys
import os
from os import path
import subprocess


def check_proto_file(filename):
    assert isinstance(filename, str) and filename.endswith(".proto"), filename
    return filename


def check_rules_file(filename):
    assert isinstance(filename, str) and filename.endswith(".rules"), filename
    return filename


def read_file(filename):
    with open(filename, 'r') as f:
        return f.read()


def assert_equals(msg, expected, actual):
    assert expected == actual, msg + ": expected " + expected + " to equal " + actual


output_dir = os.environ["TEST_UNDECLARED_OUTPUTS_DIR"]

rules_plugin, protoc = sys.argv[1], sys.argv[2]
firebase_protos = path.dirname(sys.argv[3])
google_protos = path.dirname(sys.argv[4])

testdata = sys.argv[5:]


def run_testcase(proto_file, output):
    test_protos = path.dirname(proto_file)
    test_file = path.basename(proto_file)
    subprocess.check_call(
        [
            protoc,
            "--plugin=protoc-gen-firebase_rules=" + rules_plugin,
            "--firebase_rules_out=" + output_dir,
            "--proto_path=" + google_protos,
            "--proto_path=" + firebase_protos,
            "--proto_path=" + test_protos,
            test_file,
        ],
        stderr=sys.stderr)
    actual = read_file(output_dir + "/firestore.rules")
    expected = read_file(output)
    assert_equals(test_file, expected, actual)


# Testcases should be <name>.proto as the input and <name>.rules as the output.
testdata.sort()

for i in xrange(0, len(testdata), 2):
    proto_file = check_proto_file(testdata[i])
    rules_out = check_rules_file(testdata[i + 1])
    run_testcase(proto_file, rules_out)
