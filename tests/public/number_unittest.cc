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

#include "binary_reader/number.h"

#include <limits>

#include "gtest_wrapper.h"

namespace binary_reader {

static_assert(std::is_constructible<Number, bool>::value,
              "Can construct with bool");
static_assert(std::is_constructible<Number, int>::value,
              "Can construct with int");
static_assert(!std::is_constructible<Number, int*>::value,
              "Cannot construct with pointer");

TEST(NumberTest, BasicFlow) {
  Number v{false};
  EXPECT_EQ(v.as_double(), 0.0);
  EXPECT_EQ(v.as_unsigned(), 0ull);
  EXPECT_EQ(v.as_signed(), 0ll);

  v = true;
  EXPECT_EQ(v.as_double(), 1.0);
  EXPECT_EQ(v.as_unsigned(), 1ull);
  EXPECT_EQ(v.as_signed(), 1ll);

  v = 15ull;
  EXPECT_EQ(v.as_double(), 15.0);
  EXPECT_EQ(v.as_unsigned(), 15ull);
  EXPECT_EQ(v.as_signed(), 15ll);

  v = 23ll;
  EXPECT_EQ(v.as_double(), 23.0);
  EXPECT_EQ(v.as_unsigned(), 23ull);
  EXPECT_EQ(v.as_signed(), 23ll);

  v = 4.9;
  EXPECT_EQ(v.as_double(), 4.9);
  EXPECT_EQ(v.as_unsigned(), 4ull);
  EXPECT_EQ(v.as_signed(), 4ll);
}

TEST(NumberTest, ClampsConversions) {
  Number v = Number{-12ll};
  EXPECT_EQ(v.as_double(), -12.0);
  EXPECT_EQ(v.as_unsigned(), 0ull);
  EXPECT_EQ(v.as_signed(), -12ll);

  v = Number{-6.0};
  EXPECT_EQ(v.as_double(), -6.0);
  EXPECT_EQ(v.as_unsigned(), 0ull);
  EXPECT_EQ(v.as_signed(), -6ll);

  v = Number{INFINITY};
  EXPECT_EQ(v.as_double(), INFINITY);
  EXPECT_EQ(v.as_unsigned(), std::numeric_limits<uint64_t>::max());
  EXPECT_EQ(v.as_signed(), std::numeric_limits<int64_t>::max());

  v = Number{-INFINITY};
  EXPECT_EQ(v.as_double(), -INFINITY);
  EXPECT_EQ(v.as_unsigned(), 0);
  EXPECT_EQ(v.as_signed(), std::numeric_limits<int64_t>::lowest());

  v = Number{std::numeric_limits<uint64_t>::max() - 12};
  EXPECT_EQ(v.as_double(), std::numeric_limits<uint64_t>::max() - 12);
  EXPECT_EQ(v.as_unsigned(), std::numeric_limits<uint64_t>::max() - 12);
  EXPECT_EQ(v.as_signed(), std::numeric_limits<int64_t>::max());

  v = Number{std::numeric_limits<int64_t>::lowest()};
  EXPECT_EQ(v.as_double(), std::numeric_limits<int64_t>::lowest());
  EXPECT_EQ(v.as_unsigned(), 0);
  EXPECT_EQ(v.as_signed(), std::numeric_limits<int64_t>::lowest());
}

TEST(NumberTest, Equals) {
  // Numbers are treated the same by value.
  EXPECT_TRUE(Number{12ull} == Number{12ll});
  EXPECT_TRUE(Number{12ull} == Number{12ull});
  EXPECT_TRUE(Number{12ull} == Number{12.0});
  EXPECT_TRUE(Number{12ll} == Number{12ll});
  EXPECT_TRUE(Number{12ll} == Number{12ull});
  EXPECT_TRUE(Number{12ll} == Number{12.0});
  EXPECT_TRUE(Number{12.0} == Number{12ll});
  EXPECT_TRUE(Number{12.0} == Number{12ull});
  EXPECT_TRUE(Number{12.0} == Number{12.0});
  EXPECT_FALSE(Number{12ull} == Number{10ll});
  EXPECT_FALSE(Number{12ull} == Number{10ull});
  EXPECT_FALSE(Number{12ull} == Number{10.0});
  EXPECT_FALSE(Number{12ll} == Number{10ll});
  EXPECT_FALSE(Number{12ll} == Number{10ull});
  EXPECT_FALSE(Number{12ll} == Number{10.0});
  EXPECT_FALSE(Number{12.0} == Number{10ll});
  EXPECT_FALSE(Number{12.0} == Number{10ull});
  EXPECT_FALSE(Number{12.0} == Number{10.0});

  EXPECT_TRUE(Number{-5ll} == Number{-5ll});
  EXPECT_FALSE(Number{10ull} == Number{-1ll});
  // The static cast will make it a large unsigned number, which should not be
  // the same as a negative number.
  EXPECT_FALSE(Number{static_cast<uint64_t>(-1)} == Number{-1ll});
  EXPECT_FALSE(Number{-1ll} == Number{static_cast<uint64_t>(-1)});

  EXPECT_TRUE(Number{true} == Number{true});
  EXPECT_TRUE(Number{false} == Number{false});

  EXPECT_FALSE(Number{true} == Number{false});
  EXPECT_FALSE(Number{true} == Number{12.0});
}

TEST(NumberTest, LessThan) {
  constexpr const uint64_t kNegativeTenUnsigned =
      std::numeric_limits<uint64_t>::max() - 10;

  EXPECT_TRUE(Number{8ull} < Number{45ull});
  EXPECT_TRUE(Number{8ull} < Number{45ll});
  EXPECT_FALSE(Number{8ull} < Number{-4ll});
  EXPECT_FALSE(Number{kNegativeTenUnsigned} < Number{-8ll});
  EXPECT_FALSE(Number{kNegativeTenUnsigned} < Number{-11ll});
  EXPECT_TRUE(Number{6ull} < Number{10.0});
  EXPECT_FALSE(Number{6ull} < Number{-3.0});

  EXPECT_TRUE(Number{8ll} < Number{10ull});
  EXPECT_TRUE(Number{-8ll} < Number{10ull});
  EXPECT_TRUE(Number{-8ll} < Number{kNegativeTenUnsigned});
  EXPECT_TRUE(Number{-11ll} < Number{kNegativeTenUnsigned});
  EXPECT_TRUE(Number{8ll} < Number{10ll});
  EXPECT_TRUE(Number{-8ll} < Number{10ll});
  EXPECT_FALSE(Number{8ll} < Number{-10ll});
  EXPECT_TRUE(Number{6ll} < Number{10.0});
  EXPECT_FALSE(Number{6ll} < Number{-3.0});

  EXPECT_TRUE(Number{6.0} < Number{8ull});
  EXPECT_FALSE(Number{6.0} < Number{3ull});
  EXPECT_TRUE(Number{6.0} < Number{8ll});
  EXPECT_FALSE(Number{6.0} < Number{3ll});
  EXPECT_TRUE(Number{6.0} < Number{8.0});
  EXPECT_FALSE(Number{6.0} < Number{3.0});
}

}  // namespace binary_reader
