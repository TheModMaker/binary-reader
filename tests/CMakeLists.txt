# Copyright 2021 Google LLC
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.


include_directories(.)
add_executable(
    all_tests
    "ast/option_set_unittest.cc"
    "ast/type_info_unittest.cc"
    "parser/definition_parser_unittest.cc"
    "public/codecs_unittest.cc"
    "public/file_object_unittest.cc"
    "public/file_parser_integration.cc"
    "public/file_system_unittest.cc"
    "public/json_unittest.cc"
    "public/number_unittest.cc"
    "public/options_unittest.cc"
    "util/buffered_file_reader_unittest.cc"
    "util/templates_unittest.cc"
)
target_link_libraries(all_tests gtest_main gmock base_lib)

gtest_discover_tests(all_tests)
