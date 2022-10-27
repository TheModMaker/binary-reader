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

namespace binary_reader {

ErrorCollection::ErrorCollection() {}

ErrorCollection::~ErrorCollection() = default;

void ErrorCollection::Add(const ErrorInfo& info) {
  errors_.emplace_back(info);
  if (info.level == ErrorLevel::Error)
    has_error_ = true;
}

void ErrorCollection::AddAllFrom(const ErrorCollection& errors) {
  has_error_ = has_error_ || errors.has_error_;
  errors_.reserve(errors_.size() + errors.errors_.size());
  std::copy(errors.errors_.begin(), errors.errors_.end(),
            std::back_inserter(errors_));
}

}  // namespace binary_reader
