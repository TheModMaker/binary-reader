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

#include "binary_reader/options.h"

#include "gtest_wrapper.h"

namespace binary_reader {

namespace {

Value MakeVal(const char* str) {
  return Value{UtfString::FromUtf8(str)};
}

}  // namespace

class OptionsTest : public testing::Test {
 public:
  void CheckOptionData() {
    ASSERT_TRUE(Options::CheckOptionData());
  }
};

TEST_F(OptionsTest, CheckOptionData) {
  CheckOptionData();
}

TEST_F(OptionsTest, ParseOption_Success) {
  OptionType type;
  std::any result;
  ASSERT_EQ(Options::ParseOption({}, MakeVal("signed"), &type, &result),
            Options::ParseResult::Success);
  EXPECT_EQ(type, OptionType::Signedness);
  EXPECT_EQ(std::any_cast<Signedness>(result), Signedness::Signed);

  ASSERT_EQ(Options::ParseOption({}, MakeVal("network"), &type, &result),
            Options::ParseResult::Success);
  EXPECT_EQ(type, OptionType::ByteOrder);
  EXPECT_EQ(std::any_cast<ByteOrder>(result), ByteOrder::BigEndian);
}

TEST_F(OptionsTest, ParseOption_Filter) {
  OptionType type;
  std::any result;
  ASSERT_EQ(Options::ParseOption({OptionType::Signedness}, MakeVal("signed"),
                                 &type, &result),
            Options::ParseResult::Success);
  EXPECT_EQ(type, OptionType::Signedness);
  EXPECT_EQ(std::any_cast<Signedness>(result), Signedness::Signed);

  ASSERT_EQ(Options::ParseOption({OptionType::Signedness}, MakeVal("network"),
                                 &type, &result),
            Options::ParseResult::UnknownString);
}

TEST_F(OptionsTest, ParseOption_BoolFail) {
  OptionType type;
  std::any result;
  ASSERT_EQ(Options::ParseOption({OptionType::ByteOrder}, Value{true}, &type,
                                 &result),
            Options::ParseResult::InvalidValueType);
  ASSERT_EQ(Options::ParseOption({OptionType::ByteOrder}, Value{false}, &type,
                                 &result),
            Options::ParseResult::InvalidValueType);
}

}  // namespace binary_reader
