//===--- iwyu_ast_util.h - clang-AST utilities for include-what-you-use ---===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// Utilities that make it easier to work with Clang's AST.

#include "iwyu_ast_util.h"
#include <set>
#include <string>
#include "port.h"
#include "iwyu_globals.h"
#include "iwyu_location_util.h"
#include "iwyu_output.h"
#include "iwyu_string_util.h"
#include "llvm/Support/Casting.h"
#include "llvm/Support/raw_ostream.h"
#include "clang/AST/DeclBase.h"
#include "clang/AST/DeclCXX.h"
#include "clang/AST/DeclFriend.h"
#include "clang/AST/DeclTemplate.h"
#include "clang/AST/ExprCXX.h"
#include "clang/AST/NestedNameSpecifier.h"
#include "clang/AST/Stmt.h"
#include "clang/AST/TemplateBase.h"
#include "clang/AST/TemplateName.h"
#include "clang/AST/Type.h"
#include "clang/AST/TypeLoc.h"
#include "clang/Basic/FileManager.h"
#include "clang/Basic/SourceLocation.h"

using clang::BlockPointerType;
using clang::CXXConstructExpr;
using clang::CXXConstructorDecl;
using clang::CXXDeleteExpr;
using clang::CXXDependentScopeMemberExpr;
using clang::CXXDestructorDecl;
using clang::CXXMethodDecl;
using clang::CXXRecordDecl;
using clang::CallExpr;
using clang::CastExpr;
using clang::ClassTemplateDecl;
using clang::ClassTemplatePartialSpecializationDecl;
using clang::ClassTemplateSpecializationDecl;
using clang::Decl;
using clang::DeclRefExpr;
using clang::DeclaratorDecl;
using clang::DependentNameType;
using clang::DependentScopeDeclRefExpr;
using clang::DependentTemplateName;
using clang::DependentTemplateSpecializationType;
using clang::ElaboratedType;
using clang::EnumDecl;
using clang::ExplicitCastExpr;
using clang::Expr;
using clang::ExprWithCleanups;
using clang::FileEntry;
using clang::FriendDecl;
using clang::FriendTemplateDecl;
using clang::FullSourceLoc;
using clang::FunctionDecl;
using clang::FunctionProtoType;
using clang::FunctionType;
using clang::ImplicitCastExpr;
using clang::InjectedClassNameType;
using clang::LValueReferenceType;
using clang::MemberExpr;
using clang::MemberPointerType;
using clang::NamedDecl;
using clang::NestedNameSpecifier;
using clang::NestedNameSpecifierLoc;
using clang::ObjCObjectType;
using clang::PointerType;
using clang::QualType;
using clang::QualifiedTemplateName;
using clang::RecordDecl;
using clang::RecordType;
using clang::SourceLocation;
using clang::SourceManager;
using clang::SourceRange;
using clang::Stmt;
using clang::SubstTemplateTypeParmType;
using clang::TagDecl;
using clang::TagType;
using clang::TemplateArgument;
using clang::TemplateArgumentList;
using clang::TemplateArgumentLoc;
using clang::TemplateDecl;
using clang::TemplateName;
using clang::TemplateParameterList;
using clang::TemplateSpecializationType;
using clang::Type;
using clang::TypeDecl;
using clang::TypeLoc;
using clang::TypedefType;
using clang::UnaryOperator;
using clang::UsingDirectiveDecl;
using clang::ValueDecl;
using llvm::PointerUnion;
using llvm::dyn_cast_or_null;
using llvm::errs;
using llvm::raw_string_ostream;

namespace include_what_you_use {

//------------------------------------------------------------
// ASTNode and associated utilities.

SourceLocation ASTNode::GetLocation() const {
  SourceLocation retval;
  if (FillLocationIfKnown(&retval))
    return retval;

  // OK, let's ask a parent node.
  for (const ASTNode* node = parent_; node != NULL; node = node->parent_) {
    if (node->FillLocationIfKnown(&retval))
      break;
  }
  // If the parent node shows the spelling and instantiation
  // locations are in a different file, then we're uncertain of our
  // own location.  Return an invalid location.
  if (retval.isValid()) {
    FullSourceLoc full_loc(retval, source_manager_);
    const FileEntry* spelling_file =
        source_manager_.getFileEntryForID(
            source_manager_.getFileID(full_loc.getSpellingLoc()));
    const FileEntry* instantiation_file =
        source_manager_.getFileEntryForID(
            source_manager_.getFileID(full_loc.getInstantiationLoc()));
    if (spelling_file != instantiation_file)
      return SourceLocation();
  }

  return retval;
}

bool ASTNode::FillLocationIfKnown(SourceLocation* loc) const {
  using include_what_you_use::GetLocation;
  switch (kind_) {
    case kDeclKind:
      *loc = GetLocation(as_decl_);   // in iwyu_location_util.h
      return true;
    case kStmtKind:
      *loc = GetLocation(as_stmt_);
      return true;
    case kTypelocKind:
      *loc = GetLocation(as_typeloc_);
      return true;
    case kNNSLocKind:
      *loc = GetLocation(as_nnsloc_);
      return true;
    case kTemplateArgumentLocKind:
      *loc = GetLocation(as_template_argloc_);
      return true;
    case kTypeKind:
    case kNNSKind:
    case kTemplateNameKind:
    case kTemplateArgumentKind:
      return false;
    default:
      CHECK_(false && "Unexpected kind of ASTNode");
      return false;
  }
}

// --- Utilities for ASTNode.

bool IsElaborationNode(const ASTNode* ast_node) {
  if (ast_node == NULL)
    return false;
  const ElaboratedType* elaborated_type = ast_node->GetAs<ElaboratedType>();
  return elaborated_type && elaborated_type->getKeyword() != clang::ETK_None;
}

bool IsNamespaceQualifiedNode(const ASTNode* ast_node) {
  if (ast_node == NULL)
    return false;
  const ElaboratedType* elaborated_type = ast_node->GetAs<ElaboratedType>();
  return (elaborated_type && elaborated_type->getQualifier()
          && elaborated_type->getQualifier()->getKind() ==
          NestedNameSpecifier::Namespace);
}

bool IsDeclNodeInsideFriend(const ASTNode* ast_node) {
  if (ast_node->ParentIsA<FriendDecl>() ||
      ast_node->ParentIsA<FriendTemplateDecl>())
    return true;

  // For 'template<class Foo> friend class X', 'class X's parent is
  // a ClassTemplateDecl.
  if (ast_node->ParentIsA<ClassTemplateDecl>() &&
      (ast_node->AncestorIsA<FriendDecl>(2) ||
       ast_node->AncestorIsA<FriendTemplateDecl>(2)))
    return true;

  return false;
}

bool IsNodeInsideCXXMethodBody(const ASTNode* ast_node) {
  // If we're a destructor, we're definitely part of a method body;
  // destructors don't have any other parts to them.  This case is
  // triggered when we see implicit destruction of member vars.
  if (ast_node && ast_node->IsA<CXXDestructorDecl>())
    return true;
  for (; ast_node != NULL; ast_node = ast_node->parent()) {
    // If we're a constructor, check if we're part of the
    // initializers, which also count as 'the body' of the method.
    if (const CXXConstructorDecl* ctor =
        ast_node->GetParentAs<CXXConstructorDecl>()) {
      for (CXXConstructorDecl::init_const_iterator
               it = ctor->init_begin(); it != ctor->init_end(); ++it) {
        if (ast_node->ContentIs((*it)->getInit()))
          return true;
      }
      // Now fall through to see if we're the body of the constructor.
    }
    if (const CXXMethodDecl* method_decl =
        ast_node->GetParentAs<CXXMethodDecl>()) {
      return ast_node->ContentIs(method_decl->getBody());
    }
  }
  return false;
}

bool IsDefaultTemplateTemplateArg(const ASTNode* ast_node) {
  // Is ast_node the 'D' in the following:
  //    template<template <typename A> class T = D> class C { ... }
  // ('D' might be something like 'vector').
  // D is a TemplateName, since it's a template, and its parent
  // is a TemplateArgument, since D is inside a template argument.
  // The only way a template name can be in a template argument
  // is if it's a default parameter.
  return (ast_node->IsA<TemplateName>() &&
          ast_node->ParentIsA<TemplateArgument>());
}

bool IsCXXConstructExprInInitializer(const ASTNode* ast_node) {
  if (!ast_node->IsA<CXXConstructExpr>())
    return false;

  CHECK_(ast_node->parent() && "Constructor should not be a top-level node!");

  // Typically, you can tell an initializer because its parent is a
  // constructor decl.  But sometimes -- I'm not exactly sure when --
  // there can be an ExprWithCleanups in the middle.
  return ((ast_node->ParentIsA<CXXConstructorDecl>()) ||
          (ast_node->ParentIsA<ExprWithCleanups>() &&
           ast_node->AncestorIsA<CXXConstructorDecl>(2)));
}

template<typename T>
NestedNameSpecifier* TryGetQualifier(const ASTNode* ast_node) {
  if (ast_node->IsA<T>())
    return ast_node->GetAs<T>()->getQualifier();
  return NULL;
}

const NestedNameSpecifier* GetQualifier(const ASTNode* ast_node) {
  const NestedNameSpecifier* nns = NULL;
  if (ast_node->IsA<TemplateName>()) {
    const TemplateName* tn = ast_node->GetAs<TemplateName>();
    if (const DependentTemplateName* dtn
        = tn->getAsDependentTemplateName())
      nns = dtn->getQualifier();
    else if (const QualifiedTemplateName* qtn
             = tn->getAsQualifiedTemplateName())
      nns = qtn->getQualifier();
  }
  if (!nns) nns = TryGetQualifier<ElaboratedType>(ast_node);
  if (!nns) nns = TryGetQualifier<DependentNameType>(ast_node);
  if (!nns)
    nns = TryGetQualifier<DependentTemplateSpecializationType>(ast_node);
  if (!nns) nns = TryGetQualifier<UsingDirectiveDecl>(ast_node);
  if (!nns) nns = TryGetQualifier<EnumDecl>(ast_node);
  if (!nns) nns = TryGetQualifier<RecordDecl>(ast_node);
  if (!nns) nns = TryGetQualifier<DeclaratorDecl>(ast_node);
  if (!nns) nns = TryGetQualifier<FunctionDecl>(ast_node);
  if (!nns) nns = TryGetQualifier<CXXDependentScopeMemberExpr>(ast_node);
  if (!nns) nns = TryGetQualifier<DeclRefExpr>(ast_node);
  if (!nns) nns = TryGetQualifier<DependentScopeDeclRefExpr>(ast_node);
  if (!nns) nns = TryGetQualifier<MemberExpr>(ast_node);
  return nns;
}

bool IsMemberOfATypedef(const ASTNode* ast_node) {
  // TODO(csilvers): is this ever triggered in practice?
  if (ast_node->ParentIsA<TypedefType>()) {      // my_typedef.a
    return true;
  }

  // If we're one of those objects that exposes its qualifier
  // (stuff before the ::), use that.
  const NestedNameSpecifier* nns = GetQualifier(ast_node);

  // If that doesn't work, see if our parent in the tree is an nns
  // node.  We have to be a bit careful here: 1) If we're a typedef
  // ourselves, the nns-parent is just us.  We have to go a level up
  // to see our 'real' qualifier.  2) Often the parent will be an
  // elaborated type, and we get to the qualifier that way.
  if (!nns) {
    nns = ast_node->GetParentAs<NestedNameSpecifier>();
    if (nns && ast_node->IsA<TypedefType>()) {
      nns = nns->getPrefix();
    } else if (!nns) {
      // nns will be non-NULL when processing 'a' in MyTypedef::a::b
      // But typically, such as processing 'a' in MyTypedef::a or 'b' in
      // MyTypedef::a::b, the parent will be an ElaboratedType.
      if (const ElaboratedType* elab_type =
          ast_node->GetParentAs<ElaboratedType>())
        nns = elab_type->getQualifier();
    }
  }

  for (; nns; nns = nns->getPrefix()) {
    if (nns->getAsType() && isa<TypedefType>(nns->getAsType()))
      return true;
  }
  return false;
}


//------------------------------------------------------------
// Helper functions for working with raw Clang AST nodes.

// --- Printers.

string PrintableLoc(SourceLocation loc) {
  if (loc.isInvalid()) {
    return "Invalid location";
  } else {
    std::string buffer;  // llvm wants regular string, not our versa-string
    raw_string_ostream ostream(buffer);
    loc.print(ostream, *GlobalSourceManager());
    return NormalizeFilePath(ostream.str());
  }
}

string PrintableSourceRange(SourceRange range) {
  return PrintableLoc(range.getBegin()) + " - " + PrintableLoc(range.getEnd());
}

string PrintableDecl(const Decl* decl) {
  std::string buffer;    // llvm wants regular string, not our versa-string
  raw_string_ostream ostream(buffer);
  decl->print(ostream);  // Note: can also set indentation and printingpolicy
  return ostream.str();
}

void PrintStmt(const Stmt* stmt) {
  stmt->dump(*GlobalSourceManager());  // This prints to errs().
}

string PrintableType(const Type* type) {
  return QualType(type, 0).getAsString();
}

string PrintableTypeLoc(const TypeLoc& typeloc) {
  return PrintableType(typeloc.getTypePtr());
}

string PrintableNestedNameSpecifier(
    const NestedNameSpecifier* nns) {
  std::string buffer;  // llvm wants regular string, not our versa-string
  raw_string_ostream ostream(buffer);
  nns->print(ostream, DefaultPrintPolicy());
  return ostream.str();
}

string PrintableTemplateName(const TemplateName& tpl_name) {
  std::string buffer;    // llvm wants regular string, not our versa-string
  raw_string_ostream ostream(buffer);
  tpl_name.print(ostream, DefaultPrintPolicy());
  return ostream.str();
}

string PrintableTemplateArgument(const TemplateArgument& arg) {
  return TemplateSpecializationType::PrintTemplateArgumentList(
      &arg, 1, DefaultPrintPolicy());
}

string PrintableTemplateArgumentLoc(
    const TemplateArgumentLoc& arg) {
  return TemplateSpecializationType::PrintTemplateArgumentList(
      &arg, 1, DefaultPrintPolicy());
}

// This prints to errs().  It's useful for debugging (e.g. inside gdb).
void PrintASTNode(const ASTNode* node) {
  if (const Decl* decl = node->GetAs<Decl>()) {
    errs() << "[" << decl->getDeclKindName() << "Decl] "
                 << PrintableDecl(decl) << "\n";
  } else if (const Stmt* stmt = node->GetAs<Stmt>()) {
    errs() << "[" << stmt->getStmtClassName() << "] ";
    PrintStmt(stmt);
    errs() << "\n";
  } else if (const Type* type = node->GetAs<Type>()) { // +typeloc
    errs() << "[" << type->getTypeClassName()
                 << (node->IsA<TypeLoc>() ? "TypeLoc" : "Type") << "] "
                 << PrintableType(type) << "\n";
  } else if (const NestedNameSpecifier* nns
             = node->GetAs<NestedNameSpecifier>()) {
    errs() << "[NestedNameSpecifier] "
                 << PrintableNestedNameSpecifier(nns) << "\n";
  } else if (const TemplateName* tpl_name
             = node->GetAs<TemplateName>()) {
    errs() << "[TemplateName] "
                 << PrintableTemplateName(*tpl_name) << "\n";
  } else if (const TemplateArgumentLoc* tpl_argloc
             = node->GetAs<TemplateArgumentLoc>()) {
    errs() << "[TemplateArgumentLoc] "
                 << PrintableTemplateArgumentLoc(*tpl_argloc) << "\n";
  } else if (const TemplateArgument* tpl_arg
             = node->GetAs<TemplateArgument>()) {
    errs() << "[TemplateArgument] "
                 << PrintableTemplateArgument(*tpl_arg) << "\n";
  } else {
    CHECK_(!"Unknown kind for ASTNode");
  }
}

// --- Utilities for Template Arguments.

// These utilities figure out the template arguments that are
// specified in various contexts: TemplateSpecializationType (for
// template classes) and FunctionDecl (for template functions).
//
// For classes, we care only about explicitly specified template args,
// not implicit, default args.  For functions, we care about all
// template args, since if not specified they're derived from the
// function arguments.  In either case, we only care about template
// arguments that are types (including template types), not other
// kinds of arguments such as built-in types.

// Helper function to the functions below.
// TODO(csilvers): come up with a better name?  Or refactor?
void AddTypelikeTemplateArgTo(const TemplateArgument& tpl_arg,
                                     set<const Type*>* argset) {
  if (tpl_arg.getKind() == TemplateArgument::Type) {
    // Holds all types seen in tpl_arg (may be more than one if tpl_arg
    // is a function prototype, with argument-types and a return-type).
    set<const Type*> argtypes;
    argtypes.insert(tpl_arg.getAsType().getTypePtr());
    // If the type is a function (a rare case, but happens in code like
    // TplClass<char(int, int, int)>), then the parameters are types
    // we have to consider as well.
    // TODO(csilvers): also check a fn pointer and a fn taking a fn ptr:
    // TplClass<char(*)(int, int, int)>, TplClass<char(char(*)(int, int))>
    if (const FunctionProtoType* fn_type
        = DynCastFrom(tpl_arg.getAsType().getTypePtr())) {
      argtypes.insert(fn_type->getResultType().getTypePtr());
      for (unsigned i = 0; i < fn_type->getNumArgs(); ++i) {
        argtypes.insert(fn_type->getArgType(i).getTypePtr());
      }
      // I *think* it's correct to ignore the exception specs here.
    }

    for (Each<const Type*> it(&argtypes); !it.AtEnd(); ++it) {
      VERRS(6) << "Adding a template type of interest: "
               << PrintableType(*it) << "\n";
      argset->insert(*it);
      // Recurse if we ourself are a template type.  Read through elaborations.
      const Type* subtype = RemoveElaboration(*it);
      if (const TemplateSpecializationType* tpl_type = DynCastFrom(subtype)) {
        for (unsigned i = 0; i < tpl_type->getNumArgs(); ++i)
          AddTypelikeTemplateArgTo(tpl_type->getArg(i), argset);
      }
    }
  } else if (tpl_arg.getKind() == TemplateArgument::Template) {
    const TemplateName& tpl_name = tpl_arg.getAsTemplate();
    VERRS(6) << "Noticing (but ignoring) a template template of interest: "
             << PrintableTemplateName(tpl_name) << "\n";
    // TODO(csilvers): add tpl_name to argset somehow.  This is a
    // lower-priority TODO, since for the moment we just always
    // assume template template args needs to be fully instantiated.
  }
}

// --- Utilities for Decl.

bool IsTemplatizedFunctionDecl(const FunctionDecl* decl) {
  return decl && decl->getTemplateSpecializationArgs() != NULL;
}

bool HasImplicitConversionCtor(const CXXRecordDecl* cxx_class) {
  for (CXXRecordDecl::ctor_iterator ctor = cxx_class->ctor_begin();
       ctor != cxx_class->ctor_end(); ++ctor) {
    if ((*ctor)->isExplicit() || (*ctor)->getNumParams() != 1 ||
        (*ctor)->isCopyConstructor())
      continue;
    return true;
  }
  return false;
}

const RecordDecl* GetDefinitionForClass(const Decl* decl) {
  const RecordDecl* as_record = DynCastFrom(decl);
  const ClassTemplateDecl* as_tpl = DynCastFrom(decl);
  if (as_tpl)   // Convert the template to its underlying class defn.
    as_record = dyn_cast_or_null<RecordDecl>(as_tpl->getTemplatedDecl());
  if (as_record) {
    if (const RecordDecl* record_dfn = as_record->getDefinition())
      return record_dfn;
  }
  return NULL;
}

SourceRange GetSourceRangeOfClassDecl(const Decl* decl) {
  // If we're a templatized class, go 'up' a level to get the
  // template<...> prefix as well.
  if (const CXXRecordDecl* cxx_decl = DynCastFrom(decl)) {
    if (cxx_decl->getDescribedClassTemplate())
      return cxx_decl->getDescribedClassTemplate()->getSourceRange();
  }
  // We can get source ranges of classes and template classes.
  if (const TagDecl* tag_decl = DynCastFrom(decl))
    return tag_decl->getSourceRange();
  if (const TemplateDecl* tpl_decl = DynCastFrom(decl))
    return tpl_decl->getSourceRange();
  CHECK_(!"Cannot get source range for this decl type");
  return SourceRange();
}

set<const Type*> GetTplTypeArgsOfFunction(const FunctionDecl* decl) {
  set<const Type*> retval;
  if (!decl)
    return retval;
  const TemplateArgumentList* tpl_list =
      decl->getTemplateSpecializationArgs();
  if (tpl_list) {
    for (unsigned i = 0; i < tpl_list->size(); ++i) {
      AddTypelikeTemplateArgTo(tpl_list->get(i), &retval);
    }
    // TODO(csilvers): for derived tpl args (Foo(arg), not
    // Foo<type>(arg)), remove types from retval if they don't appear
    // somewhere in a function arg (or return value) as typed.  This
    // is a simple heuristic for dealing with typedefs and default
    // arguments.  For instance, cout << "a" should not yield
    // char_traits<char> here, since the first argument of operator<<
    // is typed ostream, not basic_ostream<char, char_traits<char> >.
  }
  return retval;
}

const NamedDecl* GetInstantiatedFromDecl(const CXXRecordDecl* class_decl) {
  if (const ClassTemplateSpecializationDecl* tpl_sp_decl =
      DynCastFrom(class_decl)) {  // an instantiated class template
    PointerUnion<ClassTemplateDecl*, ClassTemplatePartialSpecializationDecl*>
        instantiated_from = tpl_sp_decl->getInstantiatedFrom();
    if (const ClassTemplateDecl* tpl_decl =
        instantiated_from.dyn_cast<ClassTemplateDecl*>()) {
      // class_decl is instantiated from a non-specialized template.
      return tpl_decl;
    } else if (const ClassTemplatePartialSpecializationDecl*
               partial_spec_decl =
               instantiated_from.dyn_cast<
                   ClassTemplatePartialSpecializationDecl*>()) {
      // class_decl is instantiated from a template partial specialization.
      return partial_spec_decl;
    }
  }
  // class_decl is not instantiated from a template.
  return class_decl;
}

const NamedDecl* GetDefinitionAsWritten(const NamedDecl* decl) {
  // First, get to decl-as-written.
  if (const CXXRecordDecl* class_decl = DynCastFrom(decl)) {
    decl = GetInstantiatedFromDecl(class_decl);
    if (const ClassTemplateDecl* tpl_decl = DynCastFrom(decl))
      decl = tpl_decl->getTemplatedDecl();  // convert back to CXXRecordDecl
  } else if (const FunctionDecl* func_decl = DynCastFrom(decl)) {
    if (const FunctionDecl* tpl_pattern =
        func_decl->getTemplateInstantiationPattern())
      decl = tpl_pattern;
  }
  // Then, get to definition.
  if (const RecordDecl* class_dfn = GetDefinitionForClass(decl)) {
    // If we started this fn as a template, convert back to a template now.
    if (const CXXRecordDecl* cxx_class_dfn = DynCastFrom(class_dfn)) {
      if (cxx_class_dfn->getDescribedClassTemplate())
        return cxx_class_dfn->getDescribedClassTemplate();
    }
    return class_dfn;
  } else if (const FunctionDecl* fn_decl = DynCastFrom(decl)) {
    for (FunctionDecl::redecl_iterator it = fn_decl->redecls_begin();
         it != fn_decl->redecls_end(); ++it) {
      if ((*it)->isThisDeclarationADefinition())
        return *it;
    }
  }
  // Couldn't find a definition, just return the original declaration.
  return decl;
}

bool IsDefaultNewOrDelete(const FunctionDecl* decl,
                          const string& decl_loc_as_quoted_include) {
  // Clang will report <new> as the location of the default new and
  // delete operators if <new> is included. Otherwise, it reports the
  // (fake) file "<built_in>".
  if (decl_loc_as_quoted_include != "<new>" &&
      !IsBuiltinFile(GetFileEntry(decl)))
    return false;

  const string decl_name = decl->getNameAsString();
  if (!StartsWith(decl_name, "operator new") &&
      !StartsWith(decl_name, "operator delete"))
    return false;

  // Placement-new/delete has 2 args, second is void*.  The only other
  // 2-arg overloads of new/delete in <new> take a const nothrow_t&.
  if (decl->getNumParams() == 2 &&
      !decl->getParamDecl(1)->getType().isConstQualified())
    return false;

  return true;
}

bool IsFriendDecl(const Decl* decl) {
  // For 'template<...> friend class T', the decl will just be 'class T'.
  // We need to go 'up' a level to check friendship in the right place.
  if (const CXXRecordDecl* cxx_decl = DynCastFrom(decl))
    if (cxx_decl->getDescribedClassTemplate())
      decl = cxx_decl->getDescribedClassTemplate();
  return decl->getFriendObjectKind() != Decl::FOK_None;
}

bool HasDefaultTemplateParameters(const TemplateDecl* decl) {
  TemplateParameterList* tpl_params = decl->getTemplateParameters();
  return tpl_params->getMinRequiredArguments() < tpl_params->size();
}

set<const RecordDecl*> GetClassRedecls(const RecordDecl* decl) {
  set<const RecordDecl*> redecls;
  for (TagDecl::redecl_iterator it = decl->redecls_begin();
       it != decl->redecls_end(); ++it) {
    const RecordDecl* redecl = cast<RecordDecl>(*it);
    // If this decl is a friend decl, don't count it: friend decls
    // don't serve as forward-declarations.  (This should never
    // happen, I think, but it sometimes does due to a clang bug:
    // http://llvm.org/bugs/show_bug.cgi?id=8669)
    if (!IsFriendDecl(redecl))
      redecls.insert(redecl);
  }
  return redecls;
}

const NamedDecl* GetNonfriendClassRedecl(const NamedDecl* decl) {
  const RecordDecl* record_decl = DynCastFrom(decl);
  const ClassTemplateDecl* tpl_decl = DynCastFrom(decl);
  if (tpl_decl)
    record_decl = tpl_decl->getTemplatedDecl();
  if (!record_decl || !IsFriendDecl(record_decl))
    return decl;

  const set<const RecordDecl*> redecls = GetClassRedecls(record_decl);
  CHECK_(!redecls.empty() && "Should be at least once 'real' decl");
  const RecordDecl* retval = *redecls.begin();  // arbitrary choice

  if (tpl_decl) {   // need to convert back to a ClassTemplateDecl
    CHECK_(isa<CXXRecordDecl>(retval) &&
           cast<CXXRecordDecl>(retval)->getDescribedClassTemplate());
    const CXXRecordDecl* cxx_decl = cast<CXXRecordDecl>(retval);
    return cxx_decl->getDescribedClassTemplate();
  }
  return retval;
}

bool DeclsAreInSameClass(const Decl* decl1, const Decl* decl2) {
  if (!decl1 || !decl2)
    return false;
  if (decl1->getDeclContext() != decl2->getDeclContext())
    return false;
  return decl1->getDeclContext()->isRecord();
}


// --- Utilities for Type.

const Type* GetTypeOf(const Expr* expr) {
  return expr->getType().getTypePtr();
}

const Type* GetTypeOf(const ValueDecl* decl) {
  return decl->getType().getTypePtr();
}

const Type* GetTypeOf(const TypeDecl* decl) {
  return decl->getTypeForDecl();
}

const Type* RemoveElaboration(const Type* type) {
  if (const ElaboratedType* elaborated_type = DynCastFrom(type))
    return elaborated_type->getNamedType().getTypePtr();
  else
    return type;
}

bool IsTemplatizedType(const Type* type) {
  return (type && isa<TemplateSpecializationType>(RemoveElaboration(type)));
}

const Type* RemoveSubstTemplateTypeParm(const Type* type) {
  if (const SubstTemplateTypeParmType* subst_type = DynCastFrom(type))
    return subst_type->getReplacementType().getTypePtr();
  else
    return type;
}

bool IsPointerOrReferenceAsWritten(const Type* type) {
  type = RemoveElaboration(type);
  return isa<PointerType>(type) || isa<LValueReferenceType>(type);
}

const Type* RemovePointersAndReferencesAsWritten(const Type* type) {
  type = RemoveElaboration(type);
  while (isa<PointerType>(type) ||
         isa<LValueReferenceType>(type)) {
    type = type->getPointeeType().getTypePtr();
  }
  return type;
}

const Type* RemovePointerFromType(const Type* type) {
  if (!IsPointerOrReferenceAsWritten(type)) {   // ah well, have to desugar
    type = type->getUnqualifiedDesugaredType();
  }
  if (!IsPointerOrReferenceAsWritten(type)) {
    return NULL;
  }
  type = RemoveElaboration(type);
  type = type->getPointeeType().getTypePtr();
  return type;
}

// This follows typedefs/etc to remove pointers, if necessary.
const Type* RemovePointersAndReferences(const Type* type) {
  while (1) {
    const Type* deref_type = RemovePointerFromType(type);
    if (deref_type == NULL)   // type wasn't a pointer (or reference) type
      break;                  // removed all pointers
    type = deref_type;
  }
  return type;
}

const NamedDecl* TypeToDeclAsWritten(const Type* type) {
  // Get past all the 'class' and 'struct' prefixes, and namespaces.
  type = RemoveElaboration(type);

  // Read past SubstTemplateTypeParmType (this can happen if a
  // template function returns the tpl-arg type: e.g. for
  // 'T MyFn<T>() {...}; MyFn<X>.a', the type of MyFn<X> will be a Subst.
  type = RemoveSubstTemplateTypeParm(type);

  CHECK_(!isa<ObjCObjectType>(type) && "IWYU doesn't support Objective-C");

  // We have to be a bit careful about the order, because we want
  // to keep typedefs as typedefs, so we do the record check last.
  // We use getAs<> when we can -- unfortunately, it only exists
  // for a few types so far.
  if (const TypedefType* typedef_type = DynCastFrom(type)) {
    return typedef_type->getDecl();
  } else if (const InjectedClassNameType* icn_type
             = type->getAs<InjectedClassNameType>()) {
    return icn_type->getDecl();
  } else if (const RecordType* record_type
             = type->getAs<RecordType>()) {
    return record_type->getDecl();
  } else if (const TagType* tag_type = DynCastFrom(type)) {
    return tag_type->getDecl();    // probably just enums
  } else if (const TemplateSpecializationType* template_spec_type
             = DynCastFrom(type)) {
    // A non-concrete template class, such as 'Myclass<T>'
    return template_spec_type->getTemplateName().getAsTemplateDecl();
  } else if (const FunctionType* function_type = DynCastFrom(type)) {
    // TODO(csilvers): is it possible to map from fn type to fn decl?
    (void)function_type;
    return NULL;
  }  else {
    return NULL;
  }
}

const Type* RemoveReferenceAsWritten(const Type* type) {
  if (const LValueReferenceType* ref_type = DynCastFrom(type))
    return ref_type->getPointeeType().getTypePtr();
  else
    return type;
}

bool CanImplicitlyConvertTo(const Type* type) {
  type = RemoveElaboration(type);  // get rid of the class keyword
  if (isa<PointerType>(type))
    return false;  // can't implicitly convert to a pointer
  if (isa<LValueReferenceType>(type) &&
      !type->getPointeeType().isConstQualified())
    return false;  // can't implicitly convert to a non-const reference

  type = RemoveReferenceAsWritten(type);
  const NamedDecl* decl = TypeToDeclAsWritten(type);
  if (!decl)  // not the kind of type that has a decl (e.g. built-in)
    return false;

  const CXXRecordDecl* cxx_class = DynCastFrom(decl);
  if (!cxx_class)
    return false;  // can't implicitly convert to a non-class type

  return HasImplicitConversionCtor(cxx_class);
}

set<const Type*> GetExplicitTplTypeArgsOf(const Type* type) {
  set<const Type*> retval;
  type = RemoveElaboration(type);  // get rid of the class keyword
  const TemplateSpecializationType* tpl_spec_type = DynCastFrom(type);
  if (!tpl_spec_type)
    return retval;
  // TemplateSpecializationType only includes explicitly specified
  // types in its args list, which is just what we want.
  for (unsigned i = 0; i < tpl_spec_type->getNumArgs(); ++i) {
    AddTypelikeTemplateArgTo(tpl_spec_type->getArg(i), &retval);
  }
  return retval;
}

// --- Utilities for Stmt.

bool IsAddressOf(const Expr* expr) {
  if (const UnaryOperator* unary = DynCastFrom(expr->IgnoreParens()))
    return unary->getOpcode() == clang::UO_AddrOf;
  return false;
}

const Type* TypeOfParentIfMethod(const CallExpr* expr) {
  // callee_expr is a MemberExpr if we're a normal class method, or
  // DeclRefExpr if we're a static class method or an overloaded operator.
  const Expr* callee_expr = expr->getCallee()->IgnoreParenCasts();
  if (const MemberExpr* member_expr = DynCastFrom(callee_expr)) {
    const Type* class_type = GetTypeOf(member_expr->getBase());
    // For class->member(), class_type is a pointer.
    return RemovePointersAndReferencesAsWritten(class_type);
  } else if (const DeclRefExpr* ref_expr = DynCastFrom(callee_expr)) {
    if (ref_expr->getQualifier()) {    // static methods like C<T>::a()
      return ref_expr->getQualifier()->getAsType();
    }
  }
  return NULL;
}

const Expr* GetFirstClassArgument(CallExpr* expr) {
  for (CallExpr::arg_iterator it = expr->arg_begin();
       it != expr->arg_end(); ++it) {
    const Type* argtype = GetTypeOf(*it);
    // Make sure we do the right thing given a function like
    //    template <typename T> void operator>>(const T& x, ostream& os);
    // In this case ('myclass >> os'), we want to be returning the
    // type of os, not of myclass, and we do, because myclass will be
    // a SubstTemplateTypeParmType, not a RecordType.
    if (isa<SubstTemplateTypeParmType>(argtype))
      continue;
    argtype = argtype->getUnqualifiedDesugaredType();  // see through typedefs
    if (isa<RecordType>(argtype) ||
        isa<TemplateSpecializationType>(argtype)) {
      return *it;
    }
  }
  return NULL;
}

const CXXDestructorDecl* GetDestructorForDeleteExpr(const CXXDeleteExpr* expr) {
  const Type* type = expr->getDestroyedType().getTypePtrOrNull();
  // type is NULL when deleting a dependent type: 'T foo; delete foo'
  if (type == NULL)
    return NULL;
  const NamedDecl* decl = TypeToDeclAsWritten(type);
  if (const CXXRecordDecl* cxx_record = DynCastFrom(decl))
    return cxx_record->getDestructor();
  return NULL;
}

const CXXDestructorDecl* GetSiblingDestructorFor(
    const CXXConstructorDecl* ctor) {
  return ctor ? ctor->getParent()->getDestructor() : NULL;
}

const CXXDestructorDecl* GetSiblingDestructorFor(
    const CXXConstructExpr* ctor_expr) {
  return GetSiblingDestructorFor(ctor_expr->getConstructor());
}

const FunctionType* GetCalleeFunctionType(CallExpr* expr) {
  const Type* callee_type = expr->getCallee()->getType().getTypePtr();
  if (const PointerType* ptr_type
      = callee_type->getAs<PointerType>()) {
    callee_type = ptr_type->getPointeeType().getTypePtr();
  } else if (const BlockPointerType* bptr_type
             = callee_type->getAs<BlockPointerType>()) {
    callee_type = bptr_type->getPointeeType().getTypePtr();
  } else if (const MemberPointerType* mptr_type
             = callee_type->getAs<MemberPointerType>()) {
    callee_type = mptr_type->getPointeeType().getTypePtr();
  }
  return callee_type->getAs<FunctionType>();
}

bool IsCastToReferenceType(const CastExpr* expr) {
  if (const ExplicitCastExpr* explicit_cast = DynCastFrom(expr)) {
    return explicit_cast->getTypeAsWritten()->isReferenceType();
  } else if (const ImplicitCastExpr* implicit_cast = DynCastFrom(expr)) {
    return implicit_cast->getValueKind() == clang::VK_LValue;
  } else {
    CHECK_(false && "Unexpected type of cast expression");
    return false;
  }
}

}  // namespace include_what_you_use
