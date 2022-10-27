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

#ifndef BINARY_READER_AST_AST_BASE_H_
#define BINARY_READER_AST_AST_BASE_H_

#include <typeinfo>

#include "binary_reader/error.h"
#include "util/macros.h"

namespace binary_reader {

/// <summary>
/// Represents the abstract base class of AST types.
/// <summary>
class AstBase {
  NON_COPYABLE_OR_MOVABLE_TYPE(AstBase);

 public:
  AstBase(const DebugInfo& debug) : debug_(debug) {}
  virtual ~AstBase() {}

  const DebugInfo& debug_info() const {
    return debug_;
  }

  bool operator==(const AstBase& other) const {
    if (this == &other)
      return true;
    // Can only compare objects of the same type.
    if (typeid(*this) != typeid(other))
      return false;
    return Equals(other);
  }
  bool operator!=(const AstBase& other) const {
    return !(*this == other);
  }

 private:
  /// <summary>
  /// Checks the given object is equal to |*this|.  This is only called if
  /// |other| is the same type as |*this|.
  /// </summary>
  virtual bool Equals(const AstBase& other) const = 0;

  const DebugInfo debug_;
};

class Statement : public AstBase {
  using AstBase::AstBase;
};

}  // namespace binary_reader

#endif  // BINARY_READER_AST_AST_BASE_H_
