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

#include <memory>
#include <string>
#include <vector>

#include "binary_reader/error_collection.h"
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
/// This lazy-loads the fields in the object.  The field is only parsed when
/// requesting the field's value.  The cached values can be cleared to reduce
/// memory usage.
/// </summary>
class FileObject sealed {
 public:
  FileObject(const FileObject&) = delete;
  FileObject(FileObject&&) = delete;
  FileObject& operator=(const FileObject&) = delete;
  FileObject& operator=(FileObject&&) = delete;

  std::vector<std::string> GetFieldNames() const;

  /// <summary>
  /// Returns whether the given field exists within the object.  This return
  /// false for fields that aren't assigned a value due to a conditional branch.
  /// </summary>
  /// <param name="name">The name of the field to get.</param>
  bool HasField(const std::string& name) const;

  /// <summary>
  /// Returns the value of the given field, or null if the field doesn't exist.
  /// This also returns null if an error occurs.
  /// </summary>
  /// <param name="name">The name of the field to get.</param>
  Value GetFieldValue(const std::string& name) const;
  /// <summary>
  /// Gets the value of the given field.  This returns success and uses null if
  /// the field doesn't exist.
  /// </summary>
  /// <param name="name">The name of the field to get.</param>
  /// <param name="value">Will be filled with the value.</param>
  /// <param name="errors">Will be filled with any errors that happen.</param>
  /// <returns>True on success, false on error.</returns>
  bool GetFieldValue(const std::string& name, Value* value,
                     ErrorCollection* errors) const;

  /// <summary>
  /// Erases any cached values stored by this object.  Fields will need to be
  /// parsed again when getting their values.  Note that if the underlying file
  /// was changed, this won't update field types/layout, only their values.  So
  /// if a conditional branch changed, the field values will be incorrect.  Use
  /// ReparseObject to update conditional branches.
  /// </summary>
  void ClearCache();

  /// <summary>
  /// Reparses the object to determine which fields exist.  This also clears
  /// any existing cached values.  Note that if a parent object is reparsed,
  /// this instance is no longer valid; this function invalidates any child
  /// FileObject.
  /// </summary>
  /// <param name="errors">Will be filled with any errors that happen.</param>
  /// <returns>True on success, false on error.</returns>
  bool ReparseObject(ErrorCollection* errors);

 private:
  friend std::shared_ptr<FileObject> MakeFileObject(const FileObjectInit&);
  friend struct FileObjectDeleter;
  FileObject(const FileObjectInit& init_data);
  ~FileObject();

  /// <summary>
  /// Ensures the field at the given index has been parsed and its value is
  /// cached.
  /// </summary>
  bool EnsureField(size_t index, ErrorCollection* errors) const;

  struct Impl;
  std::unique_ptr<Impl> impl_;
};

}  // namespace binary_reader

#endif  // BINARY_READER_INCLUDE_FILE_OBJECT_H_
