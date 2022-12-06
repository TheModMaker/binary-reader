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

#ifndef BINARY_READER_AST_EXPRESSION_H_
#define BINARY_READER_AST_EXPRESSION_H_

#include "ast/ast_base.h"
#include "binary_reader/value.h"

namespace binary_reader {

class Literal final : public Expression {
 public:
  Literal(const DebugInfo& debug, Value value);

  const Value& value() const {
    return value_;
  }

 private:
  bool Equals(const AstBase& other) const override;

  const Value value_;
};

}  // namespace binary_reader

#endif  // BINARY_READER_AST_EXPRESSION_H_
