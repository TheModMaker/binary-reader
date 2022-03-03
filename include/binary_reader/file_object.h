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

#ifndef BINARY_READER_INCLUDE_FILE_OBJECT_H_
#define BINARY_READER_INCLUDE_FILE_OBJECT_H_

#include <algorithm>
#include <memory>
#include <string>
#include <utility>

#include "binary_reader/value.h"

namespace binary_reader {

struct FileObjectInit;

/// <summary>
/// Defines a parsed object from a file.  This is an instance of a type that is
/// defined in the file definition.  An instance of this type only covers one
/// occurance of the object within the file; this may only cover a small part of
/// the binary file.
///
/// Instances of this type can only be created by the file parser and not by the
/// app.  Instance lifetime is controlled through a std::shared_ptr.
///
/// This type is iterable, and iterates over the fields in the object.
/// The order of elements is the order that they appear in the file definition.
/// The iterator's value is a pair of string field name and the Value.
/// </summary>
class FileObject sealed {
 public:
  class const_iterator sealed {
   public:
    using iterator_category = std::input_iterator_tag;
    using value_type = std::pair<std::string, Value>;
    using difference_type = ptrdiff_t;
    using pointer = const std::pair<std::string, Value>*;
    using reference = const std::pair<std::string, Value>&;

    const_iterator();
    const_iterator(const const_iterator&);
    const_iterator(const_iterator&&);
    ~const_iterator();
    const_iterator& operator=(const const_iterator&);
    const_iterator& operator=(const_iterator&&);

    reference operator*() const;
    pointer operator->() const;
    const_iterator& operator++();
    const_iterator operator++(int);
    bool operator==(const const_iterator& other) const;
    bool operator!=(const const_iterator& other) const;

   private:
    struct IteratorState;
    friend FileObject;
    explicit const_iterator(std::unique_ptr<IteratorState> state);

    std::unique_ptr<IteratorState> impl_;
  };

  FileObject(const FileObject&) = delete;
  FileObject(FileObject&&) = delete;
  FileObject& operator=(const FileObject&) = delete;
  FileObject& operator=(FileObject&&) = delete;

  const_iterator begin() const;
  const_iterator end() const;

  /// <summary>
  /// Returns an iterator to the field with the given name, or end() if the
  /// given field doesn't exist.
  /// </summary>
  /// <param name="name">The name of the field.</param>
  const_iterator find(const std::string& name) const;

  /// <summary>
  /// Returns whether the given field exists within the object.  This return
  /// false for fields that aren't assigned a value due to a conditional branch.
  /// </summary>
  /// <param name="name">The name of the field to get.</param>
  bool HasField(const std::string& name) const;
  /// <summary>
  /// Returns the value of the given field, or null if the field doesn't exist.
  /// </summary>
  /// <param name="name">The name of the field to get.</param>
  Value GetFieldValue(const std::string& name) const;

 private:
  friend std::shared_ptr<FileObject> MakeFileObject(const FileObjectInit&);
  friend struct FileObjectDeleter;
  FileObject(const FileObjectInit& init_data);
  ~FileObject();

  struct Impl;
  std::unique_ptr<Impl> impl_;
};

}  // namespace binary_reader

#endif  // BINARY_READER_INCLUDE_FILE_OBJECT_H_
