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

#ifndef BINARY_READER_AST_TYPE_DEFINITION_H_
#define BINARY_READER_AST_TYPE_DEFINITION_H_

#include <memory>
#include <string>
#include <tuple>
#include <vector>

#include "ast/ast_base.h"
#include "ast/type_info.h"
#include "binary_reader/error.h"
#include "util/macros.h"

namespace binary_reader {

/// <summary>
/// Defines a type definition.
/// </summary>
/// <example>
/// type Example {
///   ...
/// }
/// </example>
class TypeDefinition sealed
    : public TypeInfoBase,
      public Statement,
      public std::enable_shared_from_this<TypeDefinition> {
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
  bool Equals(const TypeInfoBase& other) const override;
  bool Equals(const AstBase& other) const override;
  bool Equals(const TypeDefinition& other) const;

  const std::vector<std::shared_ptr<Statement>> statements_;
};

}  // namespace binary_reader

#endif  // BINARY_READER_AST_TYPE_DEFINITION_H_
