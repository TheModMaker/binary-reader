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

#ifndef BINARY_READER_INCLUDE_VALUE_H_
#define BINARY_READER_INCLUDE_VALUE_H_

#include <iostream>
#include <memory>
#include <type_traits>
#include <variant>

#include "binary_reader/number.h"
#include "binary_reader/utf_string.h"

namespace binary_reader {

class FileObject;

enum class ValueType {
  Null,
  Number,
  String,
  Object,
};

std::ostream& operator<<(std::ostream& os, ValueType value);

/// <summary>
/// Defines a generic value stored in a file.  Values have a dynamic type
/// similar to std::any, although the list of types it can be is fixed.  When
/// accessing the value, the stored type must match the requested format; this
/// will not convert values.
/// </summary>
class Value sealed {
 public:
  Value();
  template <typename T, typename = std::enable_if_t<
                            !std::is_same_v<std::decay_t<T>, Value> &&
                            !std::is_arithmetic_v<std::decay_t<T>>>>
  explicit Value(T&& value) : value_(std::forward<T>(value)) {}
  template <typename T, typename = std::enable_if_t<std::is_arithmetic_v<T>>>
  explicit Value(T value) : value_(Number(value)) {}
  Value(const Value&);
  Value(Value&&);
  ~Value();

  Value& operator=(const Value&);
  Value& operator=(Value&&);

  template <typename T, typename = std::enable_if_t<
                            !std::is_same_v<std::decay_t<T>, Value>>>
  Value& operator=(T&& value) {
    if constexpr (std::is_same_v<std::decay_t<T>, std::nullptr_t>)
      value_ = std::monostate();
    else
      value_ = std::move(value);
    return *this;
  }

  bool operator==(const Value& other) const {
    // TODO: Consider adding comparing objects.
    return value_ == other.value_;
  }
  bool operator<(const Value& other) const {
    // Since earlier variant entries are less than later elements, this
    // comparison is what we want.
    return value_ < other.value_;
  }

  bool operator!=(const Value& other) const {
    return !(*this == other);
  }
  bool operator<=(const Value& other) const {
    return *this == other || *this < other;
  }
  bool operator>=(const Value& other) const {
    return !(*this < other);
  }
  bool operator>(const Value& other) const {
    return !(*this <= other);
  }

  ValueType value_type() const {
    return static_cast<ValueType>(value_.index());
  }

  Number as_number() const {
    return std::get<Number>(value_);
  }
  UtfString as_string() const {
    return std::get<UtfString>(value_);
  }
  std::shared_ptr<FileObject> as_object() const {
    return std::get<std::shared_ptr<FileObject>>(value_);
  }

 private:
  // Order and types must match ValueType enum.
  std::variant<std::monostate, Number, UtfString, std::shared_ptr<FileObject>>
      value_;
};

// Define a custom compare for monostate so we can use variant operator<
inline bool operator<(std::monostate, std::monostate) {
  return false;
}

}  // namespace binary_reader

#endif  // BINARY_READER_INCLUDE_VALUE_H_
