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

#ifndef BINARY_READER_AST_OPTION_SET_H_
#define BINARY_READER_AST_OPTION_SET_H_

#include <unordered_set>
#include <vector>

#include "binary_reader/error_collection.h"
#include "binary_reader/options.h"
#include "binary_reader/utf_string.h"

namespace binary_reader {

/// <summary>
/// Holds a collection of opton values set in a definition file.
/// </summary>
class OptionSet final {
 public:
  bool AddStatic(DebugInfo debug, OptionType type, const UtfString& value,
                 ErrorCollection* errors);

  bool BuildStaticOptions(const std::unordered_set<OptionType>& valid_options,
                          Options* options, ErrorCollection* errors) const;

 private:
  struct OptionValue {
    DebugInfo debug;
    OptionType type;

    // Static options
    UtfString static_value;
  };

  std::vector<OptionValue> options_;
  std::unordered_set<OptionType> option_types_;
};

}  // namespace binary_reader

#endif  // BINARY_READER_AST_OPTION_SET_H_
