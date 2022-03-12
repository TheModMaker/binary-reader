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
// See the License for the specific language governing permissionsand
// limitations under the License.

// TODO: Remove this and use fopen_s.  We can't for now since it gives
// permission errors in the tests.
#define _CRT_SECURE_NO_WARNINGS

#include "binary_reader/file_system.h"

#include <errno.h>

#include <cstdio>

#ifdef WIN32
#  define fseeko _fseeki64
#  define ftello _ftelli64
#endif

namespace binary_reader {

namespace {

constexpr const size_t kReadSize = 16 * 1024 * 1024;

class FileFileReader sealed : public FileReader {
 public:
  FileFileReader(const std::string& path, std::FILE* fs)
      : path_(path), fs_(fs), size_(GetSize(fs)) {}
  ~FileFileReader() override {}

  bool can_seek() const override {
    return true;
  }

  uint64_t position() const override {
    return ftello(fs_);
  }

  std::optional<uint64_t> size() const {
    return size_;
  }

  bool Read(uint8_t* buffer, size_t* size, ErrorInfo* error) override {
    *size = std::fread(buffer, 1, *size, fs_);
    if (std::ferror(fs_)) {
      *error = ErrorInfo{
          path_, "Error reading from file.  errno=" + std::to_string(errno),
          ErrorLevel::Error};
      return false;
    }
    return true;
  }

  bool Seek(uint64_t* position, ErrorInfo* error) override {
    // We can seek past the end of the file; since we want to only read to the
    // end of the file, clamp it to the size.
    if (*position > size_)
      *position = size_;
    if (fseeko(fs_, *position, SEEK_SET)) {
      *error = ErrorInfo{path_,
                         "Error seeking file.  errno=" + std::to_string(errno),
                         ErrorLevel::Error};
      return false;
    }
    return true;
  }

 private:
  static uint64_t GetSize(FILE* fs) {
    std::fseek(fs, 0, SEEK_END);
    const uint64_t ret = ftello(fs);
    std::fseek(fs, 0, SEEK_SET);
    return ret;
  }

  const std::string path_;
  FILE* const fs_;
  const uint64_t size_;
};

class FStreamFileSystem sealed : public FileSystem {
 public:
  FStreamFileSystem() {}

  std::shared_ptr<FileReader> Open(const std::string& path) {
    FILE* fs = fopen(path.c_str(), "rb");
    if (!fs)
      return nullptr;
    return std::make_shared<FileFileReader>(path, fs);
  }
};

}  // namespace

bool FileReader::ReadFully(std::vector<uint8_t>* buffer, ErrorInfo* error) {
  std::vector<uint8_t> ret;
  uint64_t beginning = 0;
  if (!Seek(&beginning, error))
    return false;

  std::optional<uint64_t> size = this->size();
  while (true) {
    const size_t offset = ret.size();
    size_t read = size.value_or(kReadSize);
    size.reset();
    ret.resize(offset + read);
    if (!Read(&ret[offset], &read, error))
      return false;
    ret.resize(offset + read);
    if (read == 0) {
      std::swap(*buffer, ret);
      return true;
    }
  }
}

std::shared_ptr<FileSystem> FileSystem::DefaultFileSystem() {
  return std::make_shared<FStreamFileSystem>();
}

}  // namespace binary_reader
