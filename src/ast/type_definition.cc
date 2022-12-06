// Copyright 2022 Google LLC
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissionsand
// limitations under the License.

#include "ast/type_definition.h"

#include <cassert>

#include "ast/field_info.h"
#include "binary_reader/file_object.h"
#include "public/file_object_init.h"

namespace binary_reader {

namespace {

std::optional<Size> CalculateSize(
    const std::vector<std::shared_ptr<Statement>>& statements) {
  Size size;
  for (const auto& stmt : statements) {
    if (auto field = std::dynamic_pointer_cast<FieldInfo>(stmt)) {
      if (!field->type() || !field->type()->static_size())
        return std::nullopt;
      size += *field->type()->static_size();
    } else {
      assert(false);
    }
  }
  return size;
}

}  // namespace

TypeDefinition::TypeDefinition(
    const DebugInfo& debug, const std::string& name,
    std::vector<std::shared_ptr<Statement>> statements)
    : TypeInfoBase(debug, name, name, CalculateSize(statements)),
      Statement(debug),
      statements_(std::move(statements)) {}

bool TypeDefinition::ReadValue(std::shared_ptr<BufferedFileReader> reader,
                               Value* result, ErrorCollection* errors) const {
  FileObjectInit init;
  init.file = reader;
  init.type = shared_from_this();
  init.start_position = reader->position();

  auto ret = MakeFileObject(init);
  if (!ret->ReparseObject(errors))
    return false;
  *result = ret;
  return reader->Seek(init.start_position + *static_size(), errors);
}

std::unordered_set<OptionType> TypeDefinition::GetOptionTypes() const {
  return {};
}

std::shared_ptr<TypeInfoBase> TypeDefinition::Instantiate(
    const DebugInfo& debug, Options) const  {
  return std::make_shared<TypeDefinition>(debug, alias_name(), statements_);
}

bool TypeDefinition::Equals(const TypeInfoBase& other) const {
  return Equals(static_cast<const TypeDefinition&>(other));
}

bool TypeDefinition::Equals(const AstBase& other) const {
  return Equals(static_cast<const TypeDefinition&>(other));
}

bool TypeDefinition::Equals(const TypeDefinition& other) const {
  if (!TypeInfoBase::Equals(other) ||
      statements_.size() != other.statements_.size()) {
    return false;
  }
  for (size_t i = 0; i < statements_.size(); i++) {
    if (*statements_[i] != *other.statements_[i])
      return false;
  }
  return true;
}

}  // namespace binary_reader
