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

#include "ast/field_info.h"
#include "gtest_wrapper.h"
#include "mocks.h"
#include "public/file_object_init.h"

namespace binary_reader {

namespace {

std::shared_ptr<IntegerTypeInfo> MakeInt(size_t bits) {
  return std::make_shared<IntegerTypeInfo>(
      DebugInfo{}, "", Size::FromBits(bits), Signedness::Unsigned,
      ByteOrder::BigEndian);
}

}  // namespace

class FileObjectTest : public testing::Test {
 protected:
  std::shared_ptr<FileObject> MakeObjectFromFields(
      const std::vector<std::pair<std::string, Value>>& fields) {
    FileObjectInit init;
    init.test_fields = fields;
    return MakeFileObject(init);
  }

  std::shared_ptr<FileObject> MakeObjectFromFile(
      std::shared_ptr<TypeDefinition> type,
      std::initializer_list<uint8_t> data) {
    auto file = std::make_shared<MockFileReader>(data);
    EXPECT_CALL(*file, Seek(testing::Pointee(0), testing::_))
        .WillOnce(testing::Return(true));
    EXPECT_CALL(*file, Seek(testing::Pointee(data.size()), testing::_))
        .WillOnce(testing::Return(true));

    FileObjectInit init;
    init.file = std::make_shared<BufferedFileReader>(file);
    init.type = type;
    init.start_position = Size{};

    auto ret = MakeFileObject(init);
    ErrorCollection errors;
    if (ret && !ret->ReparseObject(&errors))
      ret.reset();
    return ret;
  }
};

TEST_F(FileObjectTest, BasicFlow_TestMode) {
  auto obj = MakeObjectFromFields({{"foo", Value{1}}});
  ASSERT_TRUE(obj);
  EXPECT_TRUE(obj->HasField("foo"));
  EXPECT_FALSE(obj->HasField("bar"));
  EXPECT_EQ(obj->GetFieldValue("foo"), Value{1});
  EXPECT_EQ(obj->GetFieldValue("bar"), Value{});
}

TEST_F(FileObjectTest, BasicFlow_NormalMode) {
  auto def = std::make_shared<TypeDefinition>(
      DebugInfo{}, "", std::vector<std::shared_ptr<Statement>>{
              std::make_shared<FieldInfo>(DebugInfo{}, "a", MakeInt(16)),
              std::make_shared<FieldInfo>(DebugInfo{}, "b", MakeInt(32)),
          });

  auto obj = MakeObjectFromFile(def, {0x11, 0x22, 0x55, 0x66, 0x77, 0x88});
  ASSERT_TRUE(obj);
  EXPECT_TRUE(obj->HasField("a"));
  EXPECT_TRUE(obj->HasField("b"));
  EXPECT_FALSE(obj->HasField("c"));
  EXPECT_EQ(obj->GetFieldValue("a"), Value{0x1122});
  EXPECT_EQ(obj->GetFieldValue("b"), Value{0x55667788});
  EXPECT_EQ(obj->GetFieldValue("c"), Value{});
}

}  // namespace binary_reader
