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

#define NON_COPYABLE_TYPE(type) \
  type(const type&) = delete;   \
  type& operator=(const type&) = delete

#define NON_MOVABLE_TYPE(type) \
  type(type&&) = delete;       \
  type& operator=(type&&) = delete

#define NON_COPYABLE_OR_MOVABLE_TYPE(type) \
  NON_COPYABLE_TYPE(type);                 \
  NON_MOVABLE_TYPE(type)
