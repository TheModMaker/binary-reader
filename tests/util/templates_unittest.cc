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

#include "util/templates.h"

#include "gtest_wrapper.h"

namespace binary_reader {

namespace {

template <typename T>
T max() {
  return std::numeric_limits<T>::max();
}

template <typename T>
T min() {
  // See comment in clamp_cast
  return std::numeric_limits<T>::lowest();
}

}  // namespace

TEST(TemplatesTest, ClampCast) {
  EXPECT_EQ(clamp_cast<uint32_t>(24), 24u);
  EXPECT_EQ(clamp_cast<uint32_t>(-24), 0u);
  EXPECT_EQ(clamp_cast<uint32_t>(min<int64_t>()), 0u);
  EXPECT_EQ(clamp_cast<int32_t>(max<uint64_t>()), max<int32_t>());

  EXPECT_EQ(clamp_cast<int32_t>(max<double>()), max<int32_t>());
  EXPECT_EQ(clamp_cast<int32_t>(min<double>()), min<int32_t>());
  EXPECT_EQ(clamp_cast<uint32_t>(min<double>()), 0u);
  EXPECT_EQ(clamp_cast<uint64_t>(INFINITY), max<uint64_t>());
  EXPECT_EQ(clamp_cast<int32_t>(INFINITY), max<int32_t>());
  EXPECT_EQ(clamp_cast<int32_t>(-INFINITY), min<int32_t>());
  EXPECT_EQ(clamp_cast<uint32_t>(-INFINITY), 0u);

  EXPECT_EQ(clamp_cast<double>(12), 12.0);
  EXPECT_EQ(clamp_cast<double>(max<uint64_t>()), max<uint64_t>());
  EXPECT_EQ(clamp_cast<double>(min<int64_t>()), min<int64_t>());
  EXPECT_EQ(clamp_cast<double>(max<double>()), max<double>());
  EXPECT_EQ(clamp_cast<double>(INFINITY), INFINITY);
}

}  // namespace binary_reader
