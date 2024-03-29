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

#ifndef BINARY_READER_APP_COMMAND_LINE_H_
#define BINARY_READER_APP_COMMAND_LINE_H_

#include <string>

#include "binary_reader/file_parser.h"
#include "binary_reader/json.h"

namespace binary_reader {

class CommandLine final {
 public:
  bool Parse(const char** argv, int argc);

  std::string definition_path;
  std::string binary_path;
  std::string type_name;

  FileParserOptions parser_options;
  JsonOptions json_options;
};

}  // namespace binary_reader

#endif  // BINARY_READER_APP_COMMAND_LINE_H_
