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

#ifndef BINARY_READER_TESTS_GTEST_WRAPPER_H_
#define BINARY_READER_TESTS_GTEST_WRAPPER_H_

#ifdef _MSC_VER
#  pragma warning(disable : 26812)
#  pragma warning(disable : 26451)
#  pragma warning(disable : 26800)
#  pragma warning(disable : 26495)
#  pragma warning(disable : 26439)
#endif  // _MSC_VER

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#ifdef _MSC_VER
#  pragma warning(default : 26812)
#  pragma warning(default : 26451)
#  pragma warning(default : 26800)
#  pragma warning(default : 26495)
#  pragma warning(default : 26439)
#endif  // _MSC_VER

#endif  // BINARY_READER_TESTS_GTEST_WRAPPER_H_
