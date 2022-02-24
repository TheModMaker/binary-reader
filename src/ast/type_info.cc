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

#include "ast/type_info.h"

namespace binary_reader {

TypeInfoBase::TypeInfoBase(const std::string& alias_name,
                           const std::string& base_name)
    : alias_name_(alias_name), base_name_(base_name) {}

TypeInfoBase::~TypeInfoBase() {}

std::vector<std::shared_ptr<TypeInfoBase>> TypeInfoBase::GetBuiltInTypes() {
  std::vector<std::shared_ptr<TypeInfoBase>> ret;
#define MAKE(id, size, sign)                                               \
  ret.emplace_back(std::make_shared<IntegerTypeInfo>((id), (size), (sign), \
                                                     ByteOrder::Unset))
  MAKE("byte", 8, Signedness::Unsigned);
  MAKE("sbyte", 8, Signedness::Signed);
  MAKE("int8", 8, Signedness::Signed);
  MAKE("uint8", 8, Signedness::Unsigned);
  MAKE("int16", 16, Signedness::Signed);
  MAKE("uint16", 16, Signedness::Unsigned);
  MAKE("int32", 32, Signedness::Signed);
  MAKE("uint32", 32, Signedness::Unsigned);
  MAKE("int64", 64, Signedness::Signed);
  MAKE("uint64", 64, Signedness::Unsigned);
#undef MAKE
  return ret;
}

bool TypeInfoBase::equals(const TypeInfoBase& other) const {
  return alias_name_ == other.alias_name_ && base_name_ == other.base_name_;
}

IntegerTypeInfo::IntegerTypeInfo(const std::string& alias_name, size_t size,
                                 Signedness sign, ByteOrder order)
    : TypeInfoBase(alias_name, "integer"),
      size_(size),
      sign_(sign),
      order_(order) {}

bool IntegerTypeInfo::equals(const TypeInfoBase& other) const {
  if (auto* o = dynamic_cast<const IntegerTypeInfo*>(&other)) {
    return size_ == o->size_ && sign_ == o->sign_ && order_ == o->order_ &&
           TypeInfoBase::equals(other);
  }
  return false;
}

}  // namespace binary_reader
