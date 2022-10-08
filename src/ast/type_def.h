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

#ifndef BINARY_READER_AST_TYPE_DEF_H_
#define BINARY_READER_AST_TYPE_DEF_H_

#include <memory>
#include <string>
#include <tuple>
#include <vector>

#include "ast/type_info.h"
#include "binary_reader/error.h"
#include "util/macros.h"

namespace binary_reader {

class Statement {
  NON_COPYABLE_OR_MOVABLE_TYPE(Statement);

 public:
  Statement();
  virtual ~Statement();

  virtual bool equals(const Statement& other) const = 0;
  bool operator==(const Statement& other) const {
    return equals(other);
  }
  bool operator!=(const Statement& other) const {
    return !equals(other);
  }
};

class FieldInfo sealed : public Statement {
  NON_COPYABLE_OR_MOVABLE_TYPE(FieldInfo);

 public:
  FieldInfo(const std::string& name, std::shared_ptr<TypeInfoBase> type);

  const std::string& name() const {
    return name_;
  }
  std::shared_ptr<TypeInfoBase> type() const {
    return type_;
  }

 private:
  bool equals(const Statement& other) const override;

  const std::string name_;
  const std::shared_ptr<TypeInfoBase> type_;
};

class TypeDefinition sealed
    : public TypeInfoBase,
      public Statement,
      public std::enable_shared_from_this<TypeDefinition> {
  NON_COPYABLE_OR_MOVABLE_TYPE(TypeDefinition);

 public:
  TypeDefinition(const DebugInfo& debug, const std::string& name,
                 std::vector<std::shared_ptr<Statement>> statements);

  const std::vector<std::shared_ptr<Statement>>& statements() const {
    return statements_;
  }

  bool ReadValue(std::shared_ptr<BufferedFileReader> reader, Value* result,
                 ErrorCollection* errors) const override;

  std::unordered_set<OptionType> GetOptionTypes() const override;
  std::shared_ptr<TypeInfoBase> Instantiate(
      const DebugInfo& debug, Options options) const override;

 private:
  bool equals(const TypeInfoBase& other) const override;
  bool equals(const Statement& other) const override;
  bool equals(const TypeDefinition* other) const;

  const std::vector<std::shared_ptr<Statement>> statements_;
};

}  // namespace binary_reader

#endif  // BINARY_READER_AST_TYPE_DEF_H_
