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

#ifndef BINARY_READER_INCLUDE_ERROR_H_
#define BINARY_READER_INCLUDE_ERROR_H_

#include <string>

namespace binary_reader {

enum class ErrorLevel {
  Error,
  Warning,
  Info,
};

struct ErrorInfo sealed {
  /// <summary>
  /// The file path to the file that contains the error.
  /// </summary>
  std::string file_path;
  /// <summary>
  /// The error message.
  /// </summary>
  std::string message;
  /// <summary>
  /// The level of error this is.
  /// </summary>
  ErrorLevel level = ErrorLevel::Error;

  /// <summary>
  /// For binary files, this is the byte offset for where the error starts.
  /// For definition files, this is the character offset.
  /// For other errors, this will be 0.
  /// </summary>
  uint64_t offset = 0;
  /// <summary>
  /// For definition files, this is the (one based) line number for where the
  /// error starts.
  /// For other errors, this will be 0.
  /// </summary>
  size_t line = 0;
  /// <summary>
  /// For definition files, this is the (one based) column number for where the
  /// error starts.
  /// For other errors, this will be 0.
  /// </summary>
  size_t column = 0;
};

}  // namespace binary_reader

#endif  // BINARY_READER_INCLUDE_ERROR_H_
