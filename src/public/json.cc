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

#include "binary_reader/json.h"

namespace binary_reader {

struct JsonOptions::Impl {};

JsonOptions::JsonOptions() = default;
JsonOptions::~JsonOptions() = default;
JsonOptions::JsonOptions(JsonOptions&&) = default;
JsonOptions& JsonOptions::operator=(JsonOptions&&) = default;

// Need to explicitly list every field since std::unique_ptr isn't copyable.
JsonOptions::JsonOptions(const JsonOptions& other)
    : pretty(other.pretty), indent(other.indent) {}

JsonOptions& JsonOptions::operator=(const JsonOptions& other) {
  JsonOptions cp{other};
  std::swap(*this, cp);
  return *this;
}

void DumpJsonValue(std::ostream& os, const JsonOptions& opts, Value obj,
                   size_t indent) {
  switch (obj.value_type()) {
    case ValueType::Null:
      os << "null";
      break;
    case ValueType::Boolean:
      os << (obj.as_bool() ? "true" : "false");
      break;
    case ValueType::UnsignedInt:
      os << obj.as_unsigned();
      break;
    case ValueType::SignedInt:
      os << obj.as_signed();
      break;
    case ValueType::Double:
      os << obj.as_double();
      break;
    case ValueType::Object:
      DumpJsonObject(os, opts, obj.as_object(), indent);
      break;
    default:
      os << "<UNKNOWN_VALUE_TYPE>";
      break;
  }
}

void DumpJsonObject(std::ostream& os, const JsonOptions& opts,
                    std::shared_ptr<FileObject> obj, size_t indent) {
  bool first = true;
  os << "{";
  for (auto& pair : *obj) {
    if (!first)
      os << ",";
    if (opts.pretty)
      os << "\n" << std::string(indent + opts.indent, ' ');
    os << '"' << pair.first << "\":";
    if (opts.pretty)
      os << " ";
    DumpJsonValue(os, opts, pair.second, indent + opts.indent);
    first = false;
  }
  if (!first && opts.pretty)
    os << "\n" << std::string(indent, ' ');
  os << "}";
}

}  // namespace binary_reader
