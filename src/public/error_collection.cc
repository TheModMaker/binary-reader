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

#include "binary_reader/error_collection.h"

namespace binary_reader {

ErrorCollection::ErrorCollection() {}

ErrorCollection::ErrorCollection(const std::string& file_path)
    : file_path_(file_path) {}

ErrorCollection::~ErrorCollection() = default;

void ErrorCollection::Add(const ErrorInfo& info) {
  errors_.emplace_back(info);
  if (info.level == ErrorLevel::Error)
    has_error_ = true;
}
void ErrorCollection::AddError(const std::string& message, uint64_t offset,
                               size_t line, size_t column) {
  errors_.push_back(
      {file_path_, message, ErrorLevel::Error, offset, line, column});
  has_error_ = true;
}

void ErrorCollection::AddWarning(const std::string& message, uint64_t offset,
                                 size_t line, size_t column) {
  errors_.push_back(
      {file_path_, message, ErrorLevel::Warning, offset, line, column});
}

void ErrorCollection::AddInfo(const std::string& message, uint64_t offset,
                              size_t line, size_t column) {
  errors_.push_back(
      {file_path_, message, ErrorLevel::Info, offset, line, column});
}

}  // namespace binary_reader
