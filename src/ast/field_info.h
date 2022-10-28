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

#ifndef BINARY_READER_AST_FIELD_INFO_H_
#define BINARY_READER_AST_FIELD_INFO_H_

#include <memory>
#include <string>

#include "ast/ast_base.h"
#include "ast/type_info.h"

namespace binary_reader {

/// <summary>
/// Defines a field definition within a type.
/// </summary>
/// <example>
/// int32 foo;
/// </example>
class FieldInfo sealed : public Statement {
 public:
  FieldInfo(const DebugInfo& debug, const std::string& name,
            std::shared_ptr<TypeInfoBase> type,
            std::shared_ptr<Expression> expected = nullptr);

  const std::string& name() const {
    return name_;
  }

  std::shared_ptr<TypeInfoBase> type() const {
    return type_;
  }

  std::shared_ptr<Expression> expected() const {
    return expected_;
  }

 private:
  bool Equals(const AstBase& other) const override;

  const std::string name_;
  const std::shared_ptr<TypeInfoBase> type_;
  const std::shared_ptr<Expression> expected_;
};

}  // namespace binary_reader

#endif  // BINARY_READER_AST_FIELD_INFO_H_
