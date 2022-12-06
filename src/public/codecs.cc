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

#include "binary_reader/codecs.h"

#include <cstring>

namespace binary_reader {

namespace {

class Utf8Converter final : public TextConverter {
 public:
  Status Decode(const uint8_t* buffer, size_t size, std::u16string* output,
                ErrorInfo* error) {
    size_t offset = 0;
    while (temp_used_ > 0 || offset < size) {
      const uint8_t head = temp_used_ > 0 ? temp_[0] : buffer[offset];
      const size_t num_bytes = GetNumBytes(head);
      if (num_bytes == 0) {
        error->message = "Invalid UTF-8 byte sequence";
        error->offset += offset;
        return Status::Error;
      }

      // Check we have enough input bytes to read the whole sequence.  If not,
      // copy partial bytes to the end of temp space.
      if (num_bytes > size - offset + temp_used_) {
        std::memcpy(temp_ + temp_used_, buffer + offset, size - offset);
        temp_used_ += static_cast<uint8_t>(size - offset);
        break;
      }

      // Strip of the leading 1 bits of the first byte.
      uint32_t code_point = head & ((1u << (8 - num_bytes)) - 1);
      // Read remaining bytes
      for (size_t i = 1; i < num_bytes; i++) {
        const uint8_t cur =
            i < temp_used_ ? temp_[i] : buffer[i - temp_used_ + offset];
        if ((cur & 0xc0) != 0x80) {
          error->message = "Invalid UTF-8 byte sequence";
          error->offset += offset + i;
          return Status::Error;
        }
        code_point = (code_point << 6) | (cur & 0x3f);
      }

      // Convert to UTF-16
      if (code_point < 0xffff) {
        // Assume that an encoded surrogate pair should be passed as-is.
        output->push_back(static_cast<char16_t>(code_point));
      } else {
        code_point -= 0x10000;
        output->push_back(static_cast<char16_t>(0xd800 | (code_point >> 10)));
        output->push_back(static_cast<char16_t>(0xdc00 | (code_point & 0x3ff)));
      }

      // Temp only stores a partial sequence, so is reset after reading a
      // complete one.
      offset += num_bytes - temp_used_;
      temp_used_ = 0;
    }
    return Status::Success;
  }

  Status Encode(const char16_t* buffer, size_t size,
                std::vector<uint8_t>* output, ErrorInfo*) {
    for (size_t i = 0; i < size; i++) {
      // Encode unpaired surrogate pairs directly.
      uint32_t code_point;
      if (i + 1 < size && buffer[i] >= 0xd800 && buffer[i] <= 0xdbff &&
          buffer[i + 1] >= 0xdc00 && buffer[i + 1] <= 0xdfff) {
        code_point =
            ((buffer[i] & 0x3ff) << 10) | ((buffer[i + 1] & 0x3ff)) + 0x10000;
        i++;
      } else {
        code_point = buffer[i];
      }

      if (code_point < 0x80) {
        output->emplace_back(code_point);
      } else if (code_point < 0x800) {
        output->emplace_back(0xc0 | (code_point >> 6));
        output->emplace_back(0x80 | (code_point & 0x3f));
      } else if (code_point < 0x10000) {
        output->emplace_back(0xe0 | (code_point >> 12));
        output->emplace_back(0x80 | ((code_point >> 6) & 0x3f));
        output->emplace_back(0x80 | (code_point & 0x3f));
      } else {
        output->emplace_back(0xf0 | (code_point >> 18));
        output->emplace_back(0x80 | ((code_point >> 12) & 0x3f));
        output->emplace_back(0x80 | ((code_point >> 6) & 0x3f));
        output->emplace_back(0x80 | (code_point & 0x3f));
      }
    }
    return Status::Success;
  }

  void Reset() {
    temp_used_ = 0;
  }

 private:
  uint8_t GetNumBytes(uint8_t head) {
    if ((head & 0x80) == 0) {
      return 1;
    } else if ((head & 0xe0) == 0xc0) {
      return 2;
    } else if ((head & 0xf0) == 0xe0) {
      return 3;
    } else if ((head & 0xf8) == 0xf0) {
      return 4;
    } else {
      return 0;  // Error
    }
  }

  uint8_t temp_[4]{};
  uint8_t temp_used_ = 0;
};

template <typename T>
class DefaultCodec final : public Codec {
 public:
  std::shared_ptr<TextConverter> CreateCoder() override {
    return std::make_shared<T>();
  }
};

}  // namespace


std::shared_ptr<Codec> CodecCollection::GetCodec(const std::string& codec) {
  auto it = codecs_.find(codec);
  return it == codecs_.end() ? nullptr : it->second;
}

std::shared_ptr<Codec> CodecCollection::GetDefaultCodec() {
  return GetCodec("utf8");
}

void CodecCollection::AddCodec(const std::string& name,
                               std::shared_ptr<Codec> codec) {
  codecs_[name] = codec;
}

std::shared_ptr<CodecCollection> CodecCollection::CreateDefaultCollection() {
  auto c = std::make_shared<DefaultCodec<Utf8Converter>>();
  auto ret = std::make_shared<CodecCollection>();
  ret->AddCodec("utf8", c);
  ret->AddCodec("utf-8", c);
  ret->AddCodec("UTF8", c);
  ret->AddCodec("UTF-8", c);
  return ret;
}

}  // namespace binary_reader
