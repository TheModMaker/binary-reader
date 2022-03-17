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

#include "binary_reader/file_parser.h"

#include "binary_reader/value.h"
#include "gtest_wrapper.h"
#include "util/memory_file_system.h"

namespace binary_reader {

TEST(FileParserIntegration, BasicFlow) {
  auto fs = std::make_shared<MemoryFileSystem>();
  fs->Add("file.def", "type foo { int16 a; int32 b; }");
  fs->Add("file.bin", {0x11, 0x22, 0x33, 0x44, 0x55, 0x66});

  FileParserOptions opts;
  opts.file_system = fs;
  auto parser = FileParser::CreateFromFile("file.def", opts);
  ASSERT_TRUE(parser);

  auto bin = parser->ParseFile("file.bin");
  ASSERT_TRUE(bin);
  EXPECT_TRUE(bin->HasField("a"));
  EXPECT_TRUE(bin->HasField("b"));
  EXPECT_FALSE(bin->HasField("c"));
  EXPECT_EQ(bin->GetFieldValue("a"), Value{0x1122});
  EXPECT_EQ(bin->GetFieldValue("b"), Value{0x33445566});
  EXPECT_EQ(bin->GetFieldValue("c"), Value{});
}

}  // namespace binary_reader
