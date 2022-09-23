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

#include "binary_reader/utf_string.h"

#include "binary_reader/codecs.h"

namespace binary_reader {

UtfString::UtfString() = default;
UtfString::UtfString(const std::u16string& str) : utf16_buffer_(str) {}
UtfString::~UtfString() = default;
UtfString::UtfString(const UtfString&) = default;
UtfString::UtfString(UtfString&&) = default;
UtfString& UtfString::operator=(const UtfString&) = default;
UtfString& UtfString::operator=(UtfString&&) = default;

UtfString UtfString::FromEncoding(const uint8_t* bytes, size_t size,
                                  std::shared_ptr<Codec> codec,
                                  ErrorInfo* error) {
  UtfString ret;
  auto coder = codec->CreateCoder();
  if (coder->Decode(bytes, size, &ret.utf16_buffer_, error) ==
      TextConverter::Status::Error) {
    return {};
  }
  return ret;
}

UtfString UtfString::FromEncoding(const char* bytes, size_t size,
                                  std::shared_ptr<Codec> codec,
                                  ErrorInfo* error) {
  return FromEncoding(reinterpret_cast<const uint8_t*>(bytes), size, codec,
                      error);
}

UtfString UtfString::FromUtf8(const std::string& str) {
  ErrorInfo error;
  return FromEncoding(
      str.data(), str.size(),
      CodecCollection::CreateDefaultCollection()->GetCodec("utf8"), &error);
}

std::vector<uint8_t> UtfString::AsBytes(std::shared_ptr<Codec> codec,
                                        ErrorInfo* error) const {
  std::vector<uint8_t> ret;
  auto coder = codec->CreateCoder();
  (void)coder->Encode(utf16_buffer_.data(), utf16_buffer_.size(), &ret, error);
  return ret;
}

std::string UtfString::AsUtf8() const {
  ErrorInfo error;
  auto vec = AsBytes(
      CodecCollection::CreateDefaultCollection()->GetCodec("utf8"), &error);
  return std::string{vec.begin(), vec.end()};
}

}  // namespace binary_reader
