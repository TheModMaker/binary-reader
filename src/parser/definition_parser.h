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

#ifndef BINARY_READER_PARSER_DEFINITION_PARSER_H_
#define BINARY_READER_PARSER_DEFINITION_PARSER_H_

#include <memory>
#include <string>
#include <vector>

#include "ast/type_definition.h"
#include "binary_reader/error_collection.h"

namespace binary_reader {

bool ParseDefinitionFile(const std::string& path, const std::string& buffer,
                         std::vector<std::shared_ptr<TypeDefinition>>* defs,
                         ErrorCollection* errors);

}  // namespace binary_reader

#endif  // BINARY_READER_PARSER_DEFINITION_PARSER_H_
