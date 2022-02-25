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

#ifndef BINARY_READER_INCLUDE_FILE_SYSTEM_H_
#define BINARY_READER_INCLUDE_FILE_SYSTEM_H_

#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace binary_reader {

/// <summary>
/// An abstract class for reading from a file.  Files are assumed to not change
/// outside this object and their size cannot change at all.  The file position
/// must be clamped to within the existing file data.
///
/// This type is not thread-safe.
/// </summary>
class FileReader {
 public:
  virtual ~FileReader() {}

  /// <summary>
  /// Returns whether the file can be seeked.  This cannot change during the
  /// lifetime of the object.
  /// </summary>
  virtual bool can_seek() const = 0;

  /// <summary>
  /// Returns the absolute byte position within the file.
  /// </summary>
  virtual uint64_t position() const = 0;

  /// <summary>
  /// Returns the total file size in bytes, or nullopt if the file size isn't
  /// known.  The file size will not change while this object exists.
  /// </summary>
  virtual std::optional<uint64_t> size() const = 0;

  /// <summary>
  /// Reads the given number of bytes from the file.  This can read less bytes
  /// than asked for, but must read at least one byte unless at EOF.
  /// </summary>
  /// <param name="buffer">An output buffer to copy bytes into.</param>
  /// <param name="size">
  /// On input, contains the number of bytes to read; on output, it should be
  /// updated to the number of bytes read.
  /// </param>
  /// <returns>True on success, false on error.</returns>
  virtual bool Read(uint8_t* buffer, size_t* size) = 0;

  /// <summary>
  /// Seeks to the given absolute byte position.  This should return false if
  /// the file is not seekable.  This can seek before the given position if the
  /// file is smaller than that.
  /// </summary>
  /// <param name="position">
  /// On input, the position to seek to; on output, should be updated to the
  /// position that was seeked to.
  /// </param>
  /// <returns>True on success, false on error.</returns>
  virtual bool Seek(uint64_t* position) = 0;

  /// <summary>
  /// Reads the whole file into the given buffer.  The current file position is
  /// ignored and will be seeked to the end of the file.  Any existing data in
  /// the buffer will be erased.
  /// </summary>
  /// <param name="buffer">The buffer to fill.</param>
  /// <returns>True on success, false on error.</returns>
  bool ReadFully(std::vector<uint8_t>* buffer);
};

/// <summary>
/// An abstract class to handle file system operations.
/// </summary>
class FileSystem {
 public:
  /// <summary>
  /// Creates a new instance of the default file system implementation.
  /// </summary>
  static std::shared_ptr<FileSystem> DefaultFileSystem();

  /// <summary>
  /// Attempts to open a reader for the given file.
  /// </summary>
  /// <param name="path">The path to the file to open.</param>
  /// <returns>The new reader object on success, or null on error.</returns>
  virtual std::shared_ptr<FileReader> Open(const std::string& path) = 0;
};

}  // namespace binary_reader

#endif  // BINARY_READER_INCLUDE_FILE_SYSTEM_H_
