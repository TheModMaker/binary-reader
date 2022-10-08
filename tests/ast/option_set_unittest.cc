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

#include "ast/option_set.h"

#include "gtest_wrapper.h"

namespace binary_reader {

TEST(OptionSetTest, AddStatic_Duplicates) {
  OptionSet set;
  ErrorCollection error;
  ASSERT_TRUE(
      set.AddStatic({}, OptionType::Signedness, UtfString(u"a"), &error));
  ASSERT_FALSE(
      set.AddStatic({}, OptionType::Signedness, UtfString(u"c"), &error));
}

TEST(OptionSetTest, ParsesValues) {
  OptionSet set;
  ErrorCollection error;
  ASSERT_TRUE(
      set.AddStatic({}, OptionType::Unknown, UtfString(u"signed"), &error));
  ASSERT_TRUE(
      set.AddStatic({}, OptionType::Unknown, UtfString(u"big"), &error));
  Options opts;
  ASSERT_TRUE(set.BuildStaticOptions({}, &opts, &error));
  EXPECT_EQ(opts.signedness, Signedness::Signed);
  EXPECT_EQ(opts.byte_order, ByteOrder::BigEndian);
}

TEST(OptionSetTest, DuplicatesReplace) {
  OptionSet set;
  ErrorCollection error;
  ASSERT_TRUE(
      set.AddStatic({}, OptionType::Unknown, UtfString(u"signed"), &error));
  ASSERT_TRUE(
      set.AddStatic({}, OptionType::Unknown, UtfString(u"unsigned"), &error));
  Options opts;
  ASSERT_TRUE(set.BuildStaticOptions({}, &opts, &error));
  EXPECT_EQ(opts.signedness, Signedness::Unsigned);
}

TEST(OptionSetTest, ChecksValidStatic) {
  OptionSet set;
  ErrorCollection error;
  ASSERT_TRUE(
      set.AddStatic({}, OptionType::Signedness, UtfString(u"signed"), &error));
  Options opts;
  ASSERT_FALSE(set.BuildStaticOptions({OptionType::ByteOrder}, &opts, &error));
}

TEST(OptionSetTest, ChecksValidDynamic) {
  OptionSet set;
  ErrorCollection error;
  ASSERT_TRUE(
      set.AddStatic({}, OptionType::Unknown, UtfString(u"signed"), &error));
  Options opts;
  ASSERT_FALSE(set.BuildStaticOptions({OptionType::ByteOrder}, &opts, &error));
}

TEST(OptionSetTest, UnknownStatic) {
  OptionSet set;
  ErrorCollection error;
  ASSERT_TRUE(
      set.AddStatic({}, OptionType::Signedness, UtfString(u"big"), &error));
  Options opts;
  ASSERT_FALSE(set.BuildStaticOptions({}, &opts, &error));
}

TEST(OptionSetTest, UnknownDynamic) {
  OptionSet set;
  ErrorCollection error;
  ASSERT_TRUE(
      set.AddStatic({}, OptionType::Unknown, UtfString(u"cat"), &error));
  Options opts;
  ASSERT_FALSE(set.BuildStaticOptions({}, &opts, &error));
}

}  // namespace binary_reader
