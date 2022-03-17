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

#include "util/memory_file_system.h"

#include <algorithm>
#include <cstring>

namespace binary_reader {

MemoryFileReader::MemoryFileReader(std::initializer_list<uint8_t> buffer)
    : buffer_(std::move(buffer)), pos_(0) {}

MemoryFileReader::MemoryFileReader(const std::string& buffer)
    : buffer_(buffer.begin(), buffer.end()), pos_(0) {}

MemoryFileReader::MemoryFileReader(const std::vector<uint8_t>& buffer,
                                   size_t pos)
    : buffer_(buffer), pos_(pos) {}

bool MemoryFileReader::can_seek() const {
  return true;
}

uint64_t MemoryFileReader::position() const {
  return pos_;
}

std::optional<uint64_t> MemoryFileReader::size() const {
  return buffer_.size();
}

bool MemoryFileReader::Read(uint8_t* buffer, size_t* size, ErrorCollection*) {
  *size = std::min(*size, buffer_.size() - pos_);
  if (*size > 0)
    std::memcpy(buffer, &buffer_[pos_], *size);
  pos_ += *size;
  return true;
}

bool MemoryFileReader::Seek(uint64_t* position, ErrorCollection*) {
  pos_ = *position = std::min(buffer_.size(), *position);
  return true;
}

std::shared_ptr<MemoryFileReader> MemoryFileReader::Clone() const {
  return std::make_shared<MemoryFileReader>(buffer_, pos_);
}


void MemoryFileSystem::Add(const std::string& path,
                           std::initializer_list<uint8_t> buffer) {
  files_.emplace(path, std::make_shared<MemoryFileReader>(buffer));
}

void MemoryFileSystem::Add(const std::string& path, const std::string& buffer) {
  files_.emplace(path, std::make_shared<MemoryFileReader>(buffer));
}

std::shared_ptr<FileReader> MemoryFileSystem::Open(const std::string& path) {
  auto it = files_.find(path);
  return it != files_.end() ? it->second->Clone() : nullptr;
}

}  // namespace binary_reader
