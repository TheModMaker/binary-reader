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

#include "binary_reader/options.h"

#include <iostream>

#include "binary_reader/value.h"

namespace binary_reader {

namespace {

template <typename T>
std::any Cast(uint16_t val) {
  return static_cast<T>(val);
}

struct OptionValue {
  const char16_t* name;
  uint16_t value;
};

struct OptionTypeInfo {
  OptionType type;
  std::any (*cast)(uint16_t);
  OptionValue values[8];
};
static_assert(std::is_pod_v<OptionTypeInfo>, "Option data should be POD");

#define C(v) static_cast<uint16_t>(v)
const OptionTypeInfo kOptionData[] = {
    {OptionType::Signedness,
     &Cast<Signedness>,
     {
         {u"signed", C(Signedness::Signed)},
         {u"unsigned", C(Signedness::Unsigned)},
     }},
    {OptionType::ByteOrder,
     &Cast<ByteOrder>,
     {
         {u"big", C(ByteOrder::BigEndian)},
         {u"network", C(ByteOrder::BigEndian)},
         {u"little", C(ByteOrder::LittleEndian)},
     }},
};
#undef C

Options MakeDefault() {
  Options opt;
  opt.byte_order = ByteOrder::BigEndian;
  opt.signedness = Signedness::Unsigned;
  return opt;
}

}  // namespace

std::string to_string(OptionType type) {
  switch (type) {
    case OptionType::Unknown:
      return "Unknown";
    case OptionType::Signedness:
      return "signedness";
    case OptionType::ByteOrder:
      return "byte_order";
    default:
      return "<Unknown OptionType>";
  }
}

std::string to_string(Signedness signedness) {
  switch (signedness) {
    case Signedness::Signed:
      return "signed";
    case Signedness::Unsigned:
      return "unsigned";
    default:
      return "<Unknown Signedness>";
  }
}

std::string to_string(ByteOrder byte_order) {
  switch (byte_order) {
    case ByteOrder::LittleEndian:
      return "little";
    case ByteOrder::BigEndian:
      return "big";
    default:
      return "<Unknown ByteOrder>";
  }
}

OptionType GetOptionType(const UtfString& type) {
  if (type.AsUtf16() == u"signedness") {
    return OptionType::Signedness;
  } else if (type.AsUtf16() == u"byte_order" || type.AsUtf16() == u"order") {
    return OptionType::ByteOrder;
  } else {
    return OptionType::Unknown;
  }
}

Options::Options()
    : signedness(Signedness::Unset),
      byte_order(ByteOrder::Unset) {}

Options::Options(const Options&) = default;
Options::Options(Options&&) = default;
Options::~Options() = default;

Options& Options::operator=(const Options&) = default;
Options& Options::operator=(Options&&) = default;

const Options Options::DefaultOptions = MakeDefault();

Options::ParseResult Options::ParseOption(
    const std::unordered_set<OptionType>& types, const Value& value,
    OptionType* result_type, std::any* result) {
  if (value.value_type() != ValueType::String)
    return ParseResult::InvalidValueType;

  bool found = false;
  for (const auto& type_info : kOptionData) {
    if (!types.empty() && types.count(type_info.type) == 0)
      continue;

    for (const auto& value_info : type_info.values) {
      if (!value_info.name)
        break;
      if (value.as_string().AsUtf16() == value_info.name) {
        if (found)
          return ParseResult::Ambiguous;
        *result_type = type_info.type;
        *result = type_info.cast(value_info.value);
        found = true;
        break;
      }
    }
  }

  if (found)
    return ParseResult::Success;
  return ParseResult::UnknownString;
}

std::any Options::GetOption(OptionType type, const Options& defaults) const {
  switch (type) {
    case OptionType::Signedness:
      if (signedness == Signedness::Unset)
        return defaults.signedness;
      else
        return signedness;
    case OptionType::ByteOrder:
      if (byte_order == ByteOrder::Unset)
        return defaults.byte_order;
      else
        return byte_order;
    default:
      return {};
  }
}

bool Options::SetOption(OptionType type, std::any value) {
  switch (type) {
    case OptionType::Signedness:
      if (auto* ptr = std::any_cast<Signedness>(&value)) {
        signedness = *ptr;
        return true;
      } else {
        return false;
      }
    case OptionType::ByteOrder:
      if (auto* ptr = std::any_cast<ByteOrder>(&value)) {
        byte_order = *ptr;
        return true;
      } else {
        return false;
      }
    default:
      return false;
  }
}

bool Options::CheckOptionData() {
  std::unordered_set<OptionType> types;
  for (const auto& info : kOptionData) {
    if (types.count(info.type) != 0) {
      std::cerr << "Duplicate option type " << info.type << "\n";
      return false;
    }
    types.emplace(info.type);

    // Don't allow duplicate names within a type, but names can be ambiguous
    // between types.
    std::unordered_set<std::u16string> value_names;
    for (const auto& value : info.values) {
      if (!value.name)
        break;
      if (value_names.count(value.name) != 0) {
        std::cerr << "Duplicate option value " << UtfString{value.name}
                  << " for type " << info.type << "\n";
        return false;
      }
      value_names.emplace(value.name);
    }
  }

  return true;
}

}  // namespace binary_reader
