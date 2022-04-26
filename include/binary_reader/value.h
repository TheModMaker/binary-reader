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

#include <memory>
#include <type_traits>
#include <variant>

#include "binary_reader/utf_string.h"

namespace binary_reader {

class FileObject;

enum class ValueType {
  Null,
  Boolean,
  UnsignedInt,
  SignedInt,
  Double,
  String,
  Object,
};

/// <summary>
/// Defines a generic value stored in a file.  Values have a dynamic type
/// similar to std::any, although the list of types it can be is fixed.  This
/// will convert values as needed to different types; this will clamp results at
/// min/max value (e.g. -10 is 0 from as_unsigned).
///
/// This can store different types of integers (signed, unsigned, and doubles).
/// Numbers are stored as accurately as given.  For the sake of comparisons,
/// they are compared by value.  Other value types are compared as follows:
///
/// null < boolean < numbers < objects
/// </summary>
class Value sealed {
 public:
  Value() : Value(nullptr) {}
  template <typename T, typename = std::enable_if_t<
                            std::is_same_v<T, std::nullptr_t> ||
                            std::is_same_v<T, UtfString> ||
                            std::is_same_v<T, std::shared_ptr<FileObject>> ||
                            std::is_same_v<T, bool> || std::is_arithmetic_v<T>>>
  explicit Value(T value) {
    if constexpr (std::is_same_v<T, std::nullptr_t> ||
                  std::is_same_v<T, UtfString> ||
                  std::is_same_v<T, std::shared_ptr<FileObject>> ||
                  std::is_same_v<T, bool>) {
      value_ = value;
    } else if constexpr (std::is_arithmetic_v<T>) {
      if constexpr (std::is_floating_point_v<T>) {
        value_ = static_cast<double>(value);
      } else if constexpr (std::is_signed_v<T>) {
        value_ = static_cast<int64_t>(value);
      } else {
        value_ = static_cast<uint64_t>(value);
      }
    } else {
      // This condition is always false, but needs to depend on T.
      static_assert(std::is_same<T, bool>::value,
                    "Unable to create Value from this type.");
    }
  }
  Value(const Value&);
  Value(Value&&);
  ~Value();

  Value& operator=(const Value&);
  Value& operator=(Value&&);

  template <typename T>
  Value& operator=(T&& value) {
    Value temp{std::forward<T>(value)};
    swap(value_, temp.value_);
    return *this;
  }

  bool operator==(const Value& other) const;
  bool operator<(const Value& other) const;

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
  bool is_null() const {
    return value_.index() == 0;
  }
  bool is_bool() const {
    return value_.index() == 1;
  }
  bool is_number() const {
    return value_.index() > 1 && value_.index() < 5;
  }
  bool is_string() const {
    return value_.index() == 5;
  }
  bool is_object() const {
    return value_.index() == 6;
  }

  bool as_bool() const;
  uint64_t as_unsigned() const;
  int64_t as_signed() const;
  double as_double() const;
  UtfString as_string() const;
  std::shared_ptr<FileObject> as_object() const;

 private:
  // Order and types must match ValueType enum.
  std::variant<std::nullptr_t, bool, uint64_t, int64_t, double, UtfString,
               std::shared_ptr<FileObject>>
      value_;
};

}  // namespace binary_reader

#endif  // BINARY_READER_INCLUDE_VALUE_H_
