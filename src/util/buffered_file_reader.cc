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

#include "util/buffered_file_reader.h"

#include <assert.h>
#include <string.h>

namespace binary_reader {

namespace {

constexpr const size_t kBufferSize = 64 * 1024 * 1024;

}  // namespace

BufferedFileReader::BufferedFileReader(std::shared_ptr<FileReader> reader)
    : reader_(reader), buffer_(new uint8_t[kBufferSize]), used_(0) {}

Size BufferedFileReader::position() const {
  return start_position_ + buffer_offset_;
}

bool BufferedFileReader::Seek(Size position, ErrorCollection* errors) {
  if (position >= start_position_ &&
      position < start_position_ + Size::FromBytes(used_)) {
    buffer_offset_ = position - start_position_;
    return true;
  }

  start_position_ = position.ClipToByte();
  buffer_offset_ = Size::FromBits(position.bit_offset());
  used_ = 0;
  uint64_t byte_pos = position.byte_count();
  if (!reader_->Seek(&byte_pos, errors))
    return false;
  return true;
}

bool BufferedFileReader::Skip(Size count, ErrorCollection* errors) {
  return Seek(start_position_ + buffer_offset_ + count, errors);
}

bool BufferedFileReader::GetBuffer(const uint8_t** buffer, size_t* size,
                                   ErrorCollection* errors) {
  if (!EnsureBuffer(Size::FromBits(1), errors))
    return false;

  *buffer = buffer_.get() + buffer_offset_.byte_count();
  *size = used_ - buffer_offset_.byte_count();
  return true;
}

bool BufferedFileReader::EnsureBuffer(Size size, ErrorCollection* errors) {
  assert(size <= Size::FromBytesAndOffset(kBufferSize, 7));
  assert(buffer_offset_ <= Size::FromBytesAndOffset(kBufferSize, 7));
  assert(buffer_offset_ <= Size::FromBytesAndOffset(used_, 7));

  if (buffer_offset_ + size > Size::FromBytes(kBufferSize)) {
    // If we don't have enough space, move the existing buffer back to the
    // start of the buffer.
    const uint64_t count = buffer_offset_.byte_count();
    memmove(buffer_.get(), buffer_.get() + count, kBufferSize - count);
    used_ -= count;
    start_position_ += Size::FromBytes(count);
    buffer_offset_ = Size::FromBits(buffer_offset_.bit_offset());
  }

  while (buffer_offset_ + size > Size::FromBytes(used_)) {
    uint8_t* output = buffer_.get() + used_;
    size_t to_read = kBufferSize - used_;
    if (!reader_->Read(output, &to_read, errors))
      return false;
    if (!to_read)  // EOF
      break;
    used_ += to_read;
  }

  return true;
}

}  // namespace binary_reader
