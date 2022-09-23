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

#include <iostream>
#include <string>

namespace binary_reader {

enum class ErrorLevel {
  Error,
  Warning,
  Info,
};

struct DebugInfo sealed {
  /// <summary>
  /// The file path to the file this info refers to.
  /// </summary>
  std::string file_path;

  /// <summary>
  /// The (one based) line number within the definition file this refers to.
  /// </summary>
  size_t line = 0;
  /// <summary>
  /// The (one based) column number within the definition file this refers to.
  /// </summary>
  size_t column = 0;
};

struct ErrorInfo sealed {
  /// <summary>
  /// The debug info describing where the error happened.
  /// </summary>
  DebugInfo debug;
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
  /// For other errors, this will be 0.
  /// </summary>
  uint64_t offset = 0;
};

std::ostream& operator<<(std::ostream& os, ErrorLevel level);
std::ostream& operator<<(std::ostream& os, const ErrorInfo& error);

}  // namespace binary_reader

#endif  // BINARY_READER_INCLUDE_ERROR_H_
