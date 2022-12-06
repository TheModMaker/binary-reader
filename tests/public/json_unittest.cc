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

#include "binary_reader/json.h"

#include <algorithm>
#include <sstream>

#include "binary_reader/file_object.h"
#include "gtest_wrapper.h"
#include "public/file_object_init.h"

namespace binary_reader {

namespace {

std::shared_ptr<FileObject> MakeObject(
    std::vector<std::pair<std::string, Value>> fields) {
  FileObjectInit init;
  init.test_fields = std::move(fields);
  return MakeFileObject(init);
}

}  // namespace

TEST(JsonTest, NoFields) {
  auto obj = MakeObject({});

  {
    std::stringstream ss;
    JsonOptions opts;
    opts.pretty = false;
    DumpJsonObject(ss, opts, obj);
    EXPECT_EQ(ss.str(), "{}");
  }

  {
    std::stringstream ss;
    JsonOptions opts;
    opts.pretty = true;
    DumpJsonObject(ss, opts, obj);
    EXPECT_EQ(ss.str(), "{}\n");
  }
}

TEST(JsonTest, Primitives) {
  auto tests = {
      std::make_pair(Value{}, "null"),
      std::make_pair(Value{123ull}, "123"),
      std::make_pair(Value{77ll}, "77"),
      std::make_pair(Value{-45ll}, "-45"),
      std::make_pair(Value{2.4}, "2.4"),
  };
  auto obj = MakeObject({});

  size_t i = 0;
  for (const auto& pair : tests) {
    std::stringstream ss;
    DumpJsonValue(ss, {}, pair.first);
    EXPECT_EQ(ss.str(), pair.second) << i++;
  }
}

TEST(JsonTest, NoFieldsNested) {
  auto obj = MakeObject({
      std::make_pair("a", Value{MakeObject({})}),
  });

  {
    std::stringstream ss;
    JsonOptions opts;
    opts.pretty = false;
    DumpJsonObject(ss, opts, obj);
    EXPECT_EQ(ss.str(), "{\"a\":{}}");
  }

  {
    std::stringstream ss;
    JsonOptions opts;
    opts.pretty = true;
    DumpJsonObject(ss, opts, obj);
    EXPECT_EQ(ss.str(), "{\n  \"a\": {}\n}\n");
  }
}

TEST(JsonTest, MultipleFields) {
  auto obj = MakeObject({
      std::make_pair("a", Value{1}),
      std::make_pair("b", Value{2}),
      std::make_pair("c", Value{3}),
  });

  std::stringstream ss;
  JsonOptions opts;
  opts.pretty = false;
  DumpJsonObject(ss, opts, obj);
  EXPECT_EQ(ss.str(), "{\"a\":1,\"b\":2,\"c\":3}");
}

TEST(JsonTest, DeepNested) {
  auto obj = MakeObject({
      std::make_pair("a", Value{MakeObject({
                              std::make_pair("x1", Value{1}),
                              std::make_pair("x2", Value{2}),
                          })}),
      std::make_pair("b", Value{MakeObject({
                              std::make_pair("y1", Value{MakeObject({})}),
                          })}),
      std::make_pair("c",
                     Value{MakeObject({
                         std::make_pair("z1", Value{3}),
                         std::make_pair("z2", Value{MakeObject({
                                                  std::make_pair("w", Value{4}),
                                              })}),
                     })}),
      std::make_pair("d", Value{5}),
  });

  {
    std::stringstream ss;
    JsonOptions opts;
    opts.pretty = false;
    DumpJsonObject(ss, opts, obj);
    EXPECT_EQ(ss.str(),
              "{\"a\":{\"x1\":1,\"x2\":2},\"b\":{\"y1\":{}},\"c\":{\"z1\":3,"
              "\"z2\":{\"w\":4}},\"d\":5}");
  }

  {
    std::stringstream ss;
    JsonOptions opts;
    opts.pretty = true;
    opts.indent = 2;
    DumpJsonObject(ss, opts, obj);
    EXPECT_EQ(ss.str(), R"({
  "a": {
    "x1": 1,
    "x2": 2
  },
  "b": {
    "y1": {}
  },
  "c": {
    "z1": 3,
    "z2": {
      "w": 4
    }
  },
  "d": 5
}
)");
  }
}

}  // namespace binary_reader
