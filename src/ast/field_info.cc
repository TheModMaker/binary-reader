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

#include "ast/field_info.h"

namespace binary_reader {

FieldInfo::FieldInfo(const DebugInfo& debug, const std::string& name,
                     std::shared_ptr<TypeInfoBase> type)
    : Statement(debug), name_(name), type_(type) {}

bool FieldInfo::Equals(const AstBase& other) const {
  auto* o = static_cast<const FieldInfo*>(&other);
  return name_ == o->name_ && *type_ == *o->type_;
}

}  // namespace binary_reader
