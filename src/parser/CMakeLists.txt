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

# Generate lexer/parser
antlr_target(antlr_parser AntlrBinary.g4 PACKAGE antlr4 VISITOR)

add_library(parser STATIC
            ${ANTLR_antlr_parser_CXX_OUTPUTS}
)
if(MSVC)
  # Suppress unused parameter warning in generated code
  target_compile_options(parser PRIVATE "/wd4100")
else()
  target_compile_options(parser PRIVATE "-Wno-unused-parameter")
endif()
target_include_directories(parser PUBLIC ${ANTLR_antlr_parser_OUTPUT_DIR})
target_link_libraries(parser antlr4_static)
