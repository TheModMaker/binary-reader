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

#ifndef BINARY_READER_INCLUDE_CLONEABLE_UNIQUE_PTR_H_
#define BINARY_READER_INCLUDE_CLONEABLE_UNIQUE_PTR_H_

#include <memory>

namespace binary_reader {

/// <summary>
/// A type similar to std::unique_ptr<T> except this is copyable and will clone
/// the object on copy.  This always uses new/delete.
/// </summary>
template <typename T>
class cloneable_unique_ptr sealed {
 public:
  cloneable_unique_ptr() : ptr_(nullptr) {}
  explicit cloneable_unique_ptr(T* value) : ptr_(value) {}
  template <typename U>
  cloneable_unique_ptr(std::unique_ptr<U> value)
      : ptr_(value.release()) {}
  ~cloneable_unique_ptr() {
    reset();
  }

  cloneable_unique_ptr(const cloneable_unique_ptr<T>& other)
      : ptr_(other.ptr_ ? new T(*other.ptr_) : nullptr) {}
  cloneable_unique_ptr(cloneable_unique_ptr<T>&& other) noexcept
      : ptr_(other.ptr_) {
    other.ptr_ = nullptr;
  }
  cloneable_unique_ptr<T>& operator=(const cloneable_unique_ptr<T>& other) {
    reset();
    if (other.ptr_)
      ptr_ = new T(*other.ptr_);
    return *this;
  }
  cloneable_unique_ptr<T>& operator=(cloneable_unique_ptr<T>&& other) noexcept {
    reset();
    ptr_ = other.ptr_;
    other.ptr_ = nullptr;
    return *this;
  }

  explicit operator bool() const {
    return ptr_ != nullptr;
  }
  T* operator->() const {
    return ptr_;
  }
  T& operator*() const {
    return *ptr_;
  }

  T* get() const {
    return ptr_;
  }

  void reset(T* ptr = nullptr) {
    delete ptr_;
    ptr_ = ptr;
  }

 private:
  T* ptr_;
};

}  // namespace binary_reader

#endif  // BINARY_READER_INCLUDE_CLONEABLE_UNIQUE_PTR_H_
