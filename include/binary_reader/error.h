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

#include <initializer_list>
#include <iostream>
#include <string>
#include <string_view>

namespace binary_reader {

enum class ErrorLevel : uint8_t {
  Error,
  Warning,
  Info,
};

enum class ErrorKind : uint16_t {
  Unknown = 0,
  CannotOpen,
  IoError,

  ShadowingType = 6000,
  ShadowingMember,
  UnknownType,
  NoTypes,

  OptionMustBeString = 8000,
  OptionMustBeStringTyped,
  UnknownOptionValue,
  UnknownOptionValueTyped,
  AmbiguousOption,
  DuplicateOption,
  OptionInvalidForType,
  UnknownOptionType,

  UnexpectedEndOfStream = 10000,
  LittleEndianAlign,

  FieldsMustBeStatic = 12000,
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
  ErrorInfo();
  ErrorInfo(DebugInfo debug, ErrorKind kind,
            ErrorLevel level = ErrorLevel::Error, uint64_t offset = 0);
  ErrorInfo(DebugInfo debug, ErrorKind kind,
            std::initializer_list<std::string_view> message_args,
            ErrorLevel level = ErrorLevel::Error, uint64_t offset = 0);
  ErrorInfo(DebugInfo debug, ErrorKind kind, std::string_view message,
            ErrorLevel level = ErrorLevel::Error, uint64_t offset = 0);

  /// <summary>
  /// The debug info describing where the error happened.
  /// </summary>
  DebugInfo debug;
  /// <summary>
  /// The error message.
  /// </summary>
  std::string message;
  /// <summary>
  /// The kind of error that occurred.
  /// </summary>
  ErrorKind kind = ErrorKind::Unknown;
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

/// <summary>
/// Creates the default error message for the given error kind.  The |args|
/// specify format arguments to the error string; any missing values will use
/// the empty string and extras are ignored.
/// </summary>
std::string DefaultErrorMessage(ErrorKind kind,
                                std::initializer_list<std::string_view> args);

std::ostream& operator<<(std::ostream& os, ErrorLevel level);
std::ostream& operator<<(std::ostream& os, const ErrorInfo& error);

}  // namespace binary_reader

#endif  // BINARY_READER_INCLUDE_ERROR_H_
