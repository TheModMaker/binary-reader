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

include_directories(src)

add_subdirectory(src/parser)

add_library(base_lib STATIC
            "src/util/empty_compile.cc"
)
target_link_libraries(base_lib parser)

add_executable(binary_reader
               "src/main.cc"
)
target_link_libraries(binary_reader base_lib)

add_subdirectory(third_party/googletest)
add_subdirectory(tests)
