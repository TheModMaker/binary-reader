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

namespace binary_reader {

std::ostream& operator<<(std::ostream& os, ValueType value) {
  switch (value) {
    case ValueType::Null:
      return os << "Null";
    case ValueType::Number:
      return os << "Number";
    case ValueType::String:
      return os << "String";
    case ValueType::Object:
      return os << "Object";
    default:
      return os << "<UNKNOWN ValueType>";
  }
}

Value::Value() {}
Value::Value(const Value&) = default;
Value::Value(Value&&) = default;
Value::~Value() = default;

Value& Value::operator=(const Value&) = default;
Value& Value::operator=(Value&&) = default;

}  // namespace binary_reader
