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

#include "binary_reader/file_object.h"

#include <cassert>
#include <limits>
#include <optional>
#include <unordered_map>

#include "ast/field_info.h"
#include "public/file_object_init.h"

namespace binary_reader {

namespace {

struct ParsedFieldInfo {
  std::string name;
  std::shared_ptr<TypeInfoBase> type;
  Size offset;
  std::optional<Value> value;
};

}  // namespace

struct FileObject::Impl {
  FileObjectInit init;
  std::vector<ParsedFieldInfo> parsed_fields;
  std::unordered_map<std::string, size_t> field_name_map;
};

std::vector<std::string> FileObject::GetFieldNames() const {
  std::vector<std::string> ret;
  ret.reserve(impl_->field_name_map.size());
  for (auto& pair : impl_->field_name_map) {
    ret.emplace_back(pair.first);
  }
  return ret;
}

bool FileObject::HasField(const std::string& name) const {
  return impl_->field_name_map.count(name) != 0;
}

Value FileObject::GetFieldValue(const std::string& name) const {
  Value ret;
  ErrorCollection errors;
  (void)GetFieldValue(name, &ret, &errors);
  return ret;
}

bool FileObject::GetFieldValue(const std::string& name, Value* value,
                               ErrorCollection* errors) const {
  if (impl_->field_name_map.count(name) == 0) {
    *value = Value{};
    return true;
  }

  if (!EnsureField(impl_->field_name_map[name], errors))
    return false;
  *value = *impl_->parsed_fields[impl_->field_name_map[name]].value;
  return true;
}

void FileObject::ClearCache() {
  for (auto& info : impl_->parsed_fields) {
    info.value.reset();
  }
}

bool FileObject::ReparseObject(ErrorCollection* errors) {
  if (!impl_->init.type)
    return true;

  // TODO: This object should be invalid if the parent is reparsed.
  impl_->parsed_fields.clear();
  impl_->field_name_map.clear();
  impl_->parsed_fields.reserve(impl_->init.type->statements().size());
  impl_->field_name_map.reserve(impl_->init.type->statements().size());

  Size offset = impl_->init.start_position;
  for (auto stmt : impl_->init.type->statements()) {
    if (auto field = std::dynamic_pointer_cast<FieldInfo>(stmt)) {
      impl_->parsed_fields.push_back(
          {field->name(), field->type(), offset, {}});
      impl_->field_name_map[field->name()] = impl_->parsed_fields.size() - 1;

      if (!field->type()->static_size().has_value()) {
        errors->Add({field->type()->debug_info(),
                    ErrorKind::FieldsMustBeStatic});
        return false;
      }
      offset += *field->type()->static_size();
    } else {
      errors->Add({{}, ErrorKind::Unknown});
      return false;
    }
  }
  return true;
}

bool FileObject::EnsureField(size_t index, ErrorCollection* errors) const {
  auto& info = impl_->parsed_fields[index];
  if (info.value.has_value())
    return true;

  if (!impl_->init.file->Seek(info.offset, errors))
    return false;
  Value temp;
  if (!info.type->ReadValue(impl_->init.file, &temp, errors))
    return false;
  info.value = temp;
  return true;
}

FileObject::FileObject(const FileObjectInit& init_data) : impl_(new Impl) {
  assert(init_data.test_fields.empty() || !init_data.file);

  impl_->init = init_data;

  impl_->parsed_fields.resize(impl_->init.test_fields.size());
  for (size_t i = 0; i < impl_->init.test_fields.size(); i++) {
    impl_->parsed_fields[i].name = impl_->init.test_fields[i].first;
    impl_->parsed_fields[i].value = impl_->init.test_fields[i].second;
    impl_->field_name_map[impl_->init.test_fields[i].first] = i;
  }
}

FileObject::~FileObject() {}

}  // namespace binary_reader
