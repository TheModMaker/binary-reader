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

#ifndef BINARY_READER_INCLUDE_ERROR_COLLECTION_H_
#define BINARY_READER_INCLUDE_ERROR_COLLECTION_H_

#include <string>
#include <vector>

#include "binary_reader/error.h"

namespace binary_reader {

/// <summary>
/// Holds a collection of error objects and contains utilites for adding and
/// querying errors.  This is designed to work with a single file, and that
/// path is copied to each error.
/// </summary>
class ErrorCollection sealed {
 public:
  ErrorCollection();
  ErrorCollection(const std::string& file_path);
  ErrorCollection(const ErrorCollection&) = delete;
  ErrorCollection(ErrorCollection&&) = delete;
  ~ErrorCollection();

  ErrorCollection& operator=(const ErrorCollection&) = delete;
  ErrorCollection& operator=(ErrorCollection&&) = delete;

  auto begin() const {
    return errors_.begin();
  }
  auto end() const {
    return errors_.end();
  }

  ErrorInfo& front() {
    return errors_.front();
  }
  const ErrorInfo& front() const {
    return errors_.front();
  }
  ErrorInfo& back() {
    return errors_.back();
  }
  const ErrorInfo& back() const {
    return errors_.back();
  }

  void clear() {
    errors_.clear();
    has_error_ = false;
  }
  size_t size() {
    return errors_.size();
  }

  const std::vector<ErrorInfo>& errors() const {
    return errors_;
  }

  bool empty() const {
    return errors_.empty();
  }
  bool has_errors() const {
    return has_error_;
  }

  void Add(const ErrorInfo& info);
  void AddError(const std::string& message, uint64_t offset = 0,
                size_t line = 0, size_t column = 0);
  void AddWarning(const std::string& message, uint64_t offset = 0,
                  size_t line = 0, size_t column = 0);
  void AddInfo(const std::string& message, uint64_t offset = 0, size_t line = 0,
               size_t column = 0);

  void AddAllFrom(const ErrorCollection& errors);

 private:
  const std::string file_path_;
  std::vector<ErrorInfo> errors_;
  bool has_error_ = false;
};

}  // namespace binary_reader

#endif  // BINARY_READER_INCLUDE_ERROR_COLLECTION_H_
