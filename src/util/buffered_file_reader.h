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

#ifndef BINARY_READER_UTIL_BUFFERED_FILE_READER_H_
#define BINARY_READER_UTIL_BUFFERED_FILE_READER_H_

#include <memory>

#include "binary_reader/error_collection.h"
#include "binary_reader/file_system.h"
#include "binary_reader/size.h"
#include "util/macros.h"

namespace binary_reader {

/// <summary>
/// This reads buffers from the given FileReader and maintains the temporary
/// buffers.  This will keep larger buffers and maintain the exisitng
/// buffers.
/// </summary>
class BufferedFileReader final {
  NON_COPYABLE_OR_MOVABLE_TYPE(BufferedFileReader);

 public:
  explicit BufferedFileReader(std::shared_ptr<FileReader> reader);

  Size position() const;

  /// <summary>
  /// Seeks the current position of the reader to the given absolute file
  /// position.
  /// </summary>
  /// <param name="position">The position to seek to.</param>
  /// <param name="errors">Will be given any errors that happen.</param>
  /// <returns>True on success, false on error.</returns>
  bool Seek(Size position, ErrorCollection* errors);

  /// <summary>
  /// Skips the buffer position forward the given size.
  /// </summary>
  /// <param name="count">The size to skip forward.</param>
  /// <param name="errors">Will be given any errors that happen.</param>
  /// <returns>True on success, false on error.</returns>
  bool Skip(Size count, ErrorCollection* errors);

  /// <summary>
  /// Gets the buffer pointer at the current read position.  Since this
  /// operates on bytes, this ignores the bit offset.  The caller must
  /// account for bit offset when reading this buffer.
  /// </summary>
  /// <param name="buffer">Will be filled with the data pointer.</param>
  /// <param name="size">
  /// Will be filled in with the number of bytes available.
  /// </param>
  /// <param name="errors">Will be given any errors that happen.</param>
  /// <returns>True on success, false on error.</returns>
  bool GetBuffer(const uint8_t** buffer, size_t* size, ErrorCollection* errors);

  /// <summary>
  /// Ensures at least the given size is buffered.  If this hits
  /// EOF, this will return true and buffer to the end.
  /// </summary>
  /// <param name="size">The size to buffer.</param>
  /// <param name="errors">Will be given any errors that happen.</param>
  /// <returns>True on success, false on error.</returns>
  bool EnsureBuffer(Size size, ErrorCollection* errors);

 private:
  const std::shared_ptr<FileReader> reader_;
  const std::unique_ptr<uint8_t[]> buffer_;
  Size start_position_;
  Size buffer_offset_;
  size_t used_;
};

}  // namespace binary_reader

#endif  // BINARY_READER_UTIL_BUFFERED_FILE_READER_H_
