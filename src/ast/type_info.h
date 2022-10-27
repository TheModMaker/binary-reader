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

#include <any>
#include <memory>
#include <optional>
#include <string>
#include <unordered_set>
#include <vector>

#include "binary_reader/error_collection.h"
#include "binary_reader/options.h"
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
  TypeInfoBase(const DebugInfo& debug, const std::string& alias_name,
               const std::string& base_name, std::optional<Size> static_size);
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

  const DebugInfo& debug_info() const {
    return debug_;
  }

  /// <summary>
  /// Gets the possible options that can be set on the type.
  /// </summary>
  virtual std::unordered_set<OptionType> GetOptionTypes() const = 0;

  /// <summary>
  /// Clones the current instance with some values changed.
  /// </summary>
  /// <param name="debug">The new debug info.</param>
  /// <returns>A new instance, or nullptr on error.</returns>
  virtual std::shared_ptr<TypeInfoBase> Instantiate(const DebugInfo& debug,
                                                    Options options) const = 0;

  bool operator==(const TypeInfoBase& other) const {
    if (this == &other)
      return true;
    // Can only compare objects of the same type.
    if (typeid(*this) != typeid(other))
      return false;
    return Equals(other);
  }
  bool operator!=(const TypeInfoBase& other) const {
    return !(*this == other);
  }

  /// <summary>
  /// Reads a value from the given reader.  This moves the reader forward based
  /// on how large this type is.
  /// </summary>
  /// <param name="reader">The object to read from</param>
  /// <param name="bit_offset">The offset within the byte to start at.</param>
  /// <param name="result">Will be filled in with the result.</param>
  /// <param name="errors">Will be filled in with any errors.</param>
  /// <returns>True on success, false on error.</returns>
  virtual bool ReadValue(std::shared_ptr<BufferedFileReader> reader,
                         Value* result, ErrorCollection* errors) const = 0;

 protected:
  virtual bool Equals(const TypeInfoBase& other) const;

 private:
  const std::string alias_name_;
  const std::string base_name_;
  const std::optional<Size> static_size_;
  const DebugInfo debug_;
};

/// <summary>
/// Defines a type info about a built-in integer type.
/// </summary>
class IntegerTypeInfo sealed : public TypeInfoBase {
 public:
  IntegerTypeInfo(const DebugInfo& debug, const std::string& alias_name,
                  Size size, Signedness sign, ByteOrder order);

  Signedness signedness() const {
    return sign_;
  }
  ByteOrder byte_order() const {
    return order_;
  }

  std::unordered_set<OptionType> GetOptionTypes() const override;
  std::shared_ptr<TypeInfoBase> Instantiate(const DebugInfo& debug,
                                            Options options) const override;

  bool ReadValue(std::shared_ptr<BufferedFileReader> reader, Value* result,
                 ErrorCollection* errors) const override;

 private:
  bool Equals(const TypeInfoBase& other) const override;

  const Signedness sign_;
  const ByteOrder order_;
};

}  // namespace binary_reader

#endif  // BINARY_READER_AST_TYPE_INFO_H_
