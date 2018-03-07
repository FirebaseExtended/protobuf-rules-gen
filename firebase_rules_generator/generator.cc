// Copyright 2017 Google LLC
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "generator.h"

#include <google/protobuf/stubs/logging.h>
#include <google/protobuf/stubs/strutil.h>
#include <algorithm>
#include <iterator>
#include <limits>
#include <string>

#include "proto/firebase_rules_options.pb.h"

#define RULES_FILE "firestore.rules"

namespace google {
namespace firebase {
namespace rules {
namespace experimental {

namespace {

struct RulesContext {};

std::string StripPrefix(const std::string &str, const std::string &prefix) {
  if (prefix.size() <= str.size() && str.substr(0, prefix.size()) == prefix) {
    return str.substr(prefix.size());
  } else {
    return str;
  }
}

std::string SanitizeName(const std::string &name) {
  // Strip leading '.' characters.
  std::string sanitized_name = StripPrefix(name, ".");
  for (size_t i = 0; i < sanitized_name.size(); ++i) {
    if (sanitized_name[i] == '.') {
      sanitized_name[i] = '_';
    }
  }
  return sanitized_name;
}

void ReturnIndent(protobuf::io::Printer &printer) {
  for (int i = 0; i < 4; ++i) {
    printer.Indent();
  }
}

void ReturnOutdent(protobuf::io::Printer &printer) {
  for (int i = 0; i < 4; ++i) {
    printer.Outdent();
  }
}

std::string GetMessageName(const protobuf::Descriptor *message) {
  RulesFileOptions file_options =
      message->file()->options().GetExtension(firebase_rules);
  if (file_options.full_package_names()) {
    return SanitizeName(message->full_name());
  } else {
    return SanitizeName(
        StripPrefix(message->full_name(), message->file()->package()));
  }
}

std::string GetEnumName(const protobuf::EnumDescriptor *enumeration) {
  RulesFileOptions file_options =
      enumeration->file()->options().GetExtension(firebase_rules);
  if (file_options.full_package_names()) {
    return SanitizeName(enumeration->full_name());
  } else {
    return SanitizeName(
        StripPrefix(enumeration->full_name(), enumeration->file()->package()));
  }
}

std::vector<std::string> RequiredFields(const protobuf::Descriptor *message) {
  std::vector<std::string> required;
  for (int i = 0; i < message->field_count(); ++i) {
    const auto *field = message->field(i);
    if (field->is_required() && field->containing_oneof() == nullptr) {
      required.push_back(field->json_name());
    }
  }
  return required;
}

std::vector<std::string> OptionalFields(const protobuf::Descriptor *message) {
  std::vector<std::string> optional;
  for (int i = 0; i < message->field_count(); ++i) {
    const auto *field = message->field(i);
    if ((field->is_optional() || field->is_repeated()) &&
        field->containing_oneof() == nullptr) {
      optional.push_back(field->json_name());
    }
  }
  return optional;
}

std::vector<std::vector<std::string>> OneOfFields(
    const protobuf::Descriptor *message) {
  std::vector<std::vector<std::string>> oneofs;
  for (int i = 0; i < message->oneof_decl_count(); ++i) {
    std::vector<std::string> oneof_names;
    const auto *oneof_decl = message->oneof_decl(i);
    for (int j = 0; j < oneof_decl->field_count(); ++j) {
      oneof_names.push_back(oneof_decl->field(j)->json_name());
    }
    oneofs.push_back(oneof_names);
  }
  return oneofs;
}

std::string ToString(std::vector<std::string> vec) {
  std::string result = "[";
  for (const auto &elem : vec) {
    result.push_back('\'');
    result.append(elem);
    result.append("',");
  }
  if (vec.empty()) {
    result.push_back(']');
  } else {
    result[result.size() - 1] = ']';
  }
  return result;
}

bool FieldsRespectOneOfs(
    const std::vector<std::string> &fields,
    const std::vector<std::vector<std::string>> &oneof_fields) {
  for (const auto &oneofs : oneof_fields) {
    int count = 0;
    for (const auto &oneof_name : oneofs) {
      auto pos = std::find(fields.begin(), fields.end(), oneof_name);
      if (pos != fields.end()) {
        ++count;
      }
    }
    if (count > 1) {
      return false;
    }
  }
  return true;
}

std::vector<std::vector<std::string>> GenerateAllKCombinations(
    const std::vector<std::string> &vec, size_t k) {
  std::vector<std::vector<std::string>> result;
  if (k > 0) {
    for (size_t i = 0; i < vec.size() - k + 1; ++i) {
      const auto &t = vec[i];
      std::vector<std::string> temp = vec;  // Make a copy
      auto it = temp.begin();
      std::advance(it, i + 1);
      temp.erase(temp.begin(), it);
      auto temp_result = GenerateAllKCombinations(temp, k - 1);
      for (auto &temp_item : temp_result) {
        temp_item.push_back(t);
        result.push_back(temp_item);
      }
    }
  } else {
    std::vector<std::string> empty;
    result.push_back(empty);
  }
  return result;
}

std::vector<std::vector<std::string>> AllFieldCombinations(
    const std::vector<std::string> &required_fields,
    const std::vector<std::string> &optional_fields,
    const std::vector<std::vector<std::string>> &oneof_fields) {
  std::vector<std::vector<std::string>> combos;

  size_t max_optional_props = optional_fields.size() + oneof_fields.size();
  // Add all possible optional and oneof fields, throw out invalid combos before
  // adding to the final set.
  std::vector<std::string> all_opt_fields = optional_fields;
  for (const auto &oneof : oneof_fields) {
    all_opt_fields.insert(all_opt_fields.end(), oneof.begin(), oneof.end());
  }
  for (size_t num_opts = 0; num_opts <= max_optional_props; ++num_opts) {
    const auto &k_combos = GenerateAllKCombinations(all_opt_fields, num_opts);
    for (const auto &combo : k_combos) {
      std::vector<std::string> fields_combo = required_fields;  // Make a copy
      fields_combo.insert(fields_combo.end(), combo.begin(), combo.end());
      if (FieldsRespectOneOfs(fields_combo, oneof_fields)) {
        combos.push_back(fields_combo);
      }
    }
  }
  return combos;
}

template <typename S>
bool IsLastIteration(S idx, S size) {
  return idx + 1 >= size;
}

}  // namespace

bool RulesGenerator::Generate(const protobuf::FileDescriptor *file,
                              const std::string &parameter,
                              protobuf::compiler::GeneratorContext *context,
                              std::string *error) const {
  protobuf::io::Printer printer(context->Open(RULES_FILE), '$');

  // Start by adding a comment
  printer.Print("// @@START_GENERATED_FUNCTIONS@@\n");

  for (int i = 0; i < file->message_type_count(); ++i) {
    const auto *message = file->message_type(i);
    if (!GenerateMessage(message, printer, error)) {
      return false;
    }
  }

  for (int i = 0; i < file->enum_type_count(); ++i) {
    const auto *enumeration = file->enum_type(i);
    if (!GenerateEnum(enumeration, printer, error)) {
      return false;
    }
  }

  // Skip any RPC services...

  // End by finishing that comment
  printer.Print("// @@END_GENERATED_FUNCTIONS@@\n");
  return true;
}

bool RulesGenerator::GenerateMessage(const protobuf::Descriptor *message,
                                     protobuf::io::Printer &printer,
                                     std::string *error) const {
  if (message->options().map_entry()) {
    return true;
  }
  const auto &options = message->options().GetExtension(firebase_rules_message);
  printer.Print("function is$name$Message(resource) {\n", "name",
                GetMessageName(message));
  printer.Indent();
  printer.Print("return ");
  ReturnIndent(printer);
  // Validate properties
  const auto &required_fields = RequiredFields(message);
  const auto &optional_fields = OptionalFields(message);
  const auto &oneof_fields = OneOfFields(message);
  auto combinations =
      AllFieldCombinations(required_fields, optional_fields, oneof_fields);
  if (options.has_extra_properties()) {
    combinations.clear();
    combinations.push_back(required_fields);
  }
  if (!combinations.empty()) printer.Print("(");
  for (size_t i = 0; i < combinations.size(); ++i) {
    const auto &combo = combinations[i];
    printer.Print("(resource.keys().hasAll($properties$)", "properties",
                  ToString(combo));
    if (!options.has_extra_properties()) {
      printer.Print(" && resource.size() == $count$)", "count",
                    std::to_string(combo.size()));
    } else {
      printer.Print(")");
    }
    if (IsLastIteration(i, combinations.size())) {
      printer.Print(")");
    } else {
      printer.Print(" ||\n");
    }
  }
  // Validate inner types
  if (message->field_count() > 0) printer.Print(" &&\n");
  for (int i = 0; i < message->field_count(); ++i) {
    if (!GenerateField(message->field(i), printer, error)) {
      return false;
    }
    if (!IsLastIteration(i, message->field_count())) {
      printer.Print(" &&\n");
    }
  }
  if (options.has_validate()) {
    printer.Print(" &&\n($validate$)", "validate", options.validate());
  }
  printer.Print(";");
  ReturnOutdent(printer);
  printer.Outdent();
  printer.Print("\n}\n");
  // Handle inner messages & enums
  for (int i = 0; i < message->nested_type_count(); ++i) {
    if (!GenerateMessage(message->nested_type(i), printer, error)) {
      return false;
    }
  }
  for (int i = 0; i < message->enum_type_count(); ++i) {
    if (!GenerateEnum(message->enum_type(i), printer, error)) {
      return false;
    }
  }
  return true;
}

bool RulesGenerator::GenerateEnum(const protobuf::EnumDescriptor *enumeration,
                                  protobuf::io::Printer &printer,
                                  std::string *error) const {
  const auto &options =
      enumeration->options().GetExtension(firebase_rules_enum);
  printer.Print("function is$name$Enum(resource) {\n", "name",
                GetEnumName(enumeration));
  printer.Indent();
  printer.Print("return ");
  ReturnIndent(printer);
  for (int i = 0; i < enumeration->value_count(); ++i) {
    const auto *enum_value = enumeration->value(i);
    if (options.has_string_values()) {
      printer.Print("resource == '$value$'", "value", enum_value->name());
    } else {
      printer.Print("resource == $value$", "value",
                    std::to_string(enum_value->number()));
    }
    if (!IsLastIteration(i, enumeration->value_count())) {
      printer.Print(" ||\n");
    }
  }
  printer.Print(";");
  ReturnOutdent(printer);
  printer.Outdent();
  printer.Print("\n}\n");
  return true;
}

bool RulesGenerator::GenerateField(const protobuf::FieldDescriptor *field,
                                   protobuf::io::Printer &printer,
                                   std::string *error) const {
  const auto &options = field->options().GetExtension(firebase_rules_field);
  printer.Print("((");
  if (field->is_optional() || field->is_repeated()) {
    printer.Print("!resource.keys().hasAny(['$name$'])) || (", "name",
                  field->json_name());
  }
  if (field->is_repeated() && !field->is_map()) {
    // We should validate the type inside the list, but currently we cannot
    // do that :(
    printer.Print("resource.$name$ is list", "name", field->json_name());
  }
  if (field->is_map()) {
    // https://github.com/google/protobuf/blob/d3537c/src/google/protobuf/descriptor.proto#L463-L484
    if (!GenerateMap(field, printer, error)) {
      return false;
    }
  }
  if (!field->is_repeated()) {
    std::map<std::string, std::string> vars;
    vars.insert({"name", field->json_name()});
    switch (field->type()) {
      case protobuf::FieldDescriptor::TYPE_DOUBLE:
      case protobuf::FieldDescriptor::TYPE_FLOAT:
        // TODO(rockwood): Do we need anything special for "Nan" or "Infinity"?
        vars.insert({"type", "float"});
        printer.Print(vars, "resource.$name$ is $type$");
        break;
      case protobuf::FieldDescriptor::TYPE_INT64:
      case protobuf::FieldDescriptor::TYPE_SINT64:
      case protobuf::FieldDescriptor::TYPE_SFIXED64:
        vars.insert({"type", "int"});
        vars.insert(
            {"min", std::to_string(std::numeric_limits<int64_t>::min())});
        vars.insert(
            {"max", std::to_string(std::numeric_limits<int64_t>::max())});
        printer.Print(vars,
                      "resource.$name$ is $type$ && resource.$name$ >= $min$ "
                      "&& resource.$name$ <= $max$");
        break;
      case protobuf::FieldDescriptor::TYPE_UINT64:
      case protobuf::FieldDescriptor::TYPE_FIXED64:
        vars.insert({"type", "int"});
        vars.insert(
            {"min", std::to_string(std::numeric_limits<uint64_t>::min())});
        // TODO(rockwood): According to the following guide, large numbers
        // can be strings as well. Can we handle this?
        // https://developers.google.com/protocol-buffers/docs/proto3#json
        // Anything over max int64 can't be represented in JSON.
        vars.insert(
            {"max", std::to_string(std::numeric_limits<int64_t>::max())});
        printer.Print(vars,
                      "resource.$name$ is $type$ && resource.$name$ >= $min$ "
                      "&& resource.$name$ <= $max$");
        break;
      case protobuf::FieldDescriptor::TYPE_INT32:
      case protobuf::FieldDescriptor::TYPE_SINT32:
      case protobuf::FieldDescriptor::TYPE_SFIXED32:
        vars.insert({"type", "int"});
        vars.insert(
            {"min", std::to_string(std::numeric_limits<int32_t>::min())});
        vars.insert(
            {"max", std::to_string(std::numeric_limits<int32_t>::max())});
        printer.Print(vars,
                      "resource.$name$ is $type$ && resource.$name$ >= $min$ "
                      "&& resource.$name$ <= $max$");
        break;
      case protobuf::FieldDescriptor::TYPE_UINT32:
      case protobuf::FieldDescriptor::TYPE_FIXED32:
        vars.insert({"type", "int"});
        vars.insert(
            {"min", std::to_string(std::numeric_limits<uint32_t>::min())});
        vars.insert(
            {"max", std::to_string(std::numeric_limits<uint32_t>::max())});
        printer.Print(vars,
                      "resource.$name$ is $type$ && resource.$name$ >= $min$ "
                      "&& resource.$name$ <= $max$");
        break;
      case protobuf::FieldDescriptor::TYPE_BOOL:
        vars.insert({"type", "bool"});
        printer.Print(vars, "resource.$name$ is $type$");
        break;
      case protobuf::FieldDescriptor::TYPE_STRING:
      case protobuf::FieldDescriptor::TYPE_BYTES:  // Base64 encoded strings.
        vars.insert({"type", "string"});
        printer.Print(vars, "resource.$name$ is $type$");
        break;
      case protobuf::FieldDescriptor::TYPE_GROUP:
        *error = "Groups are not supported.";
        return false;
      case protobuf::FieldDescriptor::TYPE_MESSAGE:
        vars.insert({"type", GetMessageName(field->message_type())});
        printer.Print(vars, "is$type$Message(resource.$name$)");
        break;
      case protobuf::FieldDescriptor::TYPE_ENUM:
        vars.insert({"type", GetEnumName(field->enum_type())});
        printer.Print(vars, "is$type$Enum(resource.$name$)");
        break;
    }
  }
  if (options.has_validate()) {
    printer.Print(" && ($validate$)", "validate", options.validate());
  }
  printer.Print("))");
  return true;
}

bool RulesGenerator::GenerateMap(const protobuf::FieldDescriptor *map_field,
                                 protobuf::io::Printer &printer,
                                 std::string *error) const {
  const auto *map = map_field->message_type();
  if (map->field_count() != 2) {
    *error = "Please use the map<> syntax to create maps!";
    return false;
  }
  const auto *key_field = map->FindFieldByName("key");
  if (key_field == nullptr) {
    *error = "Please use the map<> syntax to create maps!";
    return false;
  }
  const auto *value_field = map->FindFieldByName("value");
  if (value_field == nullptr) {
    *error = "Please use the map<> syntax to create maps!";
    return false;
  }
  if (key_field->type() != protobuf::FieldDescriptor::TYPE_STRING) {
    *error = "Firestore only supports `string` keys in maps.";
    return false;
  }
  // We should validate the values inside the map, but we currently cannot do
  // that :(
  printer.Print("resource.$name$ is map", "name", map_field->json_name());
  return true;
}

}  // namespace experimental
}  // namespace rules
}  // namespace firebase
}  // namespace google
