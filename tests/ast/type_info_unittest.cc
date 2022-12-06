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

#include "ast/type_info.h"

#include <memory>

#include "gtest_wrapper.h"
#include "mocks.h"

namespace binary_reader {

namespace {

std::shared_ptr<BufferedFileReader> MakeReader(
    std::initializer_list<uint8_t> buffer, uint64_t bits = 0,
    bool eof = false) {
  std::shared_ptr<MockFileReader> file;
  {
    testing::InSequence seq;
    file = std::make_shared<MockFileReader>(buffer);
    if (eof)
      file->ExpectReadEof();
  }
  EXPECT_CALL(*file, Seek(testing::Pointee(bits / 8), testing::_))
      .WillOnce(testing::Return(true));

  auto ret = std::make_shared<BufferedFileReader>(file);
  EXPECT_TRUE(ret->Seek(Size::FromBits(bits), nullptr));
  return ret;
}

}  // namespace

TEST(IntegerTypeInfoTest, ReadValue_UnsignedInt) {
  Value result;
  ErrorCollection errors;
  IntegerTypeInfo integer({}, "", Size::FromBits(32), Signedness::Unsigned,
                          ByteOrder::BigEndian);
  ASSERT_TRUE(integer.ReadValue(
      MakeReader({0x11, 0x22, 0x33, 0x44, 0x55, 0x66}), &result, &errors));
  ASSERT_EQ(result, Value{0x11223344u});
}

TEST(IntegerTypeInfoTest, ReadValue_LargeUnaligned) {
  constexpr const uint8_t kOffset = 4;
  Value result;
  ErrorCollection errors;
  IntegerTypeInfo int32({}, "", Size::FromBits(32), Signedness::Unsigned,
                        ByteOrder::BigEndian);
  ASSERT_TRUE(
      int32.ReadValue(MakeReader({0x12, 0x34, 0x56, 0x78, 0x90, 0x12}, kOffset),
                      &result, &errors));
  ASSERT_EQ(result, Value{0x23456789u});

  IntegerTypeInfo int64({}, "", Size::FromBits(64), Signedness::Unsigned,
                        ByteOrder::BigEndian);
  ASSERT_TRUE(int64.ReadValue(
      MakeReader({0x12, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x45},
                 kOffset),
      &result, &errors));
  ASSERT_EQ(result, Value{0x2333333333333334ull});
}

TEST(IntegerTypeInfoTest, ReadValue_SmallUnaligned) {
  constexpr const uint8_t kOffset = 1;
  Value result;
  ErrorCollection errors;
  IntegerTypeInfo integer({}, "", Size::FromBits(5), Signedness::Unsigned,
                          ByteOrder::BigEndian);
  // 0110 1011
  ASSERT_TRUE(integer.ReadValue(MakeReader({0x6b}, kOffset), &result, &errors));
  ASSERT_EQ(result, Value{0x1a});  // 110 10
}

TEST(IntegerTypeInfoTest, ReadValue_SignedInt) {
  Value result;
  ErrorCollection errors;
  IntegerTypeInfo integer({}, "", Size::FromBits(32), Signedness::Signed,
                          ByteOrder::BigEndian);
  ASSERT_TRUE(integer.ReadValue(
      MakeReader({0x11, 0x22, 0x33, 0x44, 0x55, 0x66}), &result, &errors));
  ASSERT_EQ(result, Value{0x11223344});
}

TEST(IntegerTypeInfoTest, ReadValue_Negative) {
  Value result;
  ErrorCollection errors;
  IntegerTypeInfo int16({}, "", Size::FromBits(16), Signedness::Signed,
                        ByteOrder::BigEndian);
  ASSERT_TRUE(int16.ReadValue(MakeReader({0xff, 0xcd, 0x0}), &result, &errors));
  ASSERT_EQ(result, Value{-51});
  IntegerTypeInfo int64({}, "", Size::FromBits(64), Signedness::Signed,
                        ByteOrder::BigEndian);
  ASSERT_TRUE(int64.ReadValue(
      MakeReader({0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xcd, 0x0}),
      &result, &errors));
  ASSERT_EQ(result, Value{-51});
  IntegerTypeInfo int_little({}, "", Size::FromBits(16), Signedness::Signed,
                             ByteOrder::LittleEndian);
  ASSERT_TRUE(
      int_little.ReadValue(MakeReader({0xcd, 0xff, 0x0}), &result, &errors));
  ASSERT_EQ(result, Value{-51});
}

TEST(IntegerTypeInfoTest, ReadValue_Eof) {
  Value result;
  ErrorCollection errors;
  IntegerTypeInfo integer({}, "", Size::FromBits(32), Signedness::Unsigned,
                          ByteOrder::BigEndian);
  ASSERT_FALSE(integer.ReadValue(
      MakeReader({0x11, 0x22}, /* bits= */ 0, /* eof= */ true), &result,
      &errors));
}

TEST(IntegerTypeInfoTest, ReadValue_EofUnaligned) {
  constexpr const uint8_t kOffset = 3;
  Value result;
  ErrorCollection errors;
  IntegerTypeInfo integer({}, "", Size::FromBits(32), Signedness::Unsigned,
                          ByteOrder::BigEndian);
  ASSERT_FALSE(integer.ReadValue(
      MakeReader({0x11, 0x22, 0x33, 0x44}, kOffset, /* eof= */ true), &result,
      &errors));
}

}  // namespace binary_reader
