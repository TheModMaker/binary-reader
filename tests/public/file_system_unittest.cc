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

#include "binary_reader/file_system.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>

namespace binary_reader {

namespace {

class MockFileReader sealed : public FileReader {
 public:
  MOCK_METHOD(bool, can_seek, (), (const, override));
  MOCK_METHOD(uint64_t, position, (), (const, override));
  MOCK_METHOD(std::optional<uint64_t>, size, (), (const, override));
  MOCK_METHOD(bool, Read, (uint8_t*, size_t*), (override));
  MOCK_METHOD(bool, Seek, (uint64_t*), (override));
};

using testing::_;
using testing::DoAll;
using testing::Gt;
using testing::InSequence;
using testing::Invoke;
using testing::Pointee;
using testing::Return;
using testing::SetArgPointee;
using testing::SetArrayArgument;

}  // namespace

class FileSystemTest : public testing::Test {
 public:
  void SetUp() override {
    dir_ = std::filesystem::temp_directory_path() / "binary_reader_tests";
    std::filesystem::create_directories(dir_);
  }

  void TearDown() override {
    std::filesystem::remove_all(dir_);
  }

  template <size_t N>
  void MakeFile(const std::string& path, const uint8_t (&data)[N]) {
    std::fstream fs(path, std::ios::trunc | std::ios::out | std::ios::binary);
    ASSERT_TRUE(fs);
    fs << std::string{data, data + N};
  }

 protected:
  std::filesystem::path dir_;
};

TEST_F(FileSystemTest, ReadFully_Sized) {
  const uint8_t expected[] = "data";
  MockFileReader reader;
  {
    InSequence seq;
    EXPECT_CALL(reader, Seek(Pointee(0))).WillOnce(Return(true));
    EXPECT_CALL(reader, size()).WillOnce(Return(sizeof(expected)));
    EXPECT_CALL(reader, Read(_, Pointee(sizeof(expected))))
        .WillOnce(
            DoAll(SetArrayArgument<0>(expected, expected + sizeof(expected)),
                  Return(true)));
    EXPECT_CALL(reader, Read(_, _))
        .WillOnce(DoAll(SetArgPointee<1>(0), Return(true)));
  }

  std::vector<uint8_t> output;
  ASSERT_TRUE(reader.ReadFully(&output));
  ASSERT_EQ(output.size(), sizeof(expected));
  ASSERT_EQ(memcmp(output.data(), expected, sizeof(expected)), 0);
}

TEST_F(FileSystemTest, ReadFully_IncrementalReads) {
  const uint8_t expected[] = "data_foo_bar";
  MockFileReader reader;
  {
    InSequence seq;
    EXPECT_CALL(reader, Seek(Pointee(0))).WillOnce(Return(true));
    EXPECT_CALL(reader, size()).WillOnce(Return(std::nullopt));
    EXPECT_CALL(reader, Read(_, Pointee(Gt(5))))
        .WillOnce(DoAll(SetArrayArgument<0>(expected, expected + 5),
                        SetArgPointee<1>(5), Return(true)))
        .WillOnce(DoAll(SetArrayArgument<0>(expected + 5, expected + 9),
                        SetArgPointee<1>(4), Return(true)))
        .WillOnce(DoAll(
            SetArrayArgument<0>(expected + 9, expected + sizeof(expected)),
            SetArgPointee<1>(sizeof(expected) - 9), Return(true)))
        .WillOnce(DoAll(SetArgPointee<1>(0), Return(true)));
  }

  std::vector<uint8_t> output;
  ASSERT_TRUE(reader.ReadFully(&output));
  ASSERT_EQ(output.size(), sizeof(expected));
  ASSERT_EQ(memcmp(output.data(), expected, sizeof(expected)), 0);
}

TEST_F(FileSystemTest, ReadFully_ClearsExisting) {
  const uint8_t expected[] = "data";
  MockFileReader reader;
  {
    InSequence seq;
    EXPECT_CALL(reader, Seek(Pointee(0))).WillOnce(Return(true));
    EXPECT_CALL(reader, size()).WillOnce(Return(sizeof(expected)));
    EXPECT_CALL(reader, Read(_, Pointee(sizeof(expected))))
        .WillOnce(
            DoAll(SetArrayArgument<0>(expected, expected + sizeof(expected)),
                  Return(true)));
    EXPECT_CALL(reader, Read(_, _))
        .WillOnce(DoAll(SetArgPointee<1>(0), Return(true)));
  }

  std::vector<uint8_t> output;
  output.assign(expected, expected + sizeof(expected));
  ASSERT_TRUE(reader.ReadFully(&output));
  ASSERT_EQ(output.size(), sizeof(expected));
  ASSERT_EQ(memcmp(output.data(), expected, sizeof(expected)), 0);
}

TEST_F(FileSystemTest, ReadFully_PropagatesErrors) {
  {
    MockFileReader reader;
    InSequence seq;
    EXPECT_CALL(reader, Seek(Pointee(0))).WillOnce(Return(false));

    std::vector<uint8_t> output;
    ASSERT_FALSE(reader.ReadFully(&output));
  }

  {
    MockFileReader reader;
    InSequence seq;
    EXPECT_CALL(reader, Seek(Pointee(0))).WillOnce(Return(true));
    EXPECT_CALL(reader, size()).WillOnce(Return(10));
    EXPECT_CALL(reader, Read(_, _)).WillOnce(Return(false));

    std::vector<uint8_t> output;
    ASSERT_FALSE(reader.ReadFully(&output));
  }

  {
    const uint8_t expected[] = "data";
    MockFileReader reader;
    InSequence seq;
    EXPECT_CALL(reader, Seek(Pointee(0))).WillOnce(Return(true));
    EXPECT_CALL(reader, size()).WillOnce(Return(sizeof(expected)));
    EXPECT_CALL(reader, Read(_, Pointee(sizeof(expected))))
        .WillOnce(
            DoAll(SetArrayArgument<0>(expected, expected + sizeof(expected)),
                  Return(true)));
    EXPECT_CALL(reader, Read(_, _)).WillOnce(Return(false));

    std::vector<uint8_t> output;
    ASSERT_FALSE(reader.ReadFully(&output));
  }
}

TEST_F(FileSystemTest, Default_Read) {
  const uint8_t expected[] = "contents";
  MakeFile("file.def", expected);

  auto system = FileSystem::DefaultFileSystem();
  ASSERT_TRUE(system);
  auto reader = system->Open("file.def");
  ASSERT_TRUE(reader);
  EXPECT_TRUE(reader->can_seek());
  EXPECT_EQ(reader->position(), 0u);
  EXPECT_EQ(reader->size(), sizeof(expected));

  uint8_t buffer[sizeof(expected)];
  size_t read = sizeof(buffer);
  ASSERT_TRUE(reader->Read(buffer, &read));
  ASSERT_EQ(read, sizeof(buffer));
  EXPECT_EQ(memcmp(expected, buffer, sizeof(buffer)), 0);
}

TEST_F(FileSystemTest, Default_ReadPartial) {
  const uint8_t expected[] = "contents";
  MakeFile("file.def", expected);

  auto system = FileSystem::DefaultFileSystem();
  ASSERT_TRUE(system);
  auto reader = system->Open("file.def");
  ASSERT_TRUE(reader);

  uint8_t buffer[3];
  size_t read = sizeof(buffer);
  ASSERT_TRUE(reader->Read(buffer, &read));
  ASSERT_EQ(reader->position(), 3u);
  ASSERT_EQ(read, sizeof(buffer));
  EXPECT_EQ(memcmp("con", buffer, sizeof(buffer)), 0);

  ASSERT_TRUE(reader->Read(buffer, &read));
  ASSERT_EQ(reader->position(), 6u);
  ASSERT_EQ(read, sizeof(buffer));
  EXPECT_EQ(memcmp("ten", buffer, sizeof(buffer)), 0);
}

TEST_F(FileSystemTest, Default_ReadLess) {
  const uint8_t expected[] = "contents";
  MakeFile("file.def", expected);

  auto system = FileSystem::DefaultFileSystem();
  ASSERT_TRUE(system);
  auto reader = system->Open("file.def");
  ASSERT_TRUE(reader);

  uint8_t buffer[50];
  size_t read = sizeof(buffer);
  ASSERT_TRUE(reader->Read(buffer, &read));
  ASSERT_EQ(reader->position(), sizeof(expected));
  ASSERT_EQ(read, sizeof(expected));
  EXPECT_EQ(memcmp(expected, buffer, sizeof(expected)), 0);
}

TEST_F(FileSystemTest, Default_ReadEof) {
  const uint8_t expected[] = "";
  MakeFile("file.def", expected);

  auto system = FileSystem::DefaultFileSystem();
  ASSERT_TRUE(system);
  auto reader = system->Open("file.def");
  ASSERT_TRUE(reader);
  size_t seek = 1;
  ASSERT_TRUE(reader->Seek(&seek));

  uint8_t buffer[50];
  size_t read = sizeof(buffer);
  ASSERT_TRUE(reader->Read(buffer, &read));
  ASSERT_EQ(reader->position(), 1);
  ASSERT_EQ(read, 0);

  ASSERT_TRUE(reader->Read(buffer, &read));
  ASSERT_EQ(reader->position(), 1);
  ASSERT_EQ(read, 0);
}

TEST_F(FileSystemTest, Default_Seek) {
  const uint8_t expected[] = "contents";
  MakeFile("file.def", expected);

  auto system = FileSystem::DefaultFileSystem();
  ASSERT_TRUE(system);
  auto reader = system->Open("file.def");
  ASSERT_TRUE(reader);
  ASSERT_EQ(reader->position(), 0u);

  size_t seek = 5;
  ASSERT_TRUE(reader->Seek(&seek));
  ASSERT_EQ(seek, 5);
  ASSERT_EQ(reader->position(), 5u);

  seek = 200;
  ASSERT_TRUE(reader->Seek(&seek));
  ASSERT_EQ(seek, sizeof(expected));
  ASSERT_EQ(reader->position(), sizeof(expected));

  seek = 3;
  ASSERT_TRUE(reader->Seek(&seek));
  ASSERT_EQ(seek, 3);
  ASSERT_EQ(reader->position(), 3u);

  uint8_t buffer[3];
  size_t read = sizeof(buffer);
  ASSERT_TRUE(reader->Read(buffer, &read));
  ASSERT_EQ(reader->position(), 6);
  ASSERT_EQ(read, 3);
  EXPECT_EQ(memcmp("ten", buffer, 3), 0);
}

TEST_F(FileSystemTest, Default_NotExist) {
  auto system = FileSystem::DefaultFileSystem();
  EXPECT_FALSE(system->Open("foo.def"));
}

}  // namespace binary_reader