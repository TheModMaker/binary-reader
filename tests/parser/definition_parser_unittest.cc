// Copyright 2021 Google LLC
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

#include "parser/definition_parser.h"

#include "gtest_wrapper.h"

namespace binary_reader {

namespace {

bool ParseSuccess(const char* val,
                  std::vector<std::shared_ptr<TypeDefinition>>* defs) {
  ErrorCollection errors;
  ParseDefinitionFile("", val, defs, &errors);
  for (const auto& error : errors) {
    const char* levels[] = {"Error", "Warning", "Info"};
    printf("%s(%zu): %s\n", levels[static_cast<int>(error.level)], error.line,
           error.message.c_str());
  }
  return errors.empty();
}

void CheckInteger(std::shared_ptr<TypeInfoBase> info, const std::string& alias,
                  size_t size, Signedness sign,
                  ByteOrder order = ByteOrder::Unset) {
  auto* integer = dynamic_cast<IntegerTypeInfo*>(info.get());
  ASSERT_TRUE(integer);
  EXPECT_EQ(integer->alias_name(), alias);
  EXPECT_EQ(integer->base_name(), "integer");
  EXPECT_EQ(*integer->static_size(), Size::FromBits(size));
  EXPECT_EQ(integer->signedness(), sign);
  EXPECT_EQ(integer->byte_order(), order);
}

std::shared_ptr<FieldInfo> as_field(std::shared_ptr<Statement> statement) {
  return std::dynamic_pointer_cast<FieldInfo>(statement);
}

std::shared_ptr<TypeDefinition> as_type_def(
    std::shared_ptr<Statement> statement) {
  return std::dynamic_pointer_cast<TypeDefinition>(statement);
}

#define CHECK_ERROR2(error, li, c, le) \
  EXPECT_EQ((error).line, (li));       \
  EXPECT_EQ((error).column, (c));      \
  EXPECT_EQ((error).level, (le))

#define CHECK_ERROR(error, l, c) CHECK_ERROR2(error, l, c, ErrorLevel::Error)

}  // namespace

TEST(DefinitionParserTest, ParseFile_EmptyFile) {
  std::vector<std::shared_ptr<TypeDefinition>> defs;
  ASSERT_TRUE(ParseSuccess("", &defs));
  EXPECT_EQ(defs.size(), 0u);
}

TEST(DefinitionParserTest, ParseFile_EmptyType) {
  std::vector<std::shared_ptr<TypeDefinition>> defs;
  ASSERT_TRUE(ParseSuccess("type foo {}", &defs));
  ASSERT_EQ(defs.size(), 1u);

  EXPECT_EQ(defs[0]->alias_name(), "foo");
  EXPECT_EQ(defs[0]->base_name(), "foo");
  EXPECT_EQ(defs[0]->statements().size(), 0u);
}

TEST(DefinitionParserTest, ParseFile_Fields) {
  std::vector<std::shared_ptr<TypeDefinition>> defs;
  std::shared_ptr<FieldInfo> field;
  ASSERT_TRUE(ParseSuccess("type foo { int32 x; int64 y; uint16 z; }", &defs));
  ASSERT_EQ(defs.size(), 1u);

  ASSERT_EQ(defs[0]->statements().size(), 3u);
  ASSERT_TRUE((field = as_field(defs[0]->statements()[0])));
  EXPECT_EQ(field->name(), "x");
  CheckInteger(field->type(), "int32", 32, Signedness::Signed);
  ASSERT_TRUE((field = as_field(defs[0]->statements()[1])));
  EXPECT_EQ(field->name(), "y");
  CheckInteger(field->type(), "int64", 64, Signedness::Signed);
  ASSERT_TRUE((field = as_field(defs[0]->statements()[2])));
  EXPECT_EQ(field->name(), "z");
  CheckInteger(field->type(), "uint16", 16, Signedness::Unsigned);
}

TEST(DefinitionParserTest, ParseFile_CantUseRecursiveTypes) {
  std::vector<std::shared_ptr<TypeDefinition>> defs;
  std::shared_ptr<FieldInfo> field;
  ASSERT_FALSE(ParseSuccess("type foo { foo x; }", &defs));
}

TEST(DefinitionParserTest, ParseFile_Errors) {
  std::vector<std::shared_ptr<TypeDefinition>> defs;
  ErrorCollection errors;

  // Unknown type
  ASSERT_FALSE(ParseDefinitionFile("", "type foo { abc x; }", &defs, &errors));
  ASSERT_EQ(errors.size(), 1u);
  CHECK_ERROR(errors.front(), 1, 11);

  // Duplicate type
  errors.clear();
  ASSERT_FALSE(ParseDefinitionFile(
      "", "type foo { int16 x; } type foo { int32 y; }", &defs, &errors));
  ASSERT_EQ(errors.size(), 1u);

  // Duplicate field
  CHECK_ERROR(errors.front(), 1, 22);
  errors.clear();
  ASSERT_TRUE(ParseDefinitionFile("", "type foo { int16 x; int32 foo; }", &defs,
                                  &errors));
  ASSERT_EQ(errors.size(), 1u);
  CHECK_ERROR2(errors.front(), 1, 20, ErrorLevel::Warning);
  errors.clear();
  ASSERT_FALSE(ParseDefinitionFile("", "type foo { int16 x; int32 x; }", &defs,
                                   &errors));
  ASSERT_EQ(errors.size(), 1u);
  CHECK_ERROR(errors.front(), 1, 20);
  errors.clear();
  ASSERT_TRUE(
      ParseDefinitionFile("", "type foo { int16 int32; }", &defs, &errors));
  ASSERT_EQ(errors.size(), 1u);
  CHECK_ERROR2(errors.front(), 1, 11, ErrorLevel::Warning);
}

}  // namespace binary_reader
