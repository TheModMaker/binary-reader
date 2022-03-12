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

#ifndef BINARY_READER_UTIL_ERROR_COLLECTION_H_
#define BINARY_READER_UTIL_ERROR_COLLECTION_H_

#include <string>
#include <vector>

#include "binary_reader/error.h"
#include "util/macros.h"

namespace binary_reader {

/// <summary>
/// Holds a collection of error objects and contains utilites for adding and
/// querying errors.  This is designed to work with a single file, and that
/// path is copied to each error.
/// </summary>
class ErrorCollection sealed {
  NON_COPYABLE_OR_MOVABLE_TYPE(ErrorCollection);

 public:
  ErrorCollection(const std::string& file_path);
  ~ErrorCollection();

  const std::vector<ErrorInfo>& errors() const {
    return errors_;
  }

  bool has_any() const;
  bool has_errors() const;

  void Add(const ErrorInfo& info);
  void AddError(const std::string& message, uint64_t offset = 0,
                size_t line = 0, size_t column = 0);
  void AddWarning(const std::string& message, uint64_t offset = 0,
                  size_t line = 0, size_t column = 0);
  void AddInfo(const std::string& message, uint64_t offset = 0, size_t line = 0,
               size_t column = 0);

  void AddEof();

 private:
  const std::string file_path_;
  std::vector<ErrorInfo> errors_;
  bool has_error_ = false;
};

}  // namespace binary_reader

#endif  // BINARY_READER_UTIL_ERROR_COLLECTION_H_
