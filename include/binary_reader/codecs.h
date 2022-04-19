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

#ifndef BINARY_READER_INCLUDE_CODECS_H_
#define BINARY_READER_INCLUDE_CODECS_H_

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "binary_reader/error.h"

namespace binary_reader {

/// <summary>
/// Defines an interface for a text converter.  A text converter will convert
/// between a byte buffer and a UTF-16 string.  A converter instance handles a
/// streaming conversion and maintains temporary state for partial buffers.  It
/// only needs to handle encoding or decoding at a single time.
/// </summary>
class TextConverter {
 public:
  enum class Status {
    Success,
    Warning,
    Error,
  };

  TextConverter() {}
  virtual ~TextConverter() {}
  TextConverter(const TextConverter&) = delete;
  TextConverter(TextConverter&&) = delete;
  TextConverter& operator=(const TextConverter&) = delete;
  TextConverter& operator=(TextConverter&&) = delete;

  virtual Status Decode(const uint8_t* buffer, size_t size,
                        std::u16string* output, ErrorInfo* error) = 0;

  virtual Status Encode(const char16_t* buffer, size_t size,
                        std::vector<uint8_t>* output, ErrorInfo* error) = 0;

  virtual void Reset() = 0;
};

class Codec {
 public:
  Codec() {}
  virtual ~Codec() {}
  Codec(const Codec&) = delete;
  Codec(Codec&&) = delete;
  Codec& operator=(const Codec&) = delete;
  Codec& operator=(Codec&&) = delete;

  /// <summary>
  /// Returns a new instance of a converter.
  /// </summary>
  virtual std::shared_ptr<TextConverter> CreateCoder() = 0;
};

class CodecCollection {
 public:
  CodecCollection() {}
  virtual ~CodecCollection() {}
  CodecCollection(const CodecCollection&) = delete;
  CodecCollection(CodecCollection&&) = delete;
  CodecCollection& operator=(const CodecCollection&) = delete;
  CodecCollection& operator=(CodecCollection&&) = delete;

  /// <summary>
  /// Finds the codec with the given name.
  /// </summary>
  /// <param name="codec">The codec to use.</param>
  virtual std::shared_ptr<Codec> GetCodec(const std::string& codec);

  /// <summary>
  /// Gets the default, "system" codec.
  /// </summary>
  virtual std::shared_ptr<Codec> GetDefaultCodec();

  /// <summary>
  /// Adds a new codec to the collection.
  /// </summary>
  /// <param name="name">The name of the codec.</param>
  /// <param name="codec">The Codec instance.</param>
  void AddCodec(const std::string& name, std::shared_ptr<Codec> codec);

  /// <summary>
  /// Creates a new CodecCollection that is populated with the default codecs.
  /// </summary>
  static std::shared_ptr<CodecCollection> CreateDefaultCollection();

 private:
  std::unordered_map<std::string, std::shared_ptr<Codec>> codecs_;
};

}  // namespace binary_reader

#endif  // BINARY_READER_INCLUDE_CODECS_H_
