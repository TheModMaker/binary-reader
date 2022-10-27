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

#include "binary_reader/file_parser.h"

#include "ast/type_def.h"
#include "parser/definition_parser.h"
#include "util/buffered_file_reader.h"

namespace binary_reader {

struct FileParserOptions::Impl {};

FileParserOptions::FileParserOptions() : impl_(new Impl) {}

// Need to explicitly list every field since std::unique_ptr isn't copyable, so
// we can't use the default copy constructor.
FileParserOptions::FileParserOptions(const FileParserOptions& other)
    : file_system(other.file_system), impl_(new Impl(*other.impl_)) {}

FileParserOptions& FileParserOptions::operator=(
    const FileParserOptions& other) {
  FileParserOptions cp{other};
  *this = std::move(cp);
  return *this;
}

FileParserOptions::FileParserOptions(FileParserOptions&&) = default;
FileParserOptions& FileParserOptions::operator=(FileParserOptions&&) = default;
FileParserOptions::~FileParserOptions() = default;


struct FileParser::Impl {
  struct FileParserDeleter {
    void operator()(FileParser* parser) {
      delete parser;
    }
  };

  FileParserOptions options;
  std::vector<std::shared_ptr<TypeDefinition>> definitions;
};

std::shared_ptr<FileParser> FileParser::CreateFromFile(
    const std::string& path) {
  return CreateFromFile(path, FileParserOptions{}, nullptr);
}

std::shared_ptr<FileParser> FileParser::CreateFromFile(
    const std::string& path, const FileParserOptions& options) {
  return CreateFromFile(path, options, nullptr);
}

std::shared_ptr<FileParser> FileParser::CreateFromFile(
    const std::string& path, ErrorCollection* errors) {
  return CreateFromFile(path, FileParserOptions{}, errors);
}

std::shared_ptr<FileParser> FileParser::CreateFromFile(
    const std::string& path, const FileParserOptions& options,
    ErrorCollection* errors) {
  FileParserOptions opt = options;
  if (!opt.file_system)
    opt.file_system = FileSystem::DefaultFileSystem();

  // File definition files are supposed to be small, so we can just read the
  // whole file into memory.
  std::vector<uint8_t> buffer;
  auto file = opt.file_system->Open(path);
  if (!file) {
    if (errors)
      errors->Add({{path}, ErrorKind::CannotOpen, {path}});
    return nullptr;
  }
  if (!file->ReadFully(&buffer, errors)) {
    return nullptr;
  }

  // TODO: Handle file encoding.
  std::string def{buffer.begin(), buffer.end()};
  return CreateFromDefinition(def, path, opt, errors);
}

std::shared_ptr<FileParser> FileParser::CreateFromDefinition(
    const std::string& def) {
  return CreateFromDefinition(def, "", FileParserOptions{}, nullptr);
}

std::shared_ptr<FileParser> FileParser::CreateFromDefinition(
    const std::string& def, const FileParserOptions& options) {
  return CreateFromDefinition(def, "", options, nullptr);
}

std::shared_ptr<FileParser> FileParser::CreateFromDefinition(
    const std::string& def, ErrorCollection* errors) {
  return CreateFromDefinition(def, "", FileParserOptions{}, errors);
}

std::shared_ptr<FileParser> FileParser::CreateFromDefinition(
    const std::string& def, const FileParserOptions& options,
    ErrorCollection* errors) {
  return CreateFromDefinition(def, "", options, errors);
}

std::shared_ptr<FileParser> FileParser::CreateFromDefinition(
    const std::string& def, const std::string& path) {
  return CreateFromDefinition(def, path, FileParserOptions{}, nullptr);
}

std::shared_ptr<FileParser> FileParser::CreateFromDefinition(
    const std::string& def, const std::string& path,
    const FileParserOptions& options) {
  return CreateFromDefinition(def, path, options, nullptr);
}

std::shared_ptr<FileParser> FileParser::CreateFromDefinition(
    const std::string& def, const std::string& path, ErrorCollection* errors) {
  return CreateFromDefinition(def, path, FileParserOptions{}, errors);
}

std::shared_ptr<FileParser> FileParser::CreateFromDefinition(
    const std::string& def, const std::string& path,
    const FileParserOptions& options, ErrorCollection* errors) {
  std::unique_ptr<Impl> impl(new Impl);
  impl->options = options;
  if (!impl->options.file_system)
    impl->options.file_system = FileSystem::DefaultFileSystem();

  ErrorCollection ignore_errors;
  if (!errors)
    errors = &ignore_errors;
  if (!ParseDefinitionFile(path, def, &impl->definitions, errors))
    return nullptr;
  if (impl->definitions.empty()) {
    if (errors)
      errors->Add({{path}, ErrorKind::NoTypes});
    return nullptr;
  }
  return std::shared_ptr<FileParser>(new FileParser(std::move(impl)),
                                     Impl::FileParserDeleter{});
}

const FileParserOptions& FileParser::options() const {
  return impl_->options;
}

std::vector<std::string> FileParser::GetTypeNames() const {
  std::vector<std::string> ret{impl_->definitions.size()};
  for (size_t i = 0; i < ret.size(); i++) {
    ret[i] = impl_->definitions[i]->alias_name();
  }
  return ret;
}

std::shared_ptr<FileObject> FileParser::ParseFile(const std::string& path) {
  return ParseFile(path, "", nullptr);
}

std::shared_ptr<FileObject> FileParser::ParseFile(const std::string& path,
                                                  ErrorCollection* errors) {
  return ParseFile(path, "", errors);
}

std::shared_ptr<FileObject> FileParser::ParseFile(
    std::shared_ptr<FileReader> reader) {
  return ParseFile(reader, "", nullptr);
}

std::shared_ptr<FileObject> FileParser::ParseFile(
    std::shared_ptr<FileReader> reader, ErrorCollection* errors) {
  return ParseFile(reader, "", errors);
}

std::shared_ptr<FileObject> FileParser::ParseFile(const std::string& path,
                                                  const std::string& type) {
  return ParseFile(path, type, nullptr);
}

std::shared_ptr<FileObject> FileParser::ParseFile(const std::string& path,
                                                  const std::string& type,
                                                  ErrorCollection* errors) {
  auto file = options().file_system->Open(path);
  if (!file) {
    if (errors)
      errors->Add({{path}, ErrorKind::CannotOpen, {path}});
    return nullptr;
  }
  return ParseFile(file, path, type, errors);
}

std::shared_ptr<FileObject> FileParser::ParseFile(
    std::shared_ptr<FileReader> reader, const std::string& type) {
  return ParseFile(reader, type, nullptr);
}

std::shared_ptr<FileObject> FileParser::ParseFile(
    std::shared_ptr<FileReader> reader, const std::string& path,
    const std::string& type) {
  return ParseFile(reader, path, type, nullptr);
}

std::shared_ptr<FileObject> FileParser::ParseFile(std::shared_ptr<FileReader> file,
                                                  const std::string& type,
                                                  ErrorCollection* errors) {
  return ParseFile(file, "", type, errors);
}

std::shared_ptr<FileObject> FileParser::ParseFile(std::shared_ptr<FileReader> file,
                                                  const std::string& path,
                                                  const std::string& type,
                                                  ErrorCollection* errors) {
  std::shared_ptr<TypeDefinition> def;
  if (type.empty()) {
    def = impl_->definitions.back();
  } else {
    for (auto d : impl_->definitions) {
      if (d->alias_name() == type) {
        def = d;
        break;
      }
    }
  }
  if (!def) {
    if (errors)
      errors->Add({{path}, ErrorKind::UnknownType, {type}});
    return nullptr;
  }

  ErrorCollection temp_errors;
  if (!errors)
    errors = &temp_errors;

  Value val;
  auto reader = std::make_shared<BufferedFileReader>(file);
  const bool success = def->ReadValue(reader, &val, errors);
  return success ? val.as_object() : nullptr;
}

FileParser::FileParser(std::unique_ptr<Impl> impl) : impl_(std::move(impl)) {}
FileParser::~FileParser() = default;

}  // namespace binary_reader
