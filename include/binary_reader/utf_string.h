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

#ifndef BINARY_READER_INCLUDE_UTF_STRING_H_
#define BINARY_READER_INCLUDE_UTF_STRING_H_

#include <iostream>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "binary_reader/error.h"

namespace binary_reader {

class Codec;

/// <summary>
/// Defines a Unicode-aware string type that supports converting between
/// different character encodings.
/// </summary>
class UtfString {
 public:
  UtfString();
  explicit UtfString(const std::u16string& str);
  ~UtfString();
  UtfString(const UtfString&);
  UtfString(UtfString&&);
  UtfString& operator=(const UtfString&);
  UtfString& operator=(UtfString&&);

  bool operator==(const UtfString& other) const {
    return utf16_buffer_ == other.utf16_buffer_;
  }
  bool operator!=(const UtfString& other) const {
    return utf16_buffer_ != other.utf16_buffer_;
  }

  bool operator<(const UtfString& other) const {
    return utf16_buffer_ < other.utf16_buffer_;
  }
  bool operator<=(const UtfString& other) const {
    return utf16_buffer_ <= other.utf16_buffer_;
  }
  bool operator>(const UtfString& other) const {
    return utf16_buffer_ > other.utf16_buffer_;
  }
  bool operator>=(const UtfString& other) const {
    return utf16_buffer_ >= other.utf16_buffer_;
  }

  /// <summary>
  /// Creates a string from the given bytes.
  /// </summary>
  /// <param name="buffer">The input buffer.</param>
  /// <param name="size">The number of bytes in the buffer.</param>
  /// <param name="codec">The Codec object to use.</param>
  /// <param name="error">Will be filled with any errors that happen.</param>
  /// <returns>The resulting string.</returns>
  static UtfString FromEncoding(const uint8_t* bytes, size_t size,
                                std::shared_ptr<Codec> codec, ErrorInfo* error);
  static UtfString FromEncoding(const char* bytes, size_t size,
                                std::shared_ptr<Codec> codec, ErrorInfo* error);

  /// <summary>
  /// Converts the given UTF-8 encoded string into a UtfString.  This uses the
  /// built-in decoder and uses replacement characters for errors.
  ///
  /// Care should be taken when using this function as the input may not
  /// actually be UTF-8.  This is especially true on Windows, where it will
  /// usually use a system multi-byte encoding.
  /// </summary>
  /// <param name="str">The string to convert.</param>
  /// <returns>The converted string.</returns>
  static UtfString FromUtf8(const std::string& str);

  /// <summary>
  /// Converts the current string to bytes using the given Codec.
  /// </summary>
  std::vector<uint8_t> AsBytes(std::shared_ptr<Codec> codec,
                               ErrorInfo* error) const;

  /// <summary>
  /// Converts the current string to a UTF-8 encoded string.  This uses the
  /// built-in decoder and uses replacement characters for errors.
  ///
  /// Care should be taken when using this function as the output may not
  /// be using UTF-8.  This is especially true on Windows, where it will
  /// usually use a system multi-byte encoding.
  /// </summary>
  /// <returns>The converted string.</returns>
  std::string AsUtf8() const;

  /// <summary>
  /// Converts the current string to a UTF-16 encoded string.  Since this type
  /// uses UTF-16 internally, this will be the exact value, even with errors.
  ///
  /// On Windows, this should be usable for Unicode methods (e.g. CreateFileW).
  /// </summary>
  /// <returns>The converted string.</returns>
  std::u16string AsUtf16() const {
    return utf16_buffer_;
  }

  /// <summary>
  /// Returns whether the string is empty.
  /// </summary>
  bool empty() const {
    return utf16_buffer_.empty();
  }

 private:
  std::u16string utf16_buffer_;
};

std::ostream& operator<<(std::ostream& os, const UtfString& str);

}  // namespace binary_reader

template <>
struct std::hash<binary_reader::UtfString> {
  std::size_t operator()(const binary_reader::UtfString& str) const noexcept {
    return std::hash<std::u16string>()(str.AsUtf16());
  }
};

#endif  // BINARY_READER_INCLUDE_UTF_STRING_H_
