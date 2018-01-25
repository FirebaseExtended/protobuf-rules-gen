/*
 * Copyright 2017 Google LLC
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef FIREBASE_RULES_PROTOC_GENERATOR_H
#define FIREBASE_RULES_PROTOC_GENERATOR_H

#include <google/protobuf/compiler/code_generator.h>
#include <google/protobuf/descriptor.h>
#include <google/protobuf/io/printer.h>

#include <string>

namespace google {
namespace firebase {
namespace rules {
namespace experimental {

class RulesGenerator : public protobuf::compiler::CodeGenerator {
 public:
  RulesGenerator() = default;
  ~RulesGenerator() override = default;

  bool Generate(const google::protobuf::FileDescriptor* file,
                const std::string& parameter,
                protobuf::compiler::GeneratorContext* context,
                std::string* error) const override;

 private:
  bool GenerateMessage(const protobuf::Descriptor* message,
                       protobuf::io::Printer& printer,
                       std::string* error) const;

  bool GenerateEnum(const protobuf::EnumDescriptor* enumeration,
                    protobuf::io::Printer& printer, std::string* error) const;

  bool GenerateField(const protobuf::FieldDescriptor* field,
                     protobuf::io::Printer& printer, std::string* error) const;

  bool GenerateOneOf(const protobuf::OneofDescriptor* oneof,
                     protobuf::io::Printer& printer, std::string* error) const;

  bool GenerateMap(const protobuf::FieldDescriptor* map_field,
                   protobuf::io::Printer& printer, std::string* error) const;
};

}  // namespace experimental
}  // namespace rules
}  // namespace firebase
}  // namespace google

#endif  // FIREBASE_RULES_PROTOC_GENERATOR_H
