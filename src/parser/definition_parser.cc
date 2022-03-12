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

  struct DoPop sealed {
    NON_COPYABLE_OR_MOVABLE_TYPE(DoPop);

    explicit DoPop(Stack* stack) : stack_(stack) {}
    ~DoPop() {
      stack_->entries_.pop_back();
    }

    Stack* stack_;
  };

  Stack()
      : default_types_(TypeInfoBase::GetBuiltInTypes()),
        dummy_(std::make_shared<FieldInfo>("", nullptr)) {
    // Create a dummy entry to hold the global statements.
    entries_.emplace_back(std::make_shared<TypeDefinition>("<global>"));
  }

  std::shared_ptr<TypeDefinition> top() {
    return entries_.back();
  }

  DoPop AddType(const std::string& name) {
    entries_.emplace_back(std::make_shared<TypeDefinition>(name));
    return DoPop(this);
  }

  std::shared_ptr<TypeInfoBase> GetType(const std::string& name) {
    auto mem = FindMember(name, /* inherit= */ true);
    if (mem == dummy_) {
      for (auto t : default_types_) {
        if (t->alias_name() == name)
          return t;
      }
    }
    return std::dynamic_pointer_cast<TypeInfoBase>(mem);
  }

  std::shared_ptr<Statement> FindMember(const std::string& name, bool inherit) {
    for (auto it = entries_.rbegin(); it != entries_.rend(); it++) {
      for (const auto stmt : (*it)->statements()) {
        if (auto t = std::dynamic_pointer_cast<TypeDefinition>(stmt)) {
          if (t->alias_name() == name)
            return t;
        } else if (auto f = std::dynamic_pointer_cast<FieldInfo>(stmt)) {
          if (f->name() == name)
            return f;
        }
      }

      if (!inherit)
        return nullptr;
      if (name == (*it)->alias_name())
        return *it;
    }
    for (auto type : default_types_) {
      if (type->alias_name() == name)
        return dummy_;
    }
    return nullptr;
  }

  const std::vector<std::shared_ptr<TypeInfoBase>> default_types_;
  // This is a dummy field that exists so we can return a truthy value when a
  // default alias exists.  This will get removed once aliases are supported
  // generically.
  const std::shared_ptr<Statement> dummy_;
  std::vector<std::shared_ptr<TypeDefinition>> entries_;
};

class Visitor : public antlr4::AntlrBinaryVisitor {
 public:
  Visitor(const std::string& path,
          std::vector<std::shared_ptr<TypeDefinition>>* defs,
          std::vector<ErrorInfo>* errors)
      : path_(path), defs_(defs), errors_(errors) {}

  antlrcpp::Any visitMain(
      antlr4::AntlrBinaryParser::MainContext* ctx) override {
    for (auto* field : ctx->globalDefinition()) {
      HandleMember(field->accept(this), field->start);
    }

    for (auto stmt : stack_.top()->statements()) {
      if (auto def = std::dynamic_pointer_cast<TypeDefinition>(stmt)) {
        defs_->emplace_back(def);
      }
    }
    return {};
  }

  /////////////////////////////////////////////////////////////////////////////
  // Type definitions

  antlrcpp::Any visitTypeDefinition(
      antlr4::AntlrBinaryParser::TypeDefinitionContext* ctx) override {
    auto pop_stack = stack_.AddType(ctx->IDENTIFIER()->getText());

    for (auto* field : ctx->typeMember()) {
      HandleMember(field->accept(this), field->start);
    }
    return stack_.top();
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

  antlrcpp::Any visitEmpty(
      antlr4::AntlrBinaryParser::EmptyContext*) override {
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

  /// <summary>
  /// Handles adding a new member to the current stack entry
  /// </summary>
  /// <param name="field_data">The new entry to add.</param>
  /// <param name="start">The token to use for debug info.</param>
  void HandleMember(antlrcpp::Any field_data, antlr4::Token* start) {
    // This is also called for top-level definitions, which can't have fields.
    // However, the grammar ensures that this doesn't happen so those paths
    // won't get taken.
    std::string field_name;
    if (field_data.is<std::shared_ptr<FieldInfo>>()) {
      field_name = field_data.as<std::shared_ptr<FieldInfo>>()->name();
    } else {
      field_name =
          field_data.as<std::shared_ptr<TypeDefinition>>()->alias_name();
    }

    if (stack_.FindMember(field_name, /* inherit= */ false)) {
      AddError("Cannot shadow existing member " + field_name, start);
    } else if (stack_.FindMember(field_name, /* inherit= */ true)) {
      AddError("Shadowing existing member " + field_name, start,
               ErrorLevel::Warning);
    }

    if (field_data.is<std::shared_ptr<FieldInfo>>()) {
      stack_.top()->statements().emplace_back(
          field_data.as<std::shared_ptr<FieldInfo>>());
    } else {
      stack_.top()->statements().emplace_back(
          field_data.as<std::shared_ptr<TypeDefinition>>());
    }
  }

  void AddError(const std::string& message, antlr4::Token* token,
                ErrorLevel level = ErrorLevel::Error) {
    errors_->push_back({path_, message, level, token->getStartIndex(),
                        token->getLine(), token->getCharPositionInLine()});
  }

  Stack stack_;
  const std::string path_;
  std::vector<std::shared_ptr<TypeDefinition>>* const defs_;
  std::vector<ErrorInfo>* const errors_;
};

}  // namespace

bool ParseDefinitionFile(const std::string& path, const std::string& buffer,
                         std::vector<std::shared_ptr<TypeDefinition>>* defs,
                         std::vector<ErrorInfo>* errors) {
  try {
    antlr4::ANTLRInputStream input(buffer);
    antlr4::AntlrBinaryLexer lexer(&input);
    antlr4::CommonTokenStream tokens(&lexer);
    antlr4::AntlrBinaryParser parser(&tokens);

    Visitor visitor{path, defs, errors};
    visitor.visit(parser.main());
    for (const auto& err : *errors) {
      if (err.level == ErrorLevel::Error)
        return false;
    }
    return true;
  } catch (antlr4::RuntimeException&) {
    // TODO: Report parser errors.
    return false;
  }
}

}  // namespace binary_reader
