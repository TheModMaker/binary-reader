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

#ifndef BINARY_READER_AST_OPTIONS_H_
#define BINARY_READER_AST_OPTIONS_H_

#include <string>
#include <type_traits>

namespace binary_reader {

enum class Signedness {
  Unset,
  Signed,
  Unsigned,
};

enum class ByteOrder {
  Unset,
  LittleEndian,
  BigEndian,  // aka "network"
};

template <typename T>
bool ParseOption(const std::string& value, T* result) {
  if constexpr (std::is_same<T, Signedness>::value) {
    if (value == "signed") {
      *result = Signedness::Signed;
      return true;
    } else if (value == "unsigned") {
      *result = Signedness::Unsigned;
      return true;
    }
  } else if constexpr (std::is_same<T, ByteOrder>::value) {
    if (value == "big" || value == "network") {
      *result = ByteOrder::BigEndian;
      return true;
    } else if (value == "little") {
      *result = ByteOrder::LittleEndian;
      return true;
    }
  } else {
    // This condition is always false, but needs to depend on T.
    static_assert(std::is_same<T, Signedness>::value,
                  "Unknown option type specified");
  }
  return false;
}

}  // namespace binary_reader

#endif  // BINARY_READER_AST_OPTIONS_H_
