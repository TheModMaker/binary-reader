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

#include <algorithm>

namespace binary_reader {

TypeInfoBase::TypeInfoBase(const DebugInfo& debug,
                           const std::string& alias_name,
                           const std::string& base_name,
                           std::optional<Size> static_size)
    : alias_name_(alias_name),
      base_name_(base_name),
      static_size_(static_size),
      debug_(debug) {}

TypeInfoBase::~TypeInfoBase() {}

std::vector<std::shared_ptr<TypeInfoBase>> TypeInfoBase::GetBuiltInTypes() {
  std::vector<std::shared_ptr<TypeInfoBase>> ret;
#define MAKE(id, size, sign)                                      \
  ret.emplace_back(std::make_shared<IntegerTypeInfo>(             \
      DebugInfo{"<builtin>"}, (id), Size::FromBits(size), (sign), \
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

bool TypeInfoBase::Equals(const TypeInfoBase& other) const {
  return alias_name_ == other.alias_name_ && base_name_ == other.base_name_ &&
         static_size_ == other.static_size_;
}


IntegerTypeInfo::IntegerTypeInfo(const DebugInfo& debug,
                                 const std::string& alias_name, Size size,
                                 Signedness sign, ByteOrder order)
    : TypeInfoBase(debug, alias_name, "integer", size),
      sign_(sign),
      order_(order) {}

std::unordered_set<OptionType> IntegerTypeInfo::GetOptionTypes() const {
  return {OptionType::Signedness, OptionType::ByteOrder};
}

std::shared_ptr<TypeInfoBase> IntegerTypeInfo::Instantiate(
    const DebugInfo& debug, Options options) const {
  return std::make_shared<IntegerTypeInfo>(
      debug, alias_name(), static_size().value(),
      options.GetOption<Signedness>(signedness()),
      options.GetOption<ByteOrder>(byte_order()));
}

bool IntegerTypeInfo::Equals(const TypeInfoBase& other) const {
  auto* o = static_cast<const IntegerTypeInfo*>(&other);
  return sign_ == o->sign_ && order_ == o->order_ &&
         TypeInfoBase::Equals(other);
}

bool IntegerTypeInfo::ReadValue(std::shared_ptr<BufferedFileReader> reader,
                                Value* result, ErrorCollection* errors) const {
  const size_t size = static_size()->bit_count();
  const uint8_t bit_offset = reader->position().bit_offset();
  const size_t final_bits = (reader->position().bit_offset() + size) % 8;
  const size_t byte_count = (bit_offset + size) / 8 + (final_bits != 0 ? 1 : 0);
  if (order_ == ByteOrder::LittleEndian &&
      (bit_offset != 0 || final_bits != 0)) {
    errors->Add({debug_info(), ErrorKind::LittleEndianAlign, ErrorLevel::Error,
                 reader->position().byte_count()});
    return false;
  }
  if (!reader->EnsureBuffer(*static_size(), errors))
    return false;

  const uint8_t* buffer;
  size_t buffer_size;
  if (!reader->GetBuffer(&buffer, &buffer_size, errors))
    return false;
  if (buffer_size < byte_count) {
    errors->Add({debug_info(), ErrorKind::UnexpectedEndOfStream});
    return false;
  }

  uint64_t value = 0;
  size_t index = 0;
  if (bit_offset != 0) {
    // Read most significant bits to least.
    const size_t mask = (1ull << (8 - bit_offset)) - 1;
    const size_t shift = 8 - std::min<size_t>(size + bit_offset, 8);
    value = (buffer[0] & mask) >> shift;
    index++;
  }
  for (; index < byte_count - 1 || (index == byte_count - 1 && final_bits == 0);
       index++) {
    if (order_ == ByteOrder::LittleEndian) {
      value |= static_cast<uint64_t>(buffer[index]) << (8u * index);
    } else {
      value <<= 8;
      value |= buffer[index];
    }
  }

  if (final_bits != 0 && byte_count != 1) {
    value <<= final_bits;
    value |= (buffer[index] >> final_bits);
  }

  if (sign_ == Signedness::Signed && (value & (1ull << (size - 1)))) {
    // If the value is negative, then fill remaining high-level bits with 1
    // so the value is still negative.
    if (size != 64)
      value |= ~((1ull << size) - 1);
    *result = Number{static_cast<int64_t>(value)};
  } else {
    *result = Number{value};
  }
  return reader->Skip(Size::FromBits(size), errors);
}

}  // namespace binary_reader
