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

#include <algorithm>
#include <type_traits>

namespace binary_reader {

std::ostream& operator<<(std::ostream& os, ErrorLevel level) {
  const char* kLevels[] = {
      "error",
      "warning",
      "info",
  };
  auto l = static_cast<std::underlying_type<ErrorLevel>::type>(level);
  if (l < 0 || l > sizeof(kLevels) / sizeof(kLevels[0]))
    l = 0;
  return os << kLevels[l];
}

std::ostream& operator<<(std::ostream& os, const ErrorInfo& error) {
  if (error.debug.file_path.empty()) {
    // error: unknown type 'foo'
    return os << error.level << ": " << error.message;
  } else if (!error.debug.line) {
    // foo/bar.def: error: unknown type 'foo'
    return os << error.debug.file_path << ": " << error.level << ": "
              << error.message;
  } else if (!error.debug.column) {
    // foo/bar.def:6: error: unknown type 'foo'
    return os << error.debug.file_path << ":" << error.debug.line << ": "
              << error.level << ": " << error.message;
  } else {
    // foo/bar.def:6:12: error: unknown type 'foo'
    return os << error.debug.file_path << ":" << error.debug.line << ":"
              << error.debug.column << ": " << error.level << ": "
              << error.message;
  }
}


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
      {{file_path_, line, column}, message, ErrorLevel::Error, offset});
  has_error_ = true;
}

void ErrorCollection::AddWarning(const std::string& message, uint64_t offset,
                                 size_t line, size_t column) {
  errors_.push_back(
      {{file_path_, line, column}, message, ErrorLevel::Warning, offset});
}

void ErrorCollection::AddInfo(const std::string& message, uint64_t offset,
                              size_t line, size_t column) {
  errors_.push_back(
      {{file_path_, line, column}, message, ErrorLevel::Info, offset});
}

void ErrorCollection::AddAllFrom(const ErrorCollection& errors) {
  errors_.reserve(errors_.size() + errors.errors_.size());
  std::copy(errors.errors_.begin(), errors.errors_.end(),
            std::back_inserter(errors_));
}

}  // namespace binary_reader
