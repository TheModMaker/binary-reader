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

#include "binary_reader/codecs.h"

#include "gtest_wrapper.h"
#include "util/templates.h"

namespace binary_reader {

namespace {

std::shared_ptr<TextConverter> MakeUtf8() {
  return CodecCollection::CreateDefaultCollection()
      ->GetCodec("utf8")
      ->CreateCoder();
}

}  // namespace

TEST(Utf8CodecsTest, Decode) {
  const uint8_t bytes[] = {
      'a', 0xc2, 0xa3, 'b', 0xe2, 0x82, 0xac, 'c', 0xf0, 0x90, 0x90, 0xb7,
  };
  const uint16_t expected[] = {
      'a', 0xa3, 'b', 0x20ac, 'c', 0xd801, 0xdc37,
  };

  ErrorInfo err;
  std::u16string actual;
  auto conv = MakeUtf8();
  ASSERT_EQ(conv->Decode(bytes, sizeof(bytes), &actual, &err),
            TextConverter::Status::Success);
  EXPECT_EQ(actual, std::u16string(expected, expected + arraysize(expected)));
}

TEST(Utf8CodecsTest, Decode_Partial) {
  const uint8_t bytes1[] = {'a', 0xc2};
  const uint8_t bytes2[] = {0xa3};
  const uint16_t expected[] = {'a', 0xa3};

  ErrorInfo err;
  std::u16string actual;
  auto conv = MakeUtf8();
  ASSERT_EQ(conv->Decode(bytes1, sizeof(bytes1), &actual, &err),
            TextConverter::Status::Success);
  EXPECT_EQ(actual, u"a");
  ASSERT_EQ(conv->Decode(bytes2, sizeof(bytes2), &actual, &err),
            TextConverter::Status::Success);
  EXPECT_EQ(actual, std::u16string(expected, expected + arraysize(expected)));
}

TEST(Utf8CodecsTest, Decode_InvalidFirst) {
  const uint8_t bytes[] = {'a', 0x80};

  ErrorInfo err;
  std::u16string actual;
  auto conv = MakeUtf8();
  ASSERT_EQ(conv->Decode(bytes, sizeof(bytes), &actual, &err),
            TextConverter::Status::Error);
}

TEST(Utf8CodecsTest, Decode_InvalidContinue) {
  const uint8_t bytes[] = {'a', 0xc2, 0xc2, 0xa3};

  ErrorInfo err;
  std::u16string actual;
  auto conv = MakeUtf8();
  ASSERT_EQ(conv->Decode(bytes, sizeof(bytes), &actual, &err),
            TextConverter::Status::Error);
}

TEST(Utf8CodecsTest, Encode) {
  const char16_t chars[] = {
      'a', 0xa3, 'b', 0x20ac, 'c', 0xd801, 0xdc37,
  };
  const uint8_t expected[] = {
      'a', 0xc2, 0xa3, 'b', 0xe2, 0x82, 0xac, 'c', 0xf0, 0x90, 0x90, 0xb7,
  };

  ErrorInfo err;
  std::vector<uint8_t> actual;
  auto conv = MakeUtf8();
  ASSERT_EQ(conv->Encode(chars, arraysize(chars), &actual, &err),
            TextConverter::Status::Success);
  EXPECT_EQ(actual,
            std::vector<uint8_t>(expected, expected + arraysize(expected)));
}

}  // namespace binary_reader
