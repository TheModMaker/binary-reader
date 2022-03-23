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

#include "app/command_line.h"

#include <tclap/CmdLine.h>

#include <iostream>

namespace binary_reader {

bool CommandLine::Parse(const char** argv, int argc) {
  try {
    TCLAP::CmdLine cmd("Reads and processes binary files", ' ', "v0.1");
    TCLAP::ValueArg<std::string> definition(
        "f", "format", "The path to the binary file definition", true, "",
        "path", cmd);
    TCLAP::ValueArg<std::string> input(
        "i", "input", "The path to the binary file", true, "", "path", cmd);
    TCLAP::ValueArg<std::string> type(
        "t", "type", "The root type name to parse", false, "", "name", cmd);

    cmd.parse(argc, argv);

    definition_path = definition.getValue();
    binary_path = input.getValue();
    type_name = type.getValue();
    return true;
  } catch (TCLAP::ArgException& e) {
    std::cout << "FATAL ERROR: " << e.error() << " for arg " << e.argId()
              << "\n";
    return false;
  }
}

}  // namespace binary_reader
