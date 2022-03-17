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

#include "parser/definition_parser.h"

#include <antlr4-runtime.h>

#include "AntlrBinaryLexer.h"
#include "AntlrBinaryParser.h"
#include "AntlrBinaryVisitor.h"
#include "util/macros.h"

namespace binary_reader {

namespace {

struct Stack sealed {
  NON_COPYABLE_OR_MOVABLE_TYPE(Stack);

  Stack() : default_types_(TypeInfoBase::GetBuiltInTypes()) {}

  std::shared_ptr<TypeInfoBase> GetType(const std::string& name) {
    for (auto t : types_) {
      if (t->alias_name() == name)
        return t;
    }
    for (auto t : default_types_) {
      if (t->alias_name() == name)
        return t;
    }
    return nullptr;
  }

  std::shared_ptr<Statement> FindMember(const std::string& name) {
    for (auto stmt : statements_) {
      if (auto field = std::dynamic_pointer_cast<FieldInfo>(stmt)) {
        if (field->name() == name)
          return field;
      }
    }
    return nullptr;
  }

  const std::vector<std::shared_ptr<TypeInfoBase>> default_types_;
  std::vector<std::shared_ptr<TypeDefinition>> types_;
  std::vector<std::shared_ptr<Statement>> statements_;
};

class Visitor : public antlr4::AntlrBinaryVisitor {
 public:
  Visitor(const std::string& path,
          std::vector<std::shared_ptr<TypeDefinition>>* defs,
          ErrorCollection* errors)
      : path_(path), defs_(defs), errors_(errors) {}

  antlrcpp::Any visitMain(
      antlr4::AntlrBinaryParser::MainContext* ctx) override {
    for (auto* field : ctx->globalDefinition()) {
      auto parsed_type =
          field->accept(this).as<std::shared_ptr<TypeDefinition>>();
      if (stack_.GetType(parsed_type->alias_name())) {
        AddError("Cannot shadow existing type " + parsed_type->alias_name(),
                 field->start);
      }
      stack_.types_.emplace_back(parsed_type);
      defs_->emplace_back(parsed_type);
    }

    return {};
  }

  /////////////////////////////////////////////////////////////////////////////
  // Type definitions

  antlrcpp::Any visitTypeDefinition(
      antlr4::AntlrBinaryParser::TypeDefinitionContext* ctx) override {
    for (auto* field : ctx->typeMember()) {
      auto parsed_field = field->accept(this).as<std::shared_ptr<FieldInfo>>();
      if (stack_.FindMember(parsed_field->name())) {
        AddError("Cannot shadow existing member " + parsed_field->name(),
                 field->start);
      } else if (stack_.GetType(parsed_field->name()) ||
                 parsed_field->name() == ctx->IDENTIFIER()->getText()) {
        AddError("Shadowing type " + parsed_field->name(), field->start,
                 ErrorLevel::Warning);
      }
      stack_.statements_.push_back(parsed_field);
    }

    return std::make_shared<TypeDefinition>(ctx->IDENTIFIER()->getText(),
                                            stack_.statements_);
  }

  antlrcpp::Any visitDataField(
      antlr4::AntlrBinaryParser::DataFieldContext* ctx) override {
    return std::make_shared<FieldInfo>(
        ctx->IDENTIFIER()->getText(),
        visit(ctx->completeType()).as<std::shared_ptr<TypeInfoBase>>());
  }

  /////////////////////////////////////////////////////////////////////////////
  // Types

  antlrcpp::Any visitCompleteType(
      antlr4::AntlrBinaryParser::CompleteTypeContext* ctx) override {
    const std::string name = ctx->IDENTIFIER()->getText();
    auto type = stack_.GetType(name);
    if (type)
      return type;

    AddError("Unknown type " + name, ctx->start);
    return std::shared_ptr<TypeInfoBase>(nullptr);
  }

  /////////////////////////////////////////////////////////////////////////////
  // Forwards/misc

  antlrcpp::Any visitEmpty(antlr4::AntlrBinaryParser::EmptyContext*) override {
    return antlrcpp::Any();
  }

  antlrcpp::Any visitGlobalDefinition(
      antlr4::AntlrBinaryParser::GlobalDefinitionContext* ctx) override {
    return visit(ctx->typeDefinition());
  }

  antlrcpp::Any visitTypeMember(
      antlr4::AntlrBinaryParser::TypeMemberContext* ctx) override {
    return visit(ctx->dataField());
  }

  /////////////////////////////////////////////////////////////////////////////
  // Utilities

  void AddError(const std::string& message, antlr4::Token* token,
                ErrorLevel level = ErrorLevel::Error) {
    errors_->Add({path_, message, level, token->getStartIndex(),
                  token->getLine(), token->getCharPositionInLine()});
  }

  Stack stack_;
  const std::string path_;
  std::vector<std::shared_ptr<TypeDefinition>>* const defs_;
  ErrorCollection* const errors_;
};

}  // namespace

bool ParseDefinitionFile(const std::string& path, const std::string& buffer,
                         std::vector<std::shared_ptr<TypeDefinition>>* defs,
                         ErrorCollection* errors) {
  try {
    antlr4::ANTLRInputStream input(buffer);
    antlr4::AntlrBinaryLexer lexer(&input);
    antlr4::CommonTokenStream tokens(&lexer);
    antlr4::AntlrBinaryParser parser(&tokens);

    Visitor visitor{path, defs, errors};
    visitor.visit(parser.main());
    return !errors->has_errors();
  } catch (antlr4::RuntimeException&) {
    // TODO: Report parser errors.
    return false;
  }
}

}  // namespace binary_reader
