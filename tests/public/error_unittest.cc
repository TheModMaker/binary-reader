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

#include "binary_reader/error.h"

#include "gtest_wrapper.h"

namespace binary_reader {

TEST(DefaultErrorMessageTest, ReplacesArguments) {
  EXPECT_EQ(DefaultErrorMessage(ErrorKind::Unknown, {}), "Unknown error");
  EXPECT_EQ(DefaultErrorMessage(ErrorKind::CannotOpen, {"file"}),
            "Cannot open file 'file'");
  EXPECT_EQ(DefaultErrorMessage(ErrorKind::UnknownOptionValueTyped, {"a", "b"}),
            "Unknown option value 'a' for option 'b'");
}

TEST(DefaultErrorMessageTest, LessArguments) {
  EXPECT_EQ(DefaultErrorMessage(ErrorKind::CannotOpen, {}),
            "Cannot open file ''");
  EXPECT_EQ(DefaultErrorMessage(ErrorKind::UnknownOptionValueTyped, {"a"}),
            "Unknown option value 'a' for option ''");
  EXPECT_EQ(DefaultErrorMessage(ErrorKind::UnknownOptionValueTyped, {}),
            "Unknown option value '' for option ''");
}

TEST(DefaultErrorMessageTest, ExtraArguments) {
  EXPECT_EQ(DefaultErrorMessage(ErrorKind::Unknown, {"a"}), "Unknown error");
  EXPECT_EQ(DefaultErrorMessage(ErrorKind::Unknown, {"a", "b"}),
            "Unknown error");
  EXPECT_EQ(DefaultErrorMessage(ErrorKind::CannotOpen, {"file", "a", "b"}),
            "Cannot open file 'file'");
}

}  // namespace binary_reader
