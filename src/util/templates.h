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

#ifndef BINARY_READER_UTIL_TEMPLATES_H_
#define BINARY_READER_UTIL_TEMPLATES_H_

#include <cmath>
#include <cstdint>
#include <limits>

namespace binary_reader {

template <typename Dest, typename Source>
Dest clamp_cast(Source src) {
  if constexpr (std::numeric_limits<Dest>::is_integer) {
    constexpr const Dest kMinDest = std::numeric_limits<Dest>::lowest();
    constexpr const Dest kMaxDest = std::numeric_limits<Dest>::max();
    if (src > 0 &&
        (!std::isfinite(static_cast<double>(src)) ||
         static_cast<uintmax_t>(src) > static_cast<uintmax_t>(kMaxDest))) {
      return kMaxDest;
    }
    if (src < 0 &&
        (!std::isfinite(static_cast<double>(src)) ||
         static_cast<intmax_t>(src) < static_cast<intmax_t>(kMinDest))) {
      return kMinDest;
    }
    return static_cast<Dest>(src);
  } else {
    // If the result is a float, just cast the number and let it overflow if
    // necessary.
    return static_cast<Dest>(src);
  }
}

template <typename T, size_t N>
constexpr size_t arraysize(T (&)[N]) {
  return N;
}

}  // namespace binary_reader

#endif  // BINARY_READER_UTIL_TEMPLATES_H_
