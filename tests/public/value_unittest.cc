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

#include "binary_reader/value.h"

#include <limits>

#include "gtest_wrapper.h"

namespace binary_reader {

static_assert(std::is_constructible<Value, bool>::value,
              "Can construct with bool");
static_assert(std::is_constructible<Value, int>::value,
              "Can construct with int");
static_assert(!std::is_constructible<Value, int*>::value,
              "Cannot construct with pointer");

TEST(ValueTest, BasicFlow) {
  Value v{nullptr};
  ASSERT_EQ(v.value_type(), ValueType::Null);
  EXPECT_TRUE(v.is_null());
  EXPECT_FALSE(v.is_number());
  EXPECT_FALSE(v.as_bool());
  EXPECT_EQ(v.as_double(), 0.0);
  EXPECT_EQ(v.as_unsigned(), 0ull);
  EXPECT_EQ(v.as_signed(), 0ll);
  EXPECT_EQ(v.as_object(), nullptr);
  EXPECT_EQ(v.as_string().AsUtf8(), "null");

  v = false;
  ASSERT_EQ(v.value_type(), ValueType::Boolean);
  EXPECT_FALSE(v.is_null());
  EXPECT_FALSE(v.is_number());
  EXPECT_FALSE(v.as_bool());
  EXPECT_EQ(v.as_double(), 0.0);
  EXPECT_EQ(v.as_unsigned(), 0ull);
  EXPECT_EQ(v.as_signed(), 0ll);
  EXPECT_EQ(v.as_object(), nullptr);
  EXPECT_EQ(v.as_string().AsUtf8(), "false");

  v = true;
  ASSERT_EQ(v.value_type(), ValueType::Boolean);
  EXPECT_FALSE(v.is_null());
  EXPECT_FALSE(v.is_number());
  EXPECT_TRUE(v.as_bool());
  EXPECT_EQ(v.as_double(), 1.0);
  EXPECT_EQ(v.as_unsigned(), 1ull);
  EXPECT_EQ(v.as_signed(), 1ll);
  EXPECT_EQ(v.as_object(), nullptr);
  EXPECT_EQ(v.as_string().AsUtf8(), "true");

  v = 15ull;
  ASSERT_EQ(v.value_type(), ValueType::UnsignedInt);
  EXPECT_FALSE(v.is_null());
  EXPECT_TRUE(v.is_number());
  EXPECT_TRUE(v.as_bool());
  EXPECT_EQ(v.as_double(), 15.0);
  EXPECT_EQ(v.as_unsigned(), 15ull);
  EXPECT_EQ(v.as_signed(), 15ll);
  EXPECT_EQ(v.as_object(), nullptr);
  EXPECT_EQ(v.as_string().AsUtf8(), "15");

  v = 23ll;
  ASSERT_EQ(v.value_type(), ValueType::SignedInt);
  EXPECT_FALSE(v.is_null());
  EXPECT_TRUE(v.is_number());
  EXPECT_TRUE(v.as_bool());
  EXPECT_EQ(v.as_double(), 23.0);
  EXPECT_EQ(v.as_unsigned(), 23ull);
  EXPECT_EQ(v.as_signed(), 23ll);
  EXPECT_EQ(v.as_object(), nullptr);
  EXPECT_EQ(v.as_string().AsUtf8(), "23");

  v = 4.9;
  ASSERT_EQ(v.value_type(), ValueType::Double);
  EXPECT_FALSE(v.is_null());
  EXPECT_TRUE(v.is_number());
  EXPECT_TRUE(v.as_bool());
  EXPECT_EQ(v.as_double(), 4.9);
  EXPECT_EQ(v.as_unsigned(), 4ull);
  EXPECT_EQ(v.as_signed(), 4ll);
  EXPECT_EQ(v.as_object(), nullptr);
  // Don't test as_string() for double since it can have unspecified output.

  v = UtfString::FromUtf8("foo");
  ASSERT_EQ(v.value_type(), ValueType::String);
  EXPECT_FALSE(v.is_null());
  EXPECT_FALSE(v.is_number());
  EXPECT_TRUE(v.as_bool());
  EXPECT_EQ(v.as_double(), 0);
  EXPECT_EQ(v.as_unsigned(), 0ull);
  EXPECT_EQ(v.as_signed(), 0ll);
  EXPECT_EQ(v.as_object(), nullptr);
  EXPECT_EQ(v.as_string().AsUtf8(), "foo");
}

TEST(ValueTest, ClampsConversions) {
  Value v = Value{-12ll};
  EXPECT_EQ(v.as_double(), -12.0);
  EXPECT_EQ(v.as_unsigned(), 0ull);
  EXPECT_EQ(v.as_signed(), -12ll);

  v = Value{-6.0};
  EXPECT_EQ(v.as_double(), -6.0);
  EXPECT_EQ(v.as_unsigned(), 0ull);
  EXPECT_EQ(v.as_signed(), -6ll);

  v = Value{INFINITY};
  EXPECT_EQ(v.as_double(), INFINITY);
  EXPECT_EQ(v.as_unsigned(), std::numeric_limits<uint64_t>::max());
  EXPECT_EQ(v.as_signed(), std::numeric_limits<int64_t>::max());

  v = Value{-INFINITY};
  EXPECT_EQ(v.as_double(), -INFINITY);
  EXPECT_EQ(v.as_unsigned(), 0);
  EXPECT_EQ(v.as_signed(), std::numeric_limits<int64_t>::lowest());

  v = Value{std::numeric_limits<uint64_t>::max() - 12};
  EXPECT_EQ(v.as_double(), std::numeric_limits<uint64_t>::max() - 12);
  EXPECT_EQ(v.as_unsigned(), std::numeric_limits<uint64_t>::max() - 12);
  EXPECT_EQ(v.as_signed(), std::numeric_limits<int64_t>::max());

  v = Value{std::numeric_limits<int64_t>::lowest()};
  EXPECT_EQ(v.as_double(), std::numeric_limits<int64_t>::lowest());
  EXPECT_EQ(v.as_unsigned(), 0);
  EXPECT_EQ(v.as_signed(), std::numeric_limits<int64_t>::lowest());
}

TEST(ValueTest, Equals) {
  // Numbers are treated the same by value.
  EXPECT_TRUE(Value{12ull} == Value{12ll});
  EXPECT_TRUE(Value{12ull} == Value{12ull});
  EXPECT_TRUE(Value{12ull} == Value{12.0});
  EXPECT_TRUE(Value{12ll} == Value{12ll});
  EXPECT_TRUE(Value{12ll} == Value{12ull});
  EXPECT_TRUE(Value{12ll} == Value{12.0});
  EXPECT_TRUE(Value{12.0} == Value{12ll});
  EXPECT_TRUE(Value{12.0} == Value{12ull});
  EXPECT_TRUE(Value{12.0} == Value{12.0});
  EXPECT_FALSE(Value{12ull} == Value{10ll});
  EXPECT_FALSE(Value{12ull} == Value{10ull});
  EXPECT_FALSE(Value{12ull} == Value{10.0});
  EXPECT_FALSE(Value{12ll} == Value{10ll});
  EXPECT_FALSE(Value{12ll} == Value{10ull});
  EXPECT_FALSE(Value{12ll} == Value{10.0});
  EXPECT_FALSE(Value{12.0} == Value{10ll});
  EXPECT_FALSE(Value{12.0} == Value{10ull});
  EXPECT_FALSE(Value{12.0} == Value{10.0});

  EXPECT_TRUE(Value{-5ll} == Value{-5ll});
  EXPECT_FALSE(Value{10ull} == Value{-1ll});
  // The static cast will make it a large unsigned number, which should not be
  // the same as a negative number.
  EXPECT_FALSE(Value{static_cast<uint64_t>(-1)} == Value{-1ll});
  EXPECT_FALSE(Value{-1ll} == Value{static_cast<uint64_t>(-1)});

  EXPECT_TRUE(Value{nullptr} == Value{nullptr});
  EXPECT_TRUE(Value{true} == Value{true});
  EXPECT_TRUE(Value{false} == Value{false});

  EXPECT_FALSE(Value{true} == Value{false});
  EXPECT_FALSE(Value{nullptr} == Value{false});
  EXPECT_FALSE(Value{true} == Value{12.0});

  EXPECT_TRUE(Value{UtfString::FromUtf8("foo")} ==
              Value{UtfString::FromUtf8("foo")});
  EXPECT_FALSE(Value{UtfString::FromUtf8("foo")} ==
               Value{UtfString::FromUtf8("bar")});
  EXPECT_FALSE(Value{UtfString::FromUtf8("true")} == Value{true});
  EXPECT_FALSE(Value{UtfString::FromUtf8("true")} == Value{1});
  EXPECT_FALSE(Value{UtfString::FromUtf8("1")} == Value{1});
}

TEST(ValueTest, LessThan) {
  constexpr const uint64_t kNegativeTenUnsigned =
      std::numeric_limits<uint64_t>::max() - 10;

  EXPECT_TRUE(Value{nullptr} < Value{false});
  EXPECT_TRUE(Value{false} < Value{true});
  EXPECT_TRUE(Value{true} < Value{9.0});
  EXPECT_TRUE(Value{2.0} < Value{3.0});
  EXPECT_TRUE(Value{1.0} < Value{UtfString::FromUtf8("foo")});
  EXPECT_FALSE(Value{false} < Value{nullptr});
  EXPECT_FALSE(Value{false} < Value{false});
  EXPECT_FALSE(Value{1.0} < Value{false});
  EXPECT_FALSE(Value{UtfString::FromUtf8("foo")} < Value{1.0});

  EXPECT_TRUE(Value{UtfString::FromUtf8("a")} <
              Value{UtfString::FromUtf8("b")});
  EXPECT_FALSE(Value{UtfString::FromUtf8("b")} <
               Value{UtfString::FromUtf8("a")});

  EXPECT_TRUE(Value{8ull} < Value{45ull});
  EXPECT_TRUE(Value{8ull} < Value{45ll});
  EXPECT_FALSE(Value{8ull} < Value{-4ll});
  EXPECT_FALSE(Value{kNegativeTenUnsigned} < Value{-8ll});
  EXPECT_FALSE(Value{kNegativeTenUnsigned} < Value{-11ll});
  EXPECT_TRUE(Value{6ull} < Value{10.0});
  EXPECT_FALSE(Value{6ull} < Value{-3.0});

  EXPECT_TRUE(Value{8ll} < Value{10ull});
  EXPECT_TRUE(Value{-8ll} < Value{10ull});
  EXPECT_TRUE(Value{-8ll} < Value{kNegativeTenUnsigned});
  EXPECT_TRUE(Value{-11ll} < Value{kNegativeTenUnsigned});
  EXPECT_TRUE(Value{8ll} < Value{10ll});
  EXPECT_TRUE(Value{-8ll} < Value{10ll});
  EXPECT_FALSE(Value{8ll} < Value{-10ll});
  EXPECT_TRUE(Value{6ll} < Value{10.0});
  EXPECT_FALSE(Value{6ll} < Value{-3.0});

  EXPECT_TRUE(Value{6.0} < Value{8ull});
  EXPECT_FALSE(Value{6.0} < Value{3ull});
  EXPECT_TRUE(Value{6.0} < Value{8ll});
  EXPECT_FALSE(Value{6.0} < Value{3ll});
  EXPECT_TRUE(Value{6.0} < Value{8.0});
  EXPECT_FALSE(Value{6.0} < Value{3.0});
}

}  // namespace binary_reader
