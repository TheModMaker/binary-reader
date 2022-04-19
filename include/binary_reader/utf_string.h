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

#include <memory>
#include <string>

#include "binary_reader/codecs.h"

namespace binary_reader {

/// <summary>
/// Defines a Unicode-aware string type that supports converting between
/// different character encodings.
/// </summary>
class UtfString {
 public:
  UtfString();
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
  /// Converts the current string to bytes using the given Codec.
  /// </summary>
  std::vector<uint8_t> AsBytes(std::shared_ptr<Codec> codec,
                               ErrorInfo* error) const;

  /// <summary>
  /// Returns whether the string is empty.
  /// </summary>
  bool empty() const {
    return utf16_buffer_.empty();
  }

 private:
  std::u16string utf16_buffer_;
};

}  // namespace binary_reader

#endif  // BINARY_READER_INCLUDE_UTF_STRING_H_
