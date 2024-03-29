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
JsonOptions::JsonOptions(const JsonOptions& other) = default;
JsonOptions::JsonOptions(JsonOptions&&) = default;
JsonOptions& JsonOptions::operator=(const JsonOptions& other) = default;
JsonOptions& JsonOptions::operator=(JsonOptions&&) = default;

void DumpJsonValue(std::ostream& os, const JsonOptions& opts, Value obj,
                   size_t indent) {
  switch (obj.value_type()) {
    case ValueType::Null:
      os << "null";
      break;
    case ValueType::Number:
      os << obj.as_number();
      break;
    case ValueType::String:
      os << "\"" << obj.as_string() << "\"";
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
  for (auto& name : obj->GetFieldNames()) {
    if (!first)
      os << ",";
    if (opts.pretty)
      os << "\n" << std::string(indent + opts.indent, ' ');
    os << '"' << name << "\":";
    if (opts.pretty)
      os << " ";
    DumpJsonValue(os, opts, obj->GetFieldValue(name), indent + opts.indent);
    first = false;
  }
  if (!first && opts.pretty)
    os << "\n" << std::string(indent, ' ');
  os << "}";
  if (opts.pretty && indent == 0)
    os << "\n";
}

}  // namespace binary_reader
