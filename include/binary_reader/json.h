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
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef BINARY_READER_APP_JSON_H_
#define BINARY_READER_APP_JSON_H_

#include <iostream>
#include <memory>

#include "binary_reader/cloneable_unique_ptr.h"
#include "binary_reader/file_object.h"
#include "binary_reader/value.h"

namespace binary_reader {

class JsonOptions final {
 public:
  JsonOptions();
  ~JsonOptions();
  JsonOptions(const JsonOptions&);
  JsonOptions(JsonOptions&&);
  JsonOptions& operator=(const JsonOptions&);
  JsonOptions& operator=(JsonOptions&&);

  /// <summary>
  /// Whether to include newlines and indentation in the output.
  /// </summary>
  bool pretty = true;

  /// <summary>
  /// The number of spaces to add for each level of indentation.  Only has an
  /// effect when using pretty printing.
  /// </summary>
  size_t indent = 2;

 private:
  // For forward-compatibility, this allows us to add new fields without
  // breaking ABI (through member functions).
  struct Impl;
  cloneable_unique_ptr<Impl> impl_;
};

void DumpJsonValue(std::ostream& os, const JsonOptions& opts, Value obj,
                   size_t indent = 0);
void DumpJsonObject(std::ostream& os, const JsonOptions& opts,
                    std::shared_ptr<FileObject> obj, size_t indent = 0);

}  // namespace binary_reader

#endif  // BINARY_READER_APP_JSON_H_
