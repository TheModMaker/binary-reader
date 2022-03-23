// Copyright 2021 Google LLC
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

#include <iostream>
#include <type_traits>

#include "app/command_line.h"
#include "binary_reader/file_parser.h"
#include "binary_reader/json.h"

using namespace binary_reader;

int main(int argc, const char** argv) {
  CommandLine cmd;
  if (!cmd.Parse(argv, argc))
    return 1;

  ErrorCollection errors;
  auto parser = FileParser::CreateFromFile(cmd.definition_path,
                                           cmd.parser_options, &errors);
  for (auto& err : errors)
    std::cerr << err << "\n";
  if (!parser)
    return 1;

  errors.clear();
  auto file = parser->ParseFile(cmd.binary_path, cmd.type_name, &errors);
  for (auto& err : errors)
    std::cerr << err << "\n";
  if (!file)
    return 1;

  DumpJsonObject(std::cout, cmd.json_options, file);
  return 0;
}
