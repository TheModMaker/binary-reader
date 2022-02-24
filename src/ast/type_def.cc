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

#include "ast/type_def.h"

namespace binary_reader {

Statement::Statement() {}
Statement::~Statement() {}

FieldInfo::FieldInfo(const std::string& name,
                     std::shared_ptr<TypeInfoBase> type)
    : name_(name), type_(type) {}

bool FieldInfo::equals(const Statement& other) const {
  if (auto* o = dynamic_cast<const FieldInfo*>(&other)) {
    return name_ == o->name_ && *type_ == *o->type_;
  }
  return false;
}

TypeDefinition::TypeDefinition(const std::string& name)
    : TypeInfoBase(name, name) {}

bool TypeDefinition::equals(const TypeInfoBase& other) const {
  if (auto* o = dynamic_cast<const TypeDefinition*>(&other)) {
    return equals(o);
  }
  return false;
}

bool TypeDefinition::equals(const Statement& other) const {
  if (auto* o = dynamic_cast<const TypeDefinition*>(&other)) {
    return equals(o);
  }
  return false;
}

bool TypeDefinition::equals(const TypeDefinition* other) const {
  if (!TypeInfoBase::equals(*other) ||
      statements_.size() != other->statements_.size()) {
    return false;
  }
  for (size_t i = 0; i < statements_.size(); i++) {
    if (*statements_[i] != *other->statements_[i])
      return false;
  }
  return true;
}

}  // namespace binary_reader
