// Copyright 2021 Google LLC
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

#include <antlr4-runtime.h>

#include <iostream>
#include <sstream>

//#include "AntlrBinaryLexer.h"
//#include "AntlrBinaryParser.h"

int main() {
#if 0
  std::stringstream stream;
  stream.str(R"(
type foobar {
  // alias my_int = integer(32, signed);
  // my_int z;

  int32 x;
  integer(32) y;
  integer(32, signed) y;
  int32(unsigned) z;
  int32(64) w;
}
)");

  antlr4::ANTLRInputStream input(stream);
  antlr4::AntlrBinaryLexer lexer(&input);
  antlr4::CommonTokenStream tokens(&lexer);
  antlr4::AntlrBinaryParser parser(&tokens);

  auto* tree = parser.main();
  //std::cout << "Hello: " << tree->IDENTIFIER()->getText() << std::endl;
#endif
  return 0;
}

