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
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef BINARY_READER_INCLUDE_NUMBER_H_
#define BINARY_READER_INCLUDE_NUMBER_H_

#include <iostream>
#include <type_traits>
#include <variant>

namespace binary_reader {

/// <summary>
/// Defines a smart wrapper around a generic number.  This number type holds a
/// max sized integer or double and converts between them as needed.  Values are
/// compared by value, the signedness is handled like you'd expect of normal
/// numbers.
/// </summary>
class Number final {
 public:
  Number() : Number(0) {}
  template <typename T, typename = std::enable_if_t<std::is_arithmetic_v<T>>>
  explicit Number(T value) {
    if constexpr (std::is_floating_point_v<T>) {
      value_ = static_cast<double>(value);
    } else if constexpr (std::is_signed_v<T>) {
      if (value < 0)
        value_ = static_cast<intmax_t>(value);
      else
        value_ = static_cast<uintmax_t>(value);
    } else {
      value_ = static_cast<uintmax_t>(value);
    }
  }
  ~Number();

  Number(const Number&);
  Number(Number&&) noexcept;
  Number& operator=(const Number&);
  Number& operator=(Number&&) noexcept;

  template <typename T>
  Number& operator=(T&& value) {
    Number temp{std::forward<T>(value)};
    swap(value_, temp.value_);
    return *this;
  }

  operator bool() const;
  bool operator==(const Number& other) const;
  bool operator<(const Number& other) const;

  bool operator!=(const Number& other) const {
    return !(*this == other);
  }
  bool operator<=(const Number& other) const {
    return *this == other || *this < other;
  }
  bool operator>=(const Number& other) const {
    return !(*this < other);
  }
  bool operator>(const Number& other) const {
    return !(*this <= other);
  }

  bool is_negative() const;
  bool is_double() const;

  uintmax_t as_unsigned() const;
  intmax_t as_signed() const;
  double as_double() const;

 private:
  enum class NumberType {
    UnsignedInt,
    SignedInt,
    Double,
  };

  NumberType number_type() const;

  // Note that intmax_t is only used for negative values; positive integers
  // always use uintmax_t
  std::variant<uintmax_t, intmax_t, double> value_;
};

std::ostream& operator<<(std::ostream& os, const Number& value);

}  // namespace binary_reader

#endif  // BINARY_READER_INCLUDE_NUMBER_H_
