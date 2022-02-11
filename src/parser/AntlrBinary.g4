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

grammar AntlrBinary;

///////////////////////////////////////////////////////////////////////////////
// Lexer

// Skip whitespace and comments.
WHITESPACE : [\p{White_Space}]+ -> skip ;
LINE_COMMENT : '//' ~[\r\n]* '\r'? '\n' -> skip ;
LONG_COMMENT : '/*' .*? '*/' -> skip ;

TYPE : 'type' ;

fragment DIGIT : [\p{Numeric_Type=Decimal}] ;
NUMBER : DIGIT* '.' DIGIT+ ('e' [-+]? DIGIT+)?
       | DIGIT+ ('.' DIGIT*)? ('e' [-+]? DIGIT+)? ;
IDENTIFIER : '$'? [\p{ID_Start}] [\p{ID_Continue}]* ;


///////////////////////////////////////////////////////////////////////////////
// Parser

main : globalDefinition* EOF ;
globalDefinition : typeDefinition ;
empty : ;

//// Common definitions

//// Type definitions
typeDefinition : TYPE IDENTIFIER '{' typeMember* '}' ;
typeMember : globalDefinition
           | dataField ;

dataField : completeType IDENTIFIER ';' ;

//// Types
completeType : IDENTIFIER ;
