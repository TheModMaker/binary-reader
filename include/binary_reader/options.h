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

#ifndef BINARY_READER_INCLUDE_OPTIONS_H_
#define BINARY_READER_INCLUDE_OPTIONS_H_

#include <any>
#include <iostream>
#include <type_traits>
#include <unordered_map>
#include <unordered_set>

#include "binary_reader/cloneable_unique_ptr.h"
#include "binary_reader/utf_string.h"

namespace binary_reader {

class Value;

enum class OptionType : uint16_t {
  Unknown = 0,
  Signedness,
  ByteOrder,
};

enum class Signedness : uint16_t {
  Unset = 0,
  Signed,
  Unsigned,
};

enum class ByteOrder : uint16_t {
  Unset = 0,
  LittleEndian,
  BigEndian,  // aka "network"
};

std::string to_string(OptionType type);
std::string to_string(Signedness signedness);
std::string to_string(ByteOrder byte_order);

inline std::ostream& operator<<(std::ostream& os, OptionType opt) {
  return os << to_string(opt);
}
inline std::ostream& operator<<(std::ostream& os, Signedness opt) {
  return os << to_string(opt);
}
inline std::ostream& operator<<(std::ostream& os, ByteOrder opt) {
  return os << to_string(opt);
}

OptionType GetOptionType(const UtfString& type);

class Options final {
 public:
  enum class ParseResult {
    /// <summary>
    /// The value was parsed successfully.
    /// </summary>
    Success,
    /// <summary>
    /// Tried to pass an invalid value type (e.g. an Object).
    /// </summary>
    InvalidValueType,
    /// <summary>
    /// The value was a string and doesn't map to a valid option.
    /// </summary>
    UnknownString,
    /// <summary>
    /// There are two or more types that have to this value.
    /// </summary>
    Ambiguous,
  };

  /// <summary>
  /// Creates a new Options object with Unset fields.
  /// </summary>
  Options();
  Options(const Options&);
  Options(Options&&);
  ~Options();

  Options& operator=(const Options&);
  Options& operator=(Options&&);

  /// <summary>
  /// Contains an Options object with the default settings.
  /// </summary>
  static const Options DefaultOptions;

  /// <summary>
  /// Parses an option value into its enum value.  If this is given |types|,
  /// those type will be used to determine which values are valid and to
  /// resolve ambiguous values; if |types| is empty, any type can be used.
  /// </summary>
  /// <param name="types">Possible types.</param>
  /// <param name="value">The value to parse.</param>
  /// <param name="result">The parsed enum value.</param>
  /// <returns>The parse result.</returns>
  static ParseResult ParseOption(const std::unordered_set<OptionType>& types,
                                 const Value& value, OptionType* result_type,
                                 std::any* result);

  /// <summary>
  /// Gets the value of the given option in this object; if the option is unset,
  /// return the option from |defaults|.
  /// </summary>
  /// <param name="type">The type of option to get.</param>
  /// <returns>The option's value.</returns>
  std::any GetOption(OptionType type, const Options& defaults) const;
  std::any GetOption(OptionType type) const {
    return GetOption(type, Options{});
  }

  /// <summary>
  /// Gets the option value that has the type T; if the option is unset, this
  /// returns |default_|.  This can only be called for types T that have an
  /// associated option and there is not more than one.
  /// </summary>
  /// <param name="default_">
  /// The default value to return if the option is unset.
  /// </param>
  /// <returns>The option's value, or the given default.</returns>
  template <typename T>
  T GetOption(T default_ = default(T)) {
    if constexpr (std::is_same<T, Signedness>::value) {
      if (signedness == Signedness::Unset)
        return default_;
      else
        return signedness;
    } else if constexpr (std::is_same<T, ByteOrder>::value) {
      if (byte_order == ByteOrder::Unset)
        return default_;
      else
        return byte_order;
    } else {
      // Must depend on T to work correctly
      static_assert(!std::is_same<T, T>::value, "Unknown option type");
    }
  }

  /// <summary>
  /// Sets the value of the given option.
  /// </summary>
  /// <param name="type">The option type to set.</param>
  /// <param name="value">
  /// The value to set to; the type must match exactly.
  /// </param>
  /// <returns>Whether the option existed and was set.</returns>
  bool SetOption(OptionType type, std::any value);

  //// Static general options

  Signedness signedness;
  ByteOrder byte_order;

 private:
  friend class OptionsTest;
  /// <summary>
  /// Testing only.  Validates the static data is correct.
  /// </summary>
  static bool CheckOptionData();

  // New fields are added here to not break ABI.
  struct Impl;
  cloneable_unique_ptr<Impl> impl_;
};

}  // namespace binary_reader

#endif  // BINARY_READER_INCLUDE_OPTIONS_H_
