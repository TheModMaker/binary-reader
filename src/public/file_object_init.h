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

#ifndef BINARY_READER_PUBLIC_FILE_OBJECT_INIT_H_
#define BINARY_READER_PUBLIC_FILE_OBJECT_INIT_H_

#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "ast/type_definition.h"
#include "binary_reader/file_object.h"
#include "binary_reader/file_system.h"
#include "binary_reader/value.h"
#include "util/size.h"

namespace binary_reader {

struct FileObjectInit sealed {
  // Normal mode
  std::shared_ptr<BufferedFileReader> file;
  std::shared_ptr<const TypeDefinition> type;
  Size start_position;

  // Test only mode
  std::vector<std::pair<std::string, Value>> test_fields;
};

struct FileObjectDeleter sealed {
  void operator()(FileObject* o) {
    delete o;
  }
};

inline std::shared_ptr<FileObject> MakeFileObject(const FileObjectInit& init) {
  return std::shared_ptr<FileObject>(new FileObject(init), FileObjectDeleter{});
}

}  // namespace binary_reader

#endif  // BINARY_READER_PUBLIC_FILE_OBJECT_INIT_H_
