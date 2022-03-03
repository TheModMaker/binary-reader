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

#include "binary_reader/value.h"

#include <algorithm>
#include <limits>

#include "util/templates.h"

namespace binary_reader {

namespace {

template <typename T, typename Variant>
T cast_variant_number(const Variant& value) {
  switch (static_cast<ValueType>(value.index())) {
    default:
    case ValueType::Null:
      return 0;
    case ValueType::Boolean:
      return std::get<bool>(value) ? 1 : 0;
    case ValueType::UnsignedInt:
      return clamp_cast<T>(std::get<uint64_t>(value));
    case ValueType::SignedInt:
      return clamp_cast<T>(std::get<int64_t>(value));
    case ValueType::Double:
      return clamp_cast<T>(std::get<double>(value));
  }
}

}  // namespace

Value::Value(const Value&) = default;
Value::Value(Value&&) = default;
Value::~Value() = default;

Value& Value::operator=(const Value&) = default;
Value& Value::operator=(Value&&) = default;

bool Value::as_bool() const {
  switch (value_type()) {
    default:
    case ValueType::Null:
      return false;
    case ValueType::Boolean:
      return std::get<bool>(value_);
    case ValueType::UnsignedInt:
      return std::get<uint64_t>(value_) != 0;
    case ValueType::SignedInt:
      return std::get<int64_t>(value_) != 0;
    case ValueType::Double:
      return std::get<double>(value_) != 0;
  }
}

uint64_t Value::as_unsigned() const {
  return cast_variant_number<uint64_t>(value_);
}

int64_t Value::as_signed() const {
  return cast_variant_number<int64_t>(value_);
}

double Value::as_double() const {
  return cast_variant_number<double>(value_);
}

bool Value::operator==(const Value& other) const {
  const auto this_type = value_type();
  const auto other_type = other.value_type();
  if (this_type == ValueType::UnsignedInt) {
    if (other_type == ValueType::UnsignedInt) {
      return std::get<uint64_t>(value_) == std::get<uint64_t>(other.value_);
    } else if (other_type == ValueType::SignedInt) {
      return std::get<int64_t>(other.value_) >= 0 &&
             std::get<uint64_t>(value_) == std::get<int64_t>(other.value_);
    } else if (other_type == ValueType::Double) {
      return std::get<uint64_t>(value_) == std::get<double>(other.value_);
    }
  } else if (this_type == ValueType::SignedInt) {
    if (other_type == ValueType::UnsignedInt) {
      return std::get<int64_t>(value_) >= 0 &&
             std::get<int64_t>(value_) == std::get<uint64_t>(other.value_);
    } else if (other_type == ValueType::SignedInt) {
      return std::get<int64_t>(value_) == std::get<int64_t>(other.value_);
    } else if (other_type == ValueType::Double) {
      return std::get<int64_t>(value_) == std::get<double>(other.value_);
    }
  } else if (this_type == ValueType::Double) {
    if (other_type == ValueType::UnsignedInt) {
      return std::get<double>(value_) == std::get<uint64_t>(other.value_);
    } else if (other_type == ValueType::SignedInt) {
      return std::get<double>(value_) == std::get<int64_t>(other.value_);
    } else if (other_type == ValueType::Double) {
      return std::get<double>(value_) == std::get<double>(other.value_);
    }
  }

  return value_ == other.value_;
}

bool Value::operator<(const Value& other) const {
  // null < bool < numbers
  const auto other_type = other.value_type();
  switch (value_type()) {
    default:
    case ValueType::Null:
      return other_type != ValueType::Null;
    case ValueType::Boolean:
      if (other_type == ValueType::Boolean)
        return std::get<bool>(value_) < std::get<bool>(other.value_);
      else
        return other_type != ValueType::Null;
    case ValueType::UnsignedInt:
      if (other_type == ValueType::UnsignedInt) {
        return std::get<uint64_t>(value_) < std::get<uint64_t>(other.value_);
      } else if (other_type == ValueType::SignedInt) {
        return std::get<int64_t>(other.value_) >= 0 &&
               std::get<uint64_t>(value_) <
                   static_cast<uint64_t>(std::get<int64_t>(other.value_));
      } else if (other_type == ValueType::Double) {
        return static_cast<double>(std::get<uint64_t>(value_)) <
               std::get<double>(other.value_);
      } else {
        return false;
      }
    case ValueType::SignedInt:
      if (other_type == ValueType::UnsignedInt) {
        return std::get<int64_t>(value_) < 0 ||
               static_cast<uint64_t>(std::get<int64_t>(value_)) <
                   std::get<uint64_t>(other.value_);
      } else if (other_type == ValueType::SignedInt) {
        return std::get<int64_t>(value_) < std::get<int64_t>(other.value_);
      } else if (other_type == ValueType::Double) {
        return static_cast<double>(std::get<int64_t>(value_)) <
               std::get<double>(other.value_);
      } else {
        return false;
      }
    case ValueType::Double:
      if (other_type == ValueType::UnsignedInt) {
        return std::get<double>(value_) <
               static_cast<double>(std::get<uint64_t>(other.value_));
      } else if (other_type == ValueType::SignedInt) {
        return std::get<double>(value_) <
               static_cast<double>(std::get<int64_t>(other.value_));
      } else if (other_type == ValueType::Double) {
        return std::get<double>(value_) < std::get<double>(other.value_);
      } else {
        return false;
      }
  }
}

}  // namespace binary_reader
