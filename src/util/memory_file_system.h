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

#ifndef BINARY_READER_UTIL_MEMORY_FILE_SYSTEM_H_
#define BINARY_READER_UTIL_MEMORY_FILE_SYSTEM_H_

#include <string>
#include <unordered_map>
#include <vector>

#include "binary_reader/file_system.h"

namespace binary_reader {

class MemoryFileReader final : public FileReader {
 public:
  MemoryFileReader(std::initializer_list<uint8_t> buffer);
  MemoryFileReader(const std::string& buffer);
  MemoryFileReader(const std::vector<uint8_t>& buffer, size_t pos);

  bool can_seek() const override;
  uint64_t position() const override;
  std::optional<uint64_t> size() const override;

  bool Read(uint8_t* buffer, size_t* size, ErrorCollection* errors) override;
  bool Seek(uint64_t* position, ErrorCollection* errors) override;

  std::shared_ptr<MemoryFileReader> Clone() const;

 private:
  const std::vector<uint8_t> buffer_;
  size_t pos_;
};

class MemoryFileSystem final : public FileSystem {
 public:
  std::shared_ptr<FileReader> Open(const std::string& path) override;

  void Add(const std::string& path, std::initializer_list<uint8_t> buffer);
  void Add(const std::string& path, const std::string& buffer);

 private:
  std::unordered_map<std::string, std::shared_ptr<MemoryFileReader>> files_;
};

}  // namespace binary_reader

#endif  // BINARY_READER_UTIL_MEMORY_FILE_SYSTEM_H_
