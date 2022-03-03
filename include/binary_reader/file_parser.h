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

#ifndef BINARY_READER_INCLUDE_FILE_PARSER_H_
#define BINARY_READER_INCLUDE_FILE_PARSER_H_

#include <memory>
#include <string>
#include <vector>

#include "binary_reader/error.h"
#include "binary_reader/file_object.h"
#include "binary_reader/file_system.h"

namespace binary_reader {

/// <summary>
/// Options to control how the file parsing is handled. These cannot be changed
/// once the FileParser is created.
/// </summary>
class FileParserOptions sealed {
 public:
  FileParserOptions();
  ~FileParserOptions();
  FileParserOptions(const FileParserOptions&);
  FileParserOptions(FileParserOptions&&);
  FileParserOptions& operator=(const FileParserOptions&);
  FileParserOptions& operator=(FileParserOptions&&);

  /// <summary>
  /// The FileSystem object used to read files from.  This is used for both
  /// binary files and definition files.  This can be <code>nullptr</code> to
  /// use the default implementation.
  /// </summary>
  std::shared_ptr<FileSystem> file_system;

 private:
  // For forward compatibility. This allows adding new options through member
  // methods since we can't add fields without breaking ABI.
  struct Impl;
  std::unique_ptr<Impl> impl_;
};

/// <summary>
/// This can be used to parse binary files.  This uses a binary definition file
/// to describe the format and uses that to read the file.
/// </summary>
class FileParser sealed {
 public:
  FileParser(const FileParser&) = delete;
  FileParser(FileParser&&) = delete;
  FileParser& operator=(const FileParser&) = delete;
  FileParser& operator=(FileParser&&) = delete;

  /// <summary>
  /// Reads the definition file at the given path to create a parser.
  /// </summary>
  /// <param name="path">The path to the file to read.</param>
  /// <param name="options">The options to control parsing.</param>
  /// <param name="errors">A list to hold the errors that happen.</param>
  /// <returns>The resulting file parser, or nullptr on error.</returns>
  static std::shared_ptr<FileParser> CreateFromFile(const std::string& path);
  static std::shared_ptr<FileParser> CreateFromFile(
      const std::string& path, const FileParserOptions& options);
  static std::shared_ptr<FileParser> CreateFromFile(
      const std::string& path, std::vector<ErrorInfo>* errors);
  static std::shared_ptr<FileParser> CreateFromFile(
      const std::string& path, const FileParserOptions& options,
      std::vector<ErrorInfo>* errors);

  /// <summary>
  /// Reads the given string as a definition file.
  /// </summary>
  /// <param name="def">The contents of the definition file.</param>
  /// <param name="path">The path of the file, only for error messages.</param>
  /// <param name="options">The options to control parsing.</param>
  /// <param name="errors">A list to hold the errors that happen.</param>
  /// <returns>The resulting file parser, or nullptr on error.</returns>
  static std::shared_ptr<FileParser> CreateFromDefinition(
      const std::string& def);
  static std::shared_ptr<FileParser> CreateFromDefinition(
      const std::string& def, const FileParserOptions& options);
  static std::shared_ptr<FileParser> CreateFromDefinition(
      const std::string& def, std::vector<ErrorInfo>* errors);
  static std::shared_ptr<FileParser> CreateFromDefinition(
      const std::string& def, const FileParserOptions& options,
      std::vector<ErrorInfo>* errors);
  static std::shared_ptr<FileParser> CreateFromDefinition(
      const std::string& def, const std::string& path);
  static std::shared_ptr<FileParser> CreateFromDefinition(
      const std::string& def, const std::string& path,
      const FileParserOptions& options);
  static std::shared_ptr<FileParser> CreateFromDefinition(
      const std::string& def, const std::string& path,
      std::vector<ErrorInfo>* errors);
  static std::shared_ptr<FileParser> CreateFromDefinition(
      const std::string& def, const std::string& path,
      const FileParserOptions& options, std::vector<ErrorInfo>* errors);

  /// <summary>
  /// Returns the current options used to parse files.
  /// </summary>
  const FileParserOptions& options() const;

  /// <summary>
  /// Returns the names of the top-level types in the definition file.
  /// </summary>
  std::vector<std::string> GetTypeNames() const;

  /// <summary>
  /// Parses the given binary file and creates a FileObject for it.
  /// </summary>
  /// <param name="path">The path to the binary file to read.</param>
  /// <param name="type">The type name to use as the whole file.</param>
  /// <param name="reader">The object to read the definition file from.</param>
  /// <param name="errors">A list to hold the errors that happen.</param>
  /// <returns>The parsed FileObject, or nullptr on error.</returns>
  std::shared_ptr<FileObject> ParseFile(const std::string& path);
  std::shared_ptr<FileObject> ParseFile(const std::string& path,
                                        std::vector<ErrorInfo>* errors);
  std::shared_ptr<FileObject> ParseFile(std::shared_ptr<FileReader> reader);
  std::shared_ptr<FileObject> ParseFile(std::shared_ptr<FileReader> reader,
                                        std::vector<ErrorInfo>* errors);
  std::shared_ptr<FileObject> ParseFile(const std::string& path,
                                        const std::string& type);
  std::shared_ptr<FileObject> ParseFile(const std::string& path,
                                        const std::string& type,
                                        std::vector<ErrorInfo>* errors);
  std::shared_ptr<FileObject> ParseFile(std::shared_ptr<FileReader> reader,
                                        const std::string& type);
  std::shared_ptr<FileObject> ParseFile(std::shared_ptr<FileReader> reader,
                                        const std::string& type,
                                        std::vector<ErrorInfo>* errors);

 private:
  struct Impl;
  FileParser(std::unique_ptr<Impl> impl);
  ~FileParser();

  std::unique_ptr<Impl> impl_;
};

}  // namespace binary_reader

#endif  // BINARY_READER_INCLUDE_FILE_PARSER_H_
