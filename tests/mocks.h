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

#ifndef BINARY_READER_TESTS_MOCKS_H_
#define BINARY_READER_TESTS_MOCKS_H_

#include <vector>

#include "binary_reader/file_system.h"
#include "gtest_wrapper.h"

namespace binary_reader {

constexpr const bool kExpectExactRead = true;

class MockFileReader : public FileReader {
 public:
  MockFileReader() {}

  explicit MockFileReader(std::initializer_list<uint8_t> buffer)
      : first_read_(buffer) {
    ExpectReadSize(first_read_.data(), first_read_.size());
  }

  MOCK_METHOD(bool, can_seek, (), (const, override));
  MOCK_METHOD(uint64_t, position, (), (const, override));
  MOCK_METHOD(std::optional<uint64_t>, size, (), (const, override));
  MOCK_METHOD(bool, Read, (uint8_t*, size_t*, ErrorCollection*), (override));
  MOCK_METHOD(bool, Seek, (uint64_t*, ErrorCollection*), (override));

  template <size_t N>
  void ExpectRead(const uint8_t (&buffer)[N], bool exact_size = false) {
    EXPECT_CALL(*this, Read(testing::_,
                            testing::Pointee(testing::Conditional(
                                exact_size, testing::Eq(N), testing::Ge(N))),
                            testing::_))
        .WillOnce(testing::DoAll(
            testing::SetArrayArgument<0>(buffer, buffer + N),
            testing::SetArgPointee<1>(N), testing::Return(true)));
  }

  void ExpectReadSize(const uint8_t* buffer, size_t size,
                      bool exact_size = false) {
    EXPECT_CALL(*this,
                Read(testing::_,
                     testing::Pointee(testing::Conditional(
                         exact_size, testing::Eq(size), testing::Ge(size))),
                     testing::_))
        .WillOnce(testing::DoAll(
            testing::SetArrayArgument<0>(buffer, buffer + size),
            testing::SetArgPointee<1>(size), testing::Return(true)));
  }

  void ExpectReadEof() {
    EXPECT_CALL(*this, Read(testing::_, testing::_, testing::_))
        .WillOnce(testing::DoAll(testing::SetArgPointee<1>(0),
                                 testing::Return(true)));
  }

  const std::vector<uint8_t> first_read_;
};

}  // namespace binary_reader

#endif  // BINARY_READER_TESTS_MOCKS_H_
