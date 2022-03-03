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

#include "public/file_object_init.h"

namespace binary_reader {

struct FileObject::Impl {
  FileObjectInit init;
};

struct FileObject::const_iterator::IteratorState {
  const FileObject* obj = nullptr;
  decltype(FileObjectInit::fields)::iterator iter;
};

FileObject::const_iterator::const_iterator(const_iterator&&) = default;
FileObject::const_iterator& FileObject::const_iterator::operator=(
    const_iterator&&) = default;
FileObject::const_iterator::~const_iterator() = default;

FileObject::const_iterator::const_iterator() : impl_(new IteratorState) {}

FileObject::const_iterator::const_iterator(const const_iterator& other)
    : impl_(new IteratorState) {
  impl_->obj = other.impl_->obj;
  impl_->iter = other.impl_->iter;
}

FileObject::const_iterator& FileObject::const_iterator::operator=(
    const const_iterator& other) {
  const_iterator cp{other};
  std::swap(impl_, cp.impl_);
  return *this;
}

FileObject::const_iterator::reference FileObject::const_iterator::operator*()
    const {
  return *impl_->iter;
}

FileObject::const_iterator::pointer FileObject::const_iterator::operator->()
    const {
  return &*impl_->iter;
}

FileObject::const_iterator& FileObject::const_iterator::operator++() {
  ++impl_->iter;
  return *this;
}

FileObject::const_iterator FileObject::const_iterator::operator++(int) {
  const_iterator cp{*this};
  ++*this;
  return cp;
}

bool FileObject::const_iterator::operator==(const const_iterator& other) const {
  return impl_ && other.impl_ && impl_->obj == other.impl_->obj &&
         impl_->iter == other.impl_->iter;
}

bool FileObject::const_iterator::operator!=(const const_iterator& other) const {
  return !(*this == other);
}

FileObject::const_iterator::const_iterator(std::unique_ptr<IteratorState> state)
    : impl_(std::move(state)) {}

FileObject::const_iterator FileObject::begin() const {
  std::unique_ptr<const_iterator::IteratorState> state{
      new const_iterator::IteratorState};
  state->obj = this;
  state->iter = impl_->init.fields.begin();
  return const_iterator{std::move(state)};
}

FileObject::const_iterator FileObject::end() const {
  std::unique_ptr<const_iterator::IteratorState> state{
      new const_iterator::IteratorState};
  state->obj = this;
  state->iter = impl_->init.fields.end();
  return const_iterator{std::move(state)};
}

FileObject::const_iterator FileObject::find(const std::string& name) const {
  std::unique_ptr<const_iterator::IteratorState> state{
      new const_iterator::IteratorState};
  state->obj = this;
  for (state->iter = impl_->init.fields.begin();
       state->iter != impl_->init.fields.end() && state->iter->first != name;
       state->iter++)
    ;
  return const_iterator{std::move(state)};
}

bool FileObject::HasField(const std::string& name) const {
  return find(name) != end();
}

Value FileObject::GetFieldValue(const std::string& name) const {
  auto it = find(name);
  return it != end() ? it->second : Value{};
}

FileObject::FileObject(const FileObjectInit& init_data) : impl_(new Impl) {
  impl_->init = init_data;
}
FileObject::~FileObject() {}

}  // namespace binary_reader
