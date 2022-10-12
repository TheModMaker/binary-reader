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

#include "binary_reader/number.h"

#include "util/templates.h"

namespace binary_reader {

namespace {

template <typename T, typename Variant>
T cast_variant_number(const Variant& value) {
  switch (value.index()) {
    case 0:
      return clamp_cast<T>(std::get<uintmax_t>(value));
    case 1:
      return clamp_cast<T>(std::get<intmax_t>(value));
    default:
    case 2:
      return clamp_cast<T>(std::get<double>(value));
  }
}

}  // namespace

Number::~Number() = default;

Number::Number(const Number& other) = default;
Number::Number(Number&& other) noexcept = default;
Number& Number::operator=(const Number& other) = default;
Number& Number::operator=(Number&& other) noexcept = default;

bool Number::is_negative() const {
  return number_type() == NumberType::SignedInt;
}

bool Number::is_double() const {
  return number_type() == NumberType::Double;
}

uintmax_t Number::as_unsigned() const {
  return cast_variant_number<uintmax_t>(value_);
}

intmax_t Number::as_signed() const {
  return cast_variant_number<intmax_t>(value_);
}

double Number::as_double() const {
  return cast_variant_number<double>(value_);
}

Number::operator bool() const {
  switch (number_type()) {
    case NumberType::UnsignedInt:
      return std::get<uintmax_t>(value_) != 0;
    case NumberType::SignedInt:
      return std::get<intmax_t>(value_) != 0;
    default:
    case NumberType::Double:
      return std::get<double>(value_) != 0;
  }
}

bool Number::operator==(const Number& other) const {
  const auto this_type = number_type();
  const auto other_type = other.number_type();
  if (this_type == NumberType::UnsignedInt) {
    if (other_type == NumberType::UnsignedInt) {
      return std::get<uintmax_t>(value_) == std::get<uintmax_t>(other.value_);
    } else if (other_type == NumberType::SignedInt) {
      return false;  // Signed version is only used for negative values
    } else {
      return std::get<uintmax_t>(value_) == std::get<double>(other.value_);
    }
  } else if (this_type == NumberType::SignedInt) {
    if (other_type == NumberType::UnsignedInt) {
      return false;  // Signed version is only used for negative values
    } else if (other_type == NumberType::SignedInt) {
      return std::get<intmax_t>(value_) == std::get<intmax_t>(other.value_);
    } else {
      return std::get<intmax_t>(value_) == std::get<double>(other.value_);
    }
  } else {
    if (other_type == NumberType::UnsignedInt) {
      return std::get<double>(value_) == std::get<uintmax_t>(other.value_);
    } else if (other_type == NumberType::SignedInt) {
      return std::get<double>(value_) == std::get<intmax_t>(other.value_);
    } else {
      return std::get<double>(value_) == std::get<double>(other.value_);
    }
  }
}

bool Number::operator<(const Number& other) const {
  const auto other_type = other.number_type();
  switch (number_type()) {
    case NumberType::UnsignedInt:
      if (other_type == NumberType::UnsignedInt) {
        return std::get<uintmax_t>(value_) < std::get<uintmax_t>(other.value_);
      } else if (other_type == NumberType::SignedInt) {
        return false;  // Signed version is only used for negative values
      } else {
        return static_cast<double>(std::get<uintmax_t>(value_)) <
               std::get<double>(other.value_);
      }
    case NumberType::SignedInt:
      if (other_type == NumberType::UnsignedInt) {
        return true;  // Signed version is only used for negative values
      } else if (other_type == NumberType::SignedInt) {
        return std::get<intmax_t>(value_) < std::get<intmax_t>(other.value_);
      } else {
        return static_cast<double>(std::get<intmax_t>(value_)) <
               std::get<double>(other.value_);
      }
    default:
    case NumberType::Double:
      if (other_type == NumberType::UnsignedInt) {
        return std::get<double>(value_) <
               static_cast<double>(std::get<uintmax_t>(other.value_));
      } else if (other_type == NumberType::SignedInt) {
        return std::get<double>(value_) <
               static_cast<double>(std::get<intmax_t>(other.value_));
      } else {
        return std::get<double>(value_) < std::get<double>(other.value_);
      }
  }
}

Number::NumberType Number::number_type() const {
  return static_cast<NumberType>(value_.index());
}

std::ostream& operator<<(std::ostream& os, const Number& value) {
  if (value.is_double())
    return os << value.as_double();
  else if (value.is_negative())
    return os << value.as_signed();
  else
    return os << value.as_unsigned();
}

}  // namespace binary_reader
