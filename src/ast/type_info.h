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

#ifndef BINARY_READER_AST_TYPE_INFO_H_
#define BINARY_READER_AST_TYPE_INFO_H_

#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "ast/options.h"
#include "binary_reader/error_collection.h"
#include "binary_reader/value.h"
#include "util/buffered_file_reader.h"
#include "util/macros.h"
#include "util/size.h"

namespace binary_reader {

/// <summary>
/// A base class for information about a type usage.  This is used to determine
/// what type a data field is.
/// </summary>
class TypeInfoBase {
  NON_COPYABLE_OR_MOVABLE_TYPE(TypeInfoBase);

 public:
  TypeInfoBase(const std::string& alias_name, const std::string& base_name,
               std::optional<Size> static_size);
  virtual ~TypeInfoBase();

  static std::vector<std::shared_ptr<TypeInfoBase>> GetBuiltInTypes();

  const std::string& alias_name() const {
    return alias_name_;
  }
  const std::string& base_name() const {
    return base_name_;
  }
  std::optional<Size> static_size() const {
    return static_size_;
  }

  virtual bool equals(const TypeInfoBase& other) const;
  bool operator==(const TypeInfoBase& other) const {
    return equals(other);
  }
  bool operator!=(const TypeInfoBase& other) const {
    return !equals(other);
  }

  /// <summary>
  /// Reads a value from the given reader.  This moves the reader forward based
  /// on how large this type is.  The base implementation only returns an
  /// error.
  /// </summary>
  /// <param name="reader">The object to read from</param>
  /// <param name="bit_offset">The offset within the byte to start at.</param>
  /// <param name="result">Will be filled in with the result.</param>
  /// <param name="errors">Will be filled in with any errors.</param>
  /// <returns>True on success, false on error.</returns>
  virtual bool ReadValue(std::shared_ptr<BufferedFileReader> reader,
                         Value* result, ErrorCollection* errors) const;

 private:
  const std::string alias_name_;
  const std::string base_name_;
  const std::optional<Size> static_size_;
};

/// <summary>
/// Defines a type info about a built-in integer type.
/// </summary>
class IntegerTypeInfo sealed : public TypeInfoBase {
  NON_COPYABLE_OR_MOVABLE_TYPE(IntegerTypeInfo);

 public:
  IntegerTypeInfo(const std::string& alias_name, Size size, Signedness sign,
                  ByteOrder order);

  Signedness signedness() const {
    return sign_;
  }
  ByteOrder byte_order() const {
    return order_;
  }

  bool equals(const TypeInfoBase& other) const override;

  bool ReadValue(std::shared_ptr<BufferedFileReader> reader, Value* result,
                 ErrorCollection* errors) const override;

 private:
  const Signedness sign_;
  const ByteOrder order_;
};

}  // namespace binary_reader

#endif  // BINARY_READER_AST_TYPE_INFO_H_
