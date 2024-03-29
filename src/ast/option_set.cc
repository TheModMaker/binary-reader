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

#include "option_set.h"

#include "binary_reader/value.h"

#ifdef _MSC_VER
// string_view from temporary is OK since it will live until the function
// returns
#  pragma warning(disable:26449)
#endif

namespace binary_reader {

namespace {

bool AddOption(OptionType type, const Value& value, const DebugInfo& debug,
               const std::unordered_set<OptionType>& valid_options,
               Options* options, ErrorCollection* errors) {
  OptionType real_type;
  std::any real_value;
  Options::ParseResult parse_result;
  if (type == OptionType::Unknown) {
    parse_result =
        Options::ParseOption(valid_options, value, &real_type, &real_value);
  } else {
    parse_result = Options::ParseOption({type}, value, &real_type, &real_value);
  }
  switch (parse_result) {
    case Options::ParseResult::Success:
      break;
    case Options::ParseResult::InvalidValueType:
      if (type == OptionType::Unknown) {
        errors->Add({debug, ErrorKind::OptionMustBeString});
      } else {
        errors->Add(
            {debug, ErrorKind::OptionMustBeStringTyped, {to_string(type)}});
      }
      return false;
    case Options::ParseResult::UnknownString:
      if (type == OptionType::Unknown) {
        errors->Add({debug,
                     ErrorKind::UnknownOptionValue,
                     {value.as_string().AsUtf8()}});
      } else {
        errors->Add({debug,
                     ErrorKind::UnknownOptionValueTyped,
                     {value.as_string().AsUtf8(), to_string(type)}});
      }
      return false;
    case Options::ParseResult::Ambiguous:
      // Explicit types cannot be ambiguous since there's only one type possible
      errors->Add(
          {debug, ErrorKind::AmbiguousOption, {value.as_string().AsUtf8()}});
      return false;
  }
  if (!valid_options.empty() && valid_options.count(real_type) == 0) {
    errors->Add(
        {debug, ErrorKind::OptionInvalidForType, {to_string(real_type)}});
    return false;
  }

  if (!options->SetOption(real_type, real_value)) {
    errors->Add({debug, ErrorKind::Unknown});
    return false;
  }
  return true;
}

}  // namespace

bool OptionSet::AddStatic(DebugInfo debug, OptionType type,
                          const UtfString& value, ErrorCollection* errors) {
  if (type != OptionType::Unknown) {
    if (option_types_.count(type) > 0) {
      errors->Add({debug, ErrorKind::DuplicateOption, {to_string(type)}});
      return false;
    }
    option_types_.emplace(type);
  }
  options_.push_back({debug, type, value});
  return true;
}

bool OptionSet::BuildStaticOptions(
    const std::unordered_set<OptionType>& valid_options, Options* options,
    ErrorCollection* errors) const {
  bool ret = true;
  for (auto& opt : options_) {
    if (!AddOption(opt.type, Value(opt.static_value), opt.debug, valid_options,
                   options, errors)) {
      ret = false;
    }
  }
  return ret;
}

}  // namespace binary_reader
