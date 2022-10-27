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

#include "binary_reader/error.h"

#include <stdio.h>

#include <algorithm>

#include "util/templates.h"

namespace binary_reader {

namespace {

struct MessageInfo {
  constexpr MessageInfo(ErrorKind kind, const char* format)
      : kind(kind), format_count(0), format(format) {
    for (; *format; format++) {
      if (*format == '%') {
        format_count++;
        if (format[1] != 's')
          throw std::invalid_argument{"Invalid format string"};
      }
    }
  }

  ErrorKind kind;
  uint16_t format_count;
  const char* format;
};

constexpr const MessageInfo kDefaultMessages[] = {
    {ErrorKind::Unknown, "Unknown error"},
    {ErrorKind::CannotOpen, "Cannot open file '%s'"},
    {ErrorKind::IoError, "Unknown IO error: errno=%s"},

    {ErrorKind::ShadowingType, "Shadowing existing type '%s'"},
    {ErrorKind::ShadowingMember, "Shadowing existing member '%s'"},
    {ErrorKind::UnknownType, "Unknown type '%s'"},
    {ErrorKind::NoTypes, "No types in definition file"},

    {ErrorKind::OptionMustBeString, "Option values must be a string"},
    {ErrorKind::OptionMustBeStringTyped,
     "Option values must be a string for option '%s'"},
    {ErrorKind::UnknownOptionValue, "Unknown option value '%s'"},
    {ErrorKind::UnknownOptionValueTyped,
     "Unknown option value '%s' for option '%s'"},
    {ErrorKind::AmbiguousOption, "Ambiguous option value '%s'"},
    {ErrorKind::DuplicateOption, "Option '%s' set multiple times"},
    {ErrorKind::OptionInvalidForType, "Option '%s' is not valid for this type"},
    {ErrorKind::UnknownOptionType, "Unknown option '%s'"},

    {ErrorKind::UnexpectedEndOfStream, "Unexpected end of stream"},
    {ErrorKind::LittleEndianAlign,
     "Little endian numbers must be byte aligned"},

    {ErrorKind::FieldsMustBeStatic, "Fields must have a static size"},
};

template <size_t N>
struct FormatHelper {
  template <typename Iter, typename... Args>
  static std::string Format(const char* format, Iter cur, Iter end,
                            Args... args) {
    if (cur == end) {
      return FormatHelper<N - 1>::Format(format, cur, end, args..., "");
    } else {
      return FormatHelper<N - 1>::Format(format, cur + 1, end, args...,
                                         cur->data());
    }
  }
};

template <>
struct FormatHelper<0> {
  template <typename Iter, typename... Args>
  static std::string Format(const char* format, Iter, Iter, Args... args) {
    const int size = snprintf(nullptr, 0, format, args...);
    if (size <= 0)
      return {};
    std::string ret(size, '\0');
    sprintf_s(ret.data(), ret.size() + 1, format, args...);
    return ret;
  }
};

template <size_t N>
constexpr uint16_t MaxMessageFormatCount(const MessageInfo (&array)[N]) {
  uint16_t max = 0;
  for (auto& i : array) {
    if (i.format_count > max)
      max = i.format_count;
  }
  return max;
}

std::string FormatMessage(const MessageInfo& message,
                          std::initializer_list<std::string_view> args) {
  static_assert(MaxMessageFormatCount(kDefaultMessages) <= 4);
  switch (message.format_count) {
    case 0:
      return FormatHelper<0>::Format(message.format, args.begin(), args.end());
    case 1:
      return FormatHelper<1>::Format(message.format, args.begin(), args.end());
    case 2:
      return FormatHelper<2>::Format(message.format, args.begin(), args.end());
    case 3:
      return FormatHelper<3>::Format(message.format, args.begin(), args.end());
    case 4:
      return FormatHelper<4>::Format(message.format, args.begin(), args.end());
    default:
      return "!!INTERNAL ERROR: Cannot construct error message";
  }
}

}  // namespace

ErrorInfo::ErrorInfo() {}

ErrorInfo::ErrorInfo(DebugInfo debug, ErrorKind kind, ErrorLevel level,
                     uint64_t offset)
    : ErrorInfo(debug, kind, {}, level, offset) {}

ErrorInfo::ErrorInfo(DebugInfo debug, ErrorKind kind,
                     std::initializer_list<std::string_view> message_args,
                     ErrorLevel level, uint64_t offset)
    : debug(debug),
      kind(kind),
      message(DefaultErrorMessage(kind, std::move(message_args))),
      level(level),
      offset(offset) {}

ErrorInfo::ErrorInfo(DebugInfo debug, ErrorKind kind, std::string_view message,
                     ErrorLevel level, uint64_t offset)
    : debug(debug),
      kind(kind),
      message(message),
      level(level),
      offset(offset) {}

std::string DefaultErrorMessage(ErrorKind kind,
                                std::initializer_list<std::string_view> args) {
  for (auto& info : kDefaultMessages) {
    if (info.kind == kind)
      return FormatMessage(info, std::move(args));
  }
  return FormatMessage(kDefaultMessages[0], std::move(args));
}

std::ostream& operator<<(std::ostream& os, ErrorLevel level) {
  switch (level) {
    default:
    case ErrorLevel::Error:
      return os << "error";
    case ErrorLevel::Warning:
      return os << "warning";
    case ErrorLevel::Info:
      return os << "info";
  }
}

std::ostream& operator<<(std::ostream& os, const ErrorInfo& error) {
  if (error.debug.file_path.empty()) {
    // error: unknown type 'foo'
    return os << error.level << ": " << error.message;
  } else if (!error.debug.line) {
    // foo/bar.def: error: unknown type 'foo'
    return os << error.debug.file_path << ": " << error.level << ": "
              << error.message;
  } else if (!error.debug.column) {
    // foo/bar.def:6: error: unknown type 'foo'
    return os << error.debug.file_path << ":" << error.debug.line << ": "
              << error.level << ": " << error.message;
  } else {
    // foo/bar.def:6:12: error: unknown type 'foo'
    return os << error.debug.file_path << ":" << error.debug.line << ":"
              << error.debug.column << ": " << error.level << ": "
              << error.message;
  }
}

}  // namespace binary_reader
