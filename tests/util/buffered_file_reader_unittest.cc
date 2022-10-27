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

#include "util/buffered_file_reader.h"

#include "gtest_wrapper.h"
#include "mocks.h"

namespace binary_reader {

using testing::_;
using testing::InSequence;
using testing::Pointee;
using testing::Return;

TEST(BufferedFileReaderTest, BasicFlow) {
  const uint8_t expected[] = "data";
  auto mock = std::make_shared<MockFileReader>();
  mock->ExpectRead(expected);

  BufferedFileReader reader(mock);
  const uint8_t* actual;
  size_t actual_size;
  ErrorCollection errors;
  EXPECT_EQ(reader.position(), Size::FromBits(0));
  ASSERT_TRUE(reader.GetBuffer(&actual, &actual_size, &errors));
  ASSERT_EQ(actual_size, sizeof(expected));
  EXPECT_EQ(memcmp(expected, actual, sizeof(expected)), 0);
}

TEST(BufferedFileReaderTest, MultipleReads) {
  const uint8_t expected[] = "abcdefghij";
  auto mock = std::make_shared<MockFileReader>();
  InSequence seq;
  mock->ExpectReadSize(expected, 3);
  mock->ExpectReadSize(expected + 3, 4);
  mock->ExpectReadSize(expected + 7, sizeof(expected) - 7);
  mock->ExpectReadEof();

  BufferedFileReader reader(mock);
  const uint8_t* actual;
  size_t actual_size;
  ErrorCollection errors;
  ASSERT_TRUE(reader.EnsureBuffer(Size::FromBytes(20), &errors));
  ASSERT_TRUE(reader.GetBuffer(&actual, &actual_size, &errors));
  ASSERT_EQ(actual_size, sizeof(expected));
  EXPECT_EQ(memcmp(expected, actual, sizeof(expected)), 0);
}

TEST(BufferedFileReaderTest, BufferedSeek) {
  const uint8_t expected[] = "abcdefghij";
  constexpr const size_t kSeekPos = 4;
  auto mock = std::make_shared<MockFileReader>();
  mock->ExpectRead(expected);
  EXPECT_CALL(*mock, Seek(_, _)).Times(0);

  BufferedFileReader reader(mock);
  const uint8_t* actual;
  size_t actual_size;
  ErrorCollection errors;
  ASSERT_TRUE(reader.EnsureBuffer(Size::FromBytes(1), &errors));
  ASSERT_TRUE(reader.Seek(Size::FromBytes(kSeekPos), &errors));
  ASSERT_TRUE(reader.GetBuffer(&actual, &actual_size, &errors));
  ASSERT_EQ(reader.position().byte_count(), kSeekPos);
  ASSERT_EQ(actual_size, sizeof(expected) - kSeekPos);
  EXPECT_EQ(memcmp(expected + kSeekPos, actual, sizeof(expected) - kSeekPos),
            0);
}

TEST(BufferedFileReaderTest, UnbufferedSeek) {
  const uint8_t first_buffer[] = "abc";
  const uint8_t second_buffer[] = "def";
  constexpr const size_t kSeekPos = 50;
  auto mock = std::make_shared<MockFileReader>();
  InSequence seq;
  mock->ExpectRead(first_buffer);
  EXPECT_CALL(*mock, Seek(Pointee(kSeekPos), _)).WillOnce(Return(true));
  mock->ExpectRead(second_buffer);

  BufferedFileReader reader(mock);
  const uint8_t* actual;
  size_t actual_size;
  ErrorCollection errors;
  ASSERT_TRUE(reader.EnsureBuffer(Size::FromBytes(1), &errors));
  ASSERT_TRUE(reader.Seek(Size::FromBytes(kSeekPos), &errors));
  ASSERT_TRUE(reader.GetBuffer(&actual, &actual_size, &errors));
  ASSERT_EQ(reader.position().byte_count(), kSeekPos);
  ASSERT_EQ(actual_size, sizeof(second_buffer));
  EXPECT_EQ(memcmp(second_buffer, actual, sizeof(second_buffer)), 0);
}

}  // namespace binary_reader
