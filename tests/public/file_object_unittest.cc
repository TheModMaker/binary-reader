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

#include "binary_reader/file_object.h"

#include <gtest/gtest.h>

#include "public/file_object_init.h"

namespace binary_reader {

class FileObjectTest : public testing::Test {
 protected:
  std::shared_ptr<FileObject> MakeObject(
      const std::vector<std::pair<std::string, Value>>& fields) {
    FileObjectInit init;
    init.fields = fields;
    return MakeFileObject(init);
  }
};

TEST_F(FileObjectTest, BasicFlow) {
  auto obj = MakeObject({{"foo", Value{1}}});
  ASSERT_TRUE(obj);
  EXPECT_TRUE(obj->HasField("foo"));
  EXPECT_FALSE(obj->HasField("bar"));
  EXPECT_EQ(obj->GetFieldValue("foo"), Value{1});
  EXPECT_EQ(obj->GetFieldValue("bar"), Value{});

  EXPECT_NE(obj->begin(), obj->end());
  size_t i = 0;
  for (auto it = obj->begin(); it != obj->end(); it++, i++) {
    ASSERT_EQ(i, 0u);
    ASSERT_EQ(it->first, "foo");
    ASSERT_EQ(it->second, Value{1});
  }

  auto it2 = obj->find("foo");
  ASSERT_NE(it2, obj->end());
  EXPECT_EQ(it2->first, "foo");
  EXPECT_EQ(it2->second, Value{1});
}

}  // namespace binary_reader
