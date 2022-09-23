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

namespace binary_reader {

namespace {

enum class BoolValue {
  None,
  False,
  True,
};

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
  uint16_t false_value;
  uint16_t true_value;
  OptionValue values[8];
};
static_assert(std::is_pod_v<OptionTypeInfo>, "Option data should be POD");

#define C(v) static_cast<uint16_t>(v)
const OptionTypeInfo kOptionData[] = {
    {OptionType::Signedness,
     &Cast<Signedness>,
     C(Signedness::Unsigned),
     C(Signedness::Signed),
     {
         {u"signed", C(Signedness::Signed)},
         {u"unsigned", C(Signedness::Unsigned)},
     }},
    {OptionType::ByteOrder,
     &Cast<ByteOrder>,
     0,
     0,
     {
         {u"big", C(ByteOrder::BigEndian)},
         {u"network", C(ByteOrder::BigEndian)},
         {u"little", C(ByteOrder::LittleEndian)},
     }},
};

}  // namespace

std::ostream& operator<<(std::ostream& os, OptionType opt) {
  switch (opt) {
    case OptionType::Unknown:
      return os << "Unknown";
    case OptionType::Signedness:
      return os << "signedness";
    case OptionType::ByteOrder:
      return os << "byte_order";
    default:
      return os << "<Unknown OptionType>";
  }
}

std::ostream& operator<<(std::ostream& os, Signedness opt) {
  switch (opt) {
    case Signedness::Signed:
      return os << "signed";
    case Signedness::Unsigned:
      return os << "unsigned";
    default:
      return os << "<Unknown Signedness>";
  }
}

std::ostream& operator<<(std::ostream& os, ByteOrder opt) {
  switch (opt) {
    case ByteOrder::LittleEndian:
      return os << "little";
    case ByteOrder::BigEndian:
      return os << "big";
    default:
      return os << "<Unknown ByteOrder>";
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
    : signedness(Signedness::Unsigned),
      byte_order(ByteOrder::BigEndian) {}

Options::Options(const Options&) = default;
Options::Options(Options&&) = default;
Options::~Options() = default;

Options& Options::operator=(const Options&) = default;
Options& Options::operator=(Options&&) = default;

Options::ParseResult Options::ParseOption(
    const std::unordered_set<OptionType>& types, const Value& value,
    OptionType* result_type, std::any* result) {
  const bool is_bool = value.value_type() == ValueType::Boolean;
  if (!is_bool && !value.is_string())
    return ParseResult::InvalidValueType;

  bool found = false;
  for (const auto& type_info : kOptionData) {
    if (!types.empty() && types.count(type_info.type) == 0)
      continue;

#define SET_VAL(v)                 \
  if (found)                       \
    return ParseResult::Ambiguous; \
  *result = type_info.cast(v);     \
  *result_type = type_info.type;   \
  found = true

    if (is_bool && type_info.false_value) {
      SET_VAL(value.as_bool() ? type_info.true_value : type_info.false_value);
    } else {
      if (value.as_string().AsUtf16() == u"true" && type_info.true_value) {
        SET_VAL(type_info.true_value);
      } else if (value.as_string().AsUtf16() == u"false" &&
                 type_info.true_value) {
        SET_VAL(type_info.false_value);
      } else {
        for (const auto& value_info : type_info.values) {
          if (!value_info.name)
            break;
          if (value.as_string().AsUtf16() == value_info.name) {
            SET_VAL(value_info.value);
            break;
          }
        }
      }
    }
#undef SET_VAL
  }

  if (found)
    return ParseResult::Success;
  if (is_bool)
    return ParseResult::NoBool;
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
    if ((info.false_value == 0) != (info.true_value == 0)) {
      std::cerr << "Inconsistent true/false values for type " << info.type
                << "\n";
      return false;
    }
    if (info.false_value != 0 && info.false_value == info.true_value) {
      std::cerr << "true/false values same for type " << info.type << "\n";
      return false;
    }

    // Don't allow duplicate names within a type, but names can be ambiguous
    // between types.
    std::unordered_set<std::u16string> value_names;
    for (const auto& value : info.values) {
      if (!value.name)
        break;
      if (value_names.count(value.name) != 0) {
        std::cerr << "Duplicate option value " << UtfString{value.name}.AsUtf8()
                  << " for type " << info.type << "\n";
        return false;
      }
      value_names.emplace(value.name);
    }
  }

  return true;
}

}  // namespace binary_reader
