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

cmake_minimum_required (VERSION 3.8)
set(CMAKE_CXX_STANDARD 17)
set(BUILD_SHARED_LIB OFF)
set(BUILD_SHARED_LIBS OFF)

enable_testing()
include(FetchContent)
include(GoogleTest)

project("binary_reader")

# https://gitlab.kitware.com/cmake/community/-/wikis/FAQ#how-can-i-build-my-msvc-application-with-a-static-runtime
# TODO: Find a better way to do this.
if(MSVC)
  foreach(flag_var
      CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE
      CMAKE_CXX_FLAGS_MINSIZEREL CMAKE_CXX_FLAGS_RELWITHDEBINFO)
    if(${flag_var} MATCHES "/MD")
      string(REPLACE "/MD" "/MT" ${flag_var} "${${flag_var}}")
    endif()
  endforeach()

  SET(CMAKE_EXE_LINKER_FLAGS_DEBUG
      "${CMAKE_EXE_LINKER_FLAGS_DEBUG} msvcrtd.lib")
  SET(CMAKE_EXE_LINKER_FLAGS_RELEASE
      "${CMAKE_EXE_LINKER_FLAGS_RELEASE} msvcrt.lib")

  # Ignore warnings in third-party code.
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /external:W0 /external:anglebrackets")
ENDIF()

# Load Antlr4
FetchContent_Declare(
  antlr_files
  URL      https://www.antlr.org/download/antlr-4.9.2-complete.jar
  URL_HASH SHA256=BB117B1476691DC2915A318EFD36F8957C0AD93447FB1DAC01107EB15FE137CD
  DOWNLOAD_NO_EXTRACT TRUE
)
FetchContent_MakeAvailable(antlr_files)
set(ANTLR_EXECUTABLE ${antlr_files_SOURCE_DIR}/antlr-4.9.2-complete.jar)
list(APPEND CMAKE_MODULE_PATH
     ${CMAKE_CURRENT_SOURCE_DIR}/third_party/antlr4/runtime/Cpp/cmake)

add_definitions(-DANTLR4CPP_STATIC)
add_definitions(-D_SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING)
set(WITH_STATIC_CRT ON)
set(WITH_DEMO OFF)
add_subdirectory(third_party/antlr4/runtime/Cpp)
find_package(ANTLR REQUIRED)
include_directories(third_party/antlr4/runtime/Cpp/runtime/src)

# Load tclap
FetchContent_Declare(
  tclap_files
  URL      https://downloads.sourceforge.net/project/tclap/tclap-1.4.0-rc1.tar.bz2
  URL_HASH SHA1=a93c738d6e6320ae7fdda414af69d8844424fad7
)
FetchContent_MakeAvailable(tclap_files)
include_directories(${tclap_files_SOURCE_DIR}/include)

# Main library
if(MSVC)
  # Remove the default warning flag to avoid warnings about duplicate values.
  string(REGEX REPLACE "/W[0-4]" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
  add_compile_options(/W4 /WX /permissive-)
else()
  add_compile_options(-Wall -Wextra -pedantic -Werror)
endif()

include_directories(include)
include_directories(src)

add_subdirectory(src/parser)

add_library(base_lib STATIC
            "src/ast/field_info.cc"
            "src/ast/literal.cc"
            "src/ast/option_set.cc"
            "src/ast/type_definition.cc"
            "src/ast/type_info.cc"
            "src/parser/definition_parser.cc"
            "src/public/codecs.cc"
            "src/public/error.cc"
            "src/public/error_collection.cc"
            "src/public/file_object.cc"
            "src/public/file_parser.cc"
            "src/public/file_system.cc"
            "src/public/json.cc"
            "src/public/number.cc"
            "src/public/options.cc"
            "src/public/utf_string.cc"
            "src/public/value.cc"
            "src/util/buffered_file_reader.cc"
            "src/util/memory_file_system.cc"
)
target_link_libraries(base_lib parser)

# Main executable
add_executable(binary_reader
               "src/app/command_line.cc"
               "src/app/main.cc"
)
target_link_libraries(binary_reader base_lib)

# Tests
add_subdirectory(third_party/googletest)
add_subdirectory(tests)
