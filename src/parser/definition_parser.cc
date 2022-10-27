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

#include <utility>

#include "AntlrBinaryLexer.h"
#include "AntlrBinaryParser.h"
#include "AntlrBinaryVisitor.h"
#include "ast/field_info.h"
#include "ast/option_set.h"
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
        AddError(ErrorKind::ShadowingType, {parsed_type->alias_name()},
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
        AddError(ErrorKind::ShadowingMember, {parsed_field->name()},
                 field->start);
      } else if (stack_.GetType(parsed_field->name()) ||
                 parsed_field->name() == ctx->IDENTIFIER()->getText()) {
        AddError(ErrorKind::ShadowingType, {parsed_field->name()}, field->start,
                 ErrorLevel::Warning);
      }
      stack_.statements_.push_back(parsed_field);
    }

    auto statements = std::move(stack_.statements_);
    stack_.statements_.clear();
    return std::make_shared<TypeDefinition>(GetDebugInfo(ctx->start),
                                            ctx->IDENTIFIER()->getText(),
                                            std::move(statements));
  }

  antlrcpp::Any visitDataField(
      antlr4::AntlrBinaryParser::DataFieldContext* ctx) override {
    return std::make_shared<FieldInfo>(
        GetDebugInfo(ctx->start),
        ctx->IDENTIFIER()->getText(),
        visit(ctx->completeType()).as<std::shared_ptr<TypeInfoBase>>());
  }

  /////////////////////////////////////////////////////////////////////////////
  // Types

  antlrcpp::Any visitOption(
      antlr4::AntlrBinaryParser::OptionContext* ctx) override {
    OptionType type = OptionType::Unknown;
    if (ctx->name) {
      type = GetOptionType(UtfString::FromUtf8(ctx->name->getText()));
      if (type == OptionType::Unknown) {
        AddError(ErrorKind::UnknownOptionType, {ctx->name->getText()},
                 ctx->start);
      }
    }
    return std::make_tuple(GetDebugInfo(ctx->start), type,
                           UtfString::FromUtf8(ctx->expr->getText()));
  }

  antlrcpp::Any visitOptionList(
      antlr4::AntlrBinaryParser::OptionListContext* ctx) override {
    std::shared_ptr<OptionSet> options =
        ctx->optionList()
            ? visit(ctx->optionList()).as<std::shared_ptr<OptionSet>>()
            : std::make_shared<OptionSet>();
    auto tuple = visit(ctx->option())
                     .as<std::tuple<DebugInfo, OptionType, UtfString>>();
    options->AddStatic(std::get<0>(tuple), std::get<1>(tuple),
                       std::get<2>(tuple), errors_);
    return options;
  }

  antlrcpp::Any visitCompleteType(
      antlr4::AntlrBinaryParser::CompleteTypeContext* ctx) override {
    const std::string name = ctx->IDENTIFIER()->getText();
    auto ret = stack_.GetType(name);
    if (!ret) {
      AddError(ErrorKind::UnknownType, {name}, ctx->start);
      return ret;
    }

    Options options;
    if (ctx->optionList()) {
      auto valid_options = ret->GetOptionTypes();
      auto set = visit(ctx->optionList()).as<std::shared_ptr<OptionSet>>();
      set->BuildStaticOptions(valid_options, &options, errors_);
    }
    ret = ret->Instantiate(GetDebugInfo(ctx->start), options);
    if (!ret) {
      AddError(ErrorKind::Unknown, {}, ctx->start);
    }
    return ret;
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

  DebugInfo GetDebugInfo(antlr4::Token* token) {
    return {path_, token->getLine(), token->getCharPositionInLine()};
  }

  void AddError(ErrorKind kind, std::initializer_list<std::string_view> args,
                antlr4::Token* token, ErrorLevel level = ErrorLevel::Error) {
    errors_->Add({GetDebugInfo(token), kind, std::move(args), level});
  }

  Stack stack_;
  const std::string path_;
  std::vector<std::shared_ptr<TypeDefinition>>* const defs_;
  ErrorCollection* const errors_;
};

class ErrorHandler sealed : public antlr4::ANTLRErrorListener {
 public:
  ErrorHandler(const std::string& file, ErrorCollection* errors)
      : file_(file), errors_(errors) {}

  void syntaxError(antlr4::Recognizer*, antlr4::Token* token,
                   size_t line, size_t, const std::string& msg,
                   std::exception_ptr) override {
    errors_->Add({{file_, line, token->getStartIndex()}, ErrorKind::Unknown,
                  msg});
  }

  void reportAmbiguity(antlr4::Parser*, const antlr4::dfa::DFA&, size_t, size_t,
                       bool, const antlrcpp::BitSet&,
                       antlr4::atn::ATNConfigSet*) override {}

  void reportAttemptingFullContext(antlr4::Parser*, const antlr4::dfa::DFA&,
                                   size_t, size_t, const antlrcpp::BitSet&,
                                   antlr4::atn::ATNConfigSet*) override {}

  void reportContextSensitivity(antlr4::Parser*, const antlr4::dfa::DFA&,
                                size_t, size_t, size_t,
                                antlr4::atn::ATNConfigSet*) override {}

 private:
  const std::string file_;
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

    // Disable default error handlers (e.g. console logging).
    ErrorHandler error_handler(path, errors);
    lexer.removeErrorListeners();
    lexer.addErrorListener(&error_handler);
    parser.removeErrorListeners();
    parser.addErrorListener(&error_handler);

    Visitor visitor{path, defs, errors};
    visitor.visit(parser.main());
    return !errors->has_errors();
  } catch (antlr4::RuntimeException& e) {
    // TODO: Investigate if these ever happen.
    errors->Add({{path}, ErrorKind::Unknown, {e.what()}});
    return false;
  }
}

}  // namespace binary_reader
