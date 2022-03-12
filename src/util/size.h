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

#ifndef BINARY_READER_UTIL_SIZE_H_
#define BINARY_READER_UTIL_SIZE_H_

#include <cstdint>
#include <iostream>

namespace binary_reader {

/// <summary>
/// An abstraction around the size within a binary file.  This internally
/// just uses the bit count and converts to other sizes.  Using this
/// type ensures we always use the correct values and makes it all explicit.
/// </summary>
class Size sealed {
 public:
  Size() : pos_(0) {}

  static Size FromBytes(uint64_t bytes) {
    return Size(bytes * 8);
  }

  static Size FromBytesAndOffset(uint64_t bytes, uint8_t bit_offset) {
    return Size(bytes * 8 + bit_offset);
  }

  static Size FromBits(uint64_t bits) {
    return Size(bits);
  }

  Size operator+(const Size& other) const {
    return Size{pos_ + other.pos_};
  }
  Size& operator+=(const Size& other) {
    pos_ += other.pos_;
    return *this;
  }
  Size operator-(const Size& other) const {
    return Size{pos_ - other.pos_};
  }
  Size& operator-=(const Size& other) {
    pos_ += other.pos_;
    return *this;
  }

  uint64_t bit_count() const {
    return pos_;
  }

  uint8_t bit_offset() const {
    return pos_ % 8;
  }

  uint64_t byte_count() const {
    return pos_ / 8;
  }

  Size ClipToByte() const {
    return Size{pos_ - (pos_ % 8)};
  }

 private:
  explicit Size(uint64_t pos) : pos_(pos) {}

  uint64_t pos_;
};

inline std::ostream& operator<<(std::ostream& os, const Size& p) {
  return os << p.byte_count() << ":" << p.bit_offset();
}

inline bool operator==(const Size& a, const Size& b) {
  return a.bit_count() == b.bit_count();
}
inline bool operator!=(const Size& a, const Size& b) {
  return a.bit_count() != b.bit_count();
}
inline bool operator>(const Size& a, const Size& b) {
  return a.bit_count() > b.bit_count();
}
inline bool operator>=(const Size& a, const Size& b) {
  return a.bit_count() >= b.bit_count();
}
inline bool operator<(const Size& a, const Size& b) {
  return a.bit_count() < b.bit_count();
}
inline bool operator<=(const Size& a, const Size& b) {
  return a.bit_count() <= b.bit_count();
}

}  // namespace binary_reader

#endif  // BINARY_READER_UTIL_SIZE_H_
