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

#include <set>                          // for set
#include <string>                       // for string, operator+, etc
#include <utility>                      // for pair

#include "iwyu_globals.h"
#include "iwyu_location_util.h"
#include "iwyu_path_util.h"
#include "iwyu_stl_util.h"
#include "iwyu_string_util.h"
#include "iwyu_verrs.h"
#include "port.h"  // for CHECK_
#include "llvm/Support/Casting.h"
#include "llvm/Support/raw_ostream.h"
#include "clang/AST/Decl.h"
#include "clang/AST/DeclBase.h"
#include "clang/AST/DeclCXX.h"
#include "clang/AST/DeclTemplate.h"
#include "clang/AST/ExprCXX.h"
#include "clang/AST/NestedNameSpecifier.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/AST/Stmt.h"
#include "clang/AST/TemplateBase.h"
#include "clang/AST/TemplateName.h"
#include "clang/AST/Type.h"
#include "clang/AST/TypeLoc.h"
#include "clang/Basic/SourceLocation.h"
#include "clang/Basic/SourceManager.h"

namespace clang {
class FileEntry;
}  // namespace clang

using clang::ASTTemplateArgumentListInfo;
using clang::BlockPointerType;
using clang::CXXConstructExpr;
using clang::CXXConstructorDecl;
using clang::CXXDeleteExpr;
using clang::CXXDependentScopeMemberExpr;
using clang::CXXDestructorDecl;
using clang::CXXMethodDecl;
using clang::CXXNewExpr;
using clang::CXXRecordDecl;
using clang::CallExpr;
using clang::CastExpr;
using clang::ClassTemplateDecl;
using clang::ClassTemplatePartialSpecializationDecl;
using clang::ClassTemplateSpecializationDecl;
using clang::Decl;
using clang::DeclContext;
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
using clang::OverloadExpr;
using clang::PointerType;
using clang::QualType;
using clang::QualifiedTemplateName;
using clang::RecordDecl;
using clang::RecordType;
using clang::RecursiveASTVisitor;
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
using clang::TypedefNameDecl;
using clang::TypedefType;
using clang::UnaryOperator;
using clang::UsingDirectiveDecl;
using clang::ValueDecl;
using clang::VarDecl;
using llvm::PointerUnion;
using llvm::cast;
using llvm::dyn_cast_or_null;
using llvm::errs;
using llvm::isa;
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
            source_manager_.getFileID(full_loc.getExpansionLoc()));
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
      if (ast_node->ContentIs(method_decl->getBody())) {
        return true;
      }
    }
  }
  return false;
}

bool IsNestedClassAsWritten(const ASTNode* ast_node) {
  return (ast_node->IsA<RecordDecl>() &&
          (ast_node->ParentIsA<CXXRecordDecl>() ||
           // For templated nested-classes, a ClassTemplateDecl is interposed.
           (ast_node->ParentIsA<ClassTemplateDecl>() &&
            ast_node->AncestorIsA<CXXRecordDecl>(2))));
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

const DeclContext* GetDeclContext(const ASTNode* ast_node) {
  for (; ast_node != NULL; ast_node = ast_node->parent()) {
    if (ast_node->IsA<Decl>())
      return ast_node->GetAs<Decl>()->getDeclContext();
  }
  return NULL;
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
    CHECK_(false && "Unknown kind for ASTNode");
  }
}

// --- Utilities for Template Arguments.

// If the TemplateArgument is a type (and not an expression such as
// 'true', or a template such as 'vector', etc), return it.  Otherwise
// return NULL.
static const Type* GetTemplateArgAsType(const TemplateArgument& tpl_arg) {
  if (tpl_arg.getKind() == TemplateArgument::Type)
    return tpl_arg.getAsType().getTypePtr();
  return NULL;
}

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

// This helper class visits a given AST node and finds all the types
// beneath it, which it returns as a set.  For example, if you have a
// VarDecl 'vector<int(*)(const MyClass&)> x', it would return
// (vector<int(*)(const MyClass&)>, int(*)(const MyClass&),
// int(const MyClass&), int, const MyClass&, MyClass).  Note that
// this function only returns types-as-typed, so it does *not* return
// alloc<int(*)(const MyClass&)>, even though it's part of vector.
class TypeEnumerator : public RecursiveASTVisitor<TypeEnumerator> {
 public:
  typedef RecursiveASTVisitor<TypeEnumerator> Base;

  // --- Public interface
  // We can add more entry points as needed.
  set<const Type*> Enumerate(const Type* type) {
    seen_types_.clear();
    if (!type)
      return seen_types_;
    TraverseType(QualType(type, 0));
    return seen_types_;
  }

  set<const Type*> Enumerate(const TemplateArgument& tpl_arg) {
    seen_types_.clear();
    TraverseTemplateArgument(tpl_arg);
    return seen_types_;
  }

  // --- Methods on RecursiveASTVisitor
  bool VisitType(Type* type) {
    seen_types_.insert(type);
    return true;
  }

 private:
  set<const Type*> seen_types_;
};

// A 'component' of a type is a type beneath it in the AST tree.
// So 'Foo*' has component 'Foo', as does 'vector<Foo>', while
// vector<pair<Foo, Bar> > has components pair<Foo,Bar>, Foo, and Bar.
set<const Type*> GetComponentsOfType(const Type* type) {
  TypeEnumerator type_enumerator;
  return type_enumerator.Enumerate(type);
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
  if (as_tpl) {  // Convert the template to its underlying class defn.
    as_record = dyn_cast_or_null<RecordDecl>(as_tpl->getTemplatedDecl());
  }
  if (as_record) {
    if (const RecordDecl* record_dfn = as_record->getDefinition()) {
      return record_dfn;
    }
    // If we're a templated class that was never instantiated (because
    // we were never "used"), then getDefinition() will return NULL.
    if (const ClassTemplateSpecializationDecl* spec_decl = DynCastFrom(decl)) {
      PointerUnion<ClassTemplateDecl*,
          ClassTemplatePartialSpecializationDecl*>
          specialized_decl = spec_decl->getSpecializedTemplateOrPartial();
      if (const ClassTemplatePartialSpecializationDecl*
          partial_spec_decl =
          specialized_decl.dyn_cast<
          ClassTemplatePartialSpecializationDecl*>()) {
        // decl would be instantiated from a template partial
        // specialization.
        CHECK_(partial_spec_decl->hasDefinition());
        return partial_spec_decl->getDefinition();
      } else if (const ClassTemplateDecl* tpl_decl =
                 specialized_decl.dyn_cast<ClassTemplateDecl*>()) {
        // decl would be instantiated from a non-specialized
        // template.
        if (tpl_decl->getTemplatedDecl()->hasDefinition())
          return tpl_decl->getTemplatedDecl()->getDefinition();
      }
    }
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
  CHECK_(false && "Cannot get source range for this decl type");
  return SourceRange();
}

// Helper for the Get*ResugarMap*() functions.  Given a map from
// desugared->resugared types, looks at each component of the
// resugared type (eg, both hash_set<Foo>* and vector<hash_set<Foo> >
// have two components: hash_set<Foo> and Foo), and returns a map that
// contains the original map elements plus mapping for the components.
// This is because when a type is 'owned' by the template
// instantiator, all parts of the type are owned.  We only consider
// type-components as typed.
static map<const Type*, const Type*> ResugarTypeComponents(
    const map<const Type*, const Type*>& resugar_map) {
  map<const Type*, const Type*> retval = resugar_map;
  for (Each<const Type*, const Type*> it(&resugar_map); !it.AtEnd(); ++it) {
    const set<const Type*>& components = GetComponentsOfType(it->second);
    for (Each<const Type*> component_type(&components);
         !component_type.AtEnd(); ++component_type) {
      const Type* desugared_type = GetCanonicalType(*component_type);
      if (!ContainsKey(retval, desugared_type)) {
        retval[desugared_type] = *component_type;
        VERRS(6) << "Adding a type-components of interest: "
                 << PrintableType(*component_type) << "\n";
      }
    }
  }
  return retval;
}

// Helpers for GetTplTypeResugarMapForFunction().
static map<const Type*, const Type*> GetTplTypeResugarMapForFunctionNoCallExpr(
    const FunctionDecl* decl, unsigned start_arg) {
  map<const Type*, const Type*> retval;
  if (!decl)   // can be NULL if the function call is via a function pointer
    return retval;
  if (const TemplateArgumentList* tpl_list
      = decl->getTemplateSpecializationArgs()) {
    for (unsigned i = start_arg; i < tpl_list->size(); ++i) {
      if (const Type* arg_type = GetTemplateArgAsType(tpl_list->get(i))) {
        retval[GetCanonicalType(arg_type)] = arg_type;
        VERRS(6) << "Adding an implicit tpl-function type of interest: "
                 << PrintableType(arg_type) << "\n";
      }
    }
  }
  return retval;
}

static map<const Type*, const Type*>
GetTplTypeResugarMapForFunctionExplicitTplArgs(
    const FunctionDecl* decl,
    const ASTTemplateArgumentListInfo* explicit_tpl_list) {
  map<const Type*, const Type*> retval;
  if (explicit_tpl_list) {
    for (unsigned i = 0; i < explicit_tpl_list->NumTemplateArgs; ++i) {
      const TemplateArgument& arg
          = explicit_tpl_list->getTemplateArgs()[i].getArgument();
      if (const Type* arg_type = GetTemplateArgAsType(arg)) {
        retval[GetCanonicalType(arg_type)] = arg_type;
        VERRS(6) << "Adding an explicit template-function type of interest: "
                 << PrintableType(arg_type) << "\n";
      }
    }
  }
  return retval;
}

map<const Type*, const Type*> GetTplTypeResugarMapForFunction(
    const FunctionDecl* decl, const Expr* calling_expr) {
  map<const Type*, const Type*> retval;

  // If calling_expr is NULL, then we can't find any explicit template
  // arguments, if they were specified (e.g. 'Fn<int>()'), and we
  // won't be able to get the function arguments as written.  So we
  // can't resugar at all.  We just have to hope that the types happen
  // to be already sugared, because the actual-type is already canonical.
  if (calling_expr == NULL) {
    retval = GetTplTypeResugarMapForFunctionNoCallExpr(decl, 0);
    retval = ResugarTypeComponents(retval);  // add in retval's decomposition
    return retval;
  }

  // If calling_expr is a CXXConstructExpr of CXXNewExpr, then it's
  // impossible to explicitly specify template arguments; all we have
  // to go on is function arguments.  If it's a CallExpr, and some
  // arguments might be explicit, and others implicit.  Otherwise,
  // it's a type that doesn't take function template args at all (like
  // CXXDeleteExpr) or only takes explicit args (like DeclRefExpr).
  Expr** fn_args = NULL;
  unsigned num_args = 0;
  unsigned start_of_implicit_args = 0;
  if (const CXXConstructExpr* ctor_expr = DynCastFrom(calling_expr)) {
    fn_args = ctor_expr->getArgs();
    num_args = ctor_expr->getNumArgs();
  } else if (const CallExpr* call_expr = DynCastFrom(calling_expr)) {
    fn_args = const_cast<CallExpr*>(call_expr)->getArgs();
    num_args = call_expr->getNumArgs();
    const Expr* callee_expr = call_expr->getCallee()->IgnoreParenCasts();
    if (const ASTTemplateArgumentListInfo* explicit_tpl_args
        = GetExplicitTplArgs(callee_expr)) {
      retval = GetTplTypeResugarMapForFunctionExplicitTplArgs(
          decl, explicit_tpl_args);
      start_of_implicit_args = explicit_tpl_args->NumTemplateArgs;
    }
  } else {
    // If calling_expr has explicit template args, then consider them.
    if (const ASTTemplateArgumentListInfo* explicit_tpl_args
        = GetExplicitTplArgs(calling_expr)) {
      retval = GetTplTypeResugarMapForFunctionExplicitTplArgs(
          decl, explicit_tpl_args);
      retval = ResugarTypeComponents(retval);
    }
    return retval;
  }

  // Now we have to figure out, as best we can, the sugar-mappings for
  // compiler-deduced template args.  We do this by looking at every
  // type specified in any part of the function arguments as written.
  // If any of these types matches a template type, then we take that
  // to be the resugar mapping.  If none of the types match, then we
  // assume that the template is matching some desugared part of the
  // type, and we ignore it.  For instance:
  //     operator<<(basic_ostream<char, T>& o, int i);
  // If I pass in an ostream as the first argument, then no part
  // of the (sugared) argument types match T, so we ignore it.
  const map<const Type*, const Type*>& desugared_types
      = GetTplTypeResugarMapForFunctionNoCallExpr(decl, start_of_implicit_args);

  // TODO(csilvers): SubstTemplateTypeParms are always desugared,
  //                 making this less useful than it should be.
  // TODO(csilvers): if the GetArg(i) expr has an implicit cast
  //                 under it, take the pre-cast type instead?
  set<const Type*> fn_arg_types;
  for (unsigned i = 0; i < num_args; ++i) {
    const Type* argtype = GetTypeOf(fn_args[i]);
    // TODO(csilvers): handle RecordTypes that are a TemplateSpecializationDecl
    InsertAllInto(GetComponentsOfType(argtype), &fn_arg_types);
  }

  for (Each<const Type*> it(&fn_arg_types); !it.AtEnd(); ++it) {
    // See if any of the template args in retval are the desugared form of us.
    const Type* desugared_type = GetCanonicalType(*it);
    if (ContainsKey(desugared_types, desugared_type)) {
      retval[desugared_type] = *it;
      if (desugared_type != *it) {
        VERRS(6) << "Remapping template arg of interest: "
                 << PrintableType(desugared_type) << " -> "
                 << PrintableType(*it) << "\n";
      }
    }
  }

  // Log the types we never mapped.
  for (Each<const Type*, const Type*> it(&desugared_types); !it.AtEnd(); ++it) {
    if (!ContainsKey(retval, it->first)) {
      VERRS(6) << "Ignoring unseen-in-fn-args template arg of interest: "
               << PrintableType(it->first) << "\n";
    }
  }

  retval = ResugarTypeComponents(retval);  // add in the decomposition of retval
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
  if (const NamedDecl* class_dfn = GetDefinitionForClass(decl)) {
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

bool IsForwardDecl(const clang::TagDecl* decl) {
  return (isa<RecordDecl>(decl) &&   // not an enum
          !decl->isCompleteDefinition() && !IsFriendDecl(decl) &&
          !decl->isEmbeddedInDeclarator());
}

// Two possibilities: it's written as a nested class (that is, with a
// qualifier) or it's actually living inside another class.
bool IsNestedClass(const TagDecl* decl) {
  if (decl->getQualifier() &&
      decl->getQualifier()->getKind() == NestedNameSpecifier::TypeSpec) {
    return true;
  }
  return isa<RecordDecl>(decl->getDeclContext());
}

bool HasDefaultTemplateParameters(const TemplateDecl* decl) {
  TemplateParameterList* tpl_params = decl->getTemplateParameters();
  return tpl_params->getMinRequiredArguments() < tpl_params->size();
}

template <class T> inline set<const clang::NamedDecl*> GetRedeclsOfRedeclarable(
    const clang::Redeclarable<T>* decl) {
  return set<const clang::NamedDecl*>(decl->redecls_begin(),
                                      decl->redecls_end());
}

// The only way to find out whether a decl can be dyn_cast to a
// Redeclarable<T> and what T is is to enumerate the possibilities.
// Hence we hard-code the list.
set<const clang::NamedDecl*> GetNonclassRedecls(const clang::NamedDecl* decl) {
  CHECK_(!isa<RecordDecl>(decl) && "For classes, call GetClassRedecls()");
  CHECK_(!isa<ClassTemplateDecl>(decl) && "For tpls, call GetClassRedecls()");
  if (const TagDecl* specific_decl = DynCastFrom(decl))
    return GetRedeclsOfRedeclarable(specific_decl);
  // TODO(wan): go through iwyu to replace TypedefDecl with
  // TypedefNameDecl as needed.
  if (const TypedefNameDecl* specific_decl = DynCastFrom(decl))
    return GetRedeclsOfRedeclarable(specific_decl);
  if (const FunctionDecl* specific_decl = DynCastFrom(decl))
    return GetRedeclsOfRedeclarable(specific_decl);
  if (const VarDecl* specific_decl = DynCastFrom(decl))
    return GetRedeclsOfRedeclarable(specific_decl);
  // Not redeclarable, so the output is just the input.
  set<const clang::NamedDecl*> retval;
  retval.insert(decl);
  return retval;
}

set<const NamedDecl*> GetClassRedecls(const NamedDecl* decl) {
  const RecordDecl* record_decl = DynCastFrom(decl);
  const ClassTemplateDecl* tpl_decl = DynCastFrom(decl);
  if (tpl_decl)
    record_decl = tpl_decl->getTemplatedDecl();
  if (!record_decl)
    return set<const NamedDecl*>();

  set<const NamedDecl*> redecls;
  for (TagDecl::redecl_iterator it = record_decl->redecls_begin();
       it != record_decl->redecls_end(); ++it) {
    const RecordDecl* redecl = cast<RecordDecl>(*it);
    // If this decl is a friend decl, don't count it: friend decls
    // don't serve as forward-declarations.  (This should never
    // happen, I think, but it sometimes does due to a clang bug:
    // http://llvm.org/bugs/show_bug.cgi?id=8669).  The only exception
    // is made because every decl is a redecl of itself.
    if (IsFriendDecl(redecl) && redecl != decl)
      continue;

    if (tpl_decl) {   // need to convert back to a ClassTemplateDecl
      CHECK_(isa<CXXRecordDecl>(redecl) &&
             cast<CXXRecordDecl>(redecl)->getDescribedClassTemplate());
      const CXXRecordDecl* cxx_redecl = cast<CXXRecordDecl>(redecl);
      redecls.insert(cxx_redecl->getDescribedClassTemplate());
    } else {
      redecls.insert(redecl);
    }
  }
  return redecls;
}

const NamedDecl* GetFirstRedecl(const NamedDecl* decl) {
  const NamedDecl* first_decl = decl;
  FullSourceLoc first_decl_loc(GetLocation(first_decl), *GlobalSourceManager());
  set<const NamedDecl*> all_redecls = GetClassRedecls(decl);
  if (all_redecls.empty())  // input is not a class or class template
    return NULL;

  for (Each<const NamedDecl*> it(&all_redecls); !it.AtEnd(); ++it) {
    const FullSourceLoc redecl_loc(GetLocation(*it), *GlobalSourceManager());
    if (redecl_loc.isBeforeInTranslationUnitThan(first_decl_loc)) {
      first_decl = *it;
      first_decl_loc = redecl_loc;
    }
  }
  return first_decl;
}

const NamedDecl* GetNonfriendClassRedecl(const NamedDecl* decl) {
  const RecordDecl* record_decl = DynCastFrom(decl);
  const ClassTemplateDecl* tpl_decl = DynCastFrom(decl);
  if (tpl_decl)
    record_decl = tpl_decl->getTemplatedDecl();
  // This check is so we return the input decl whenever possible.
  if (!record_decl || !IsFriendDecl(record_decl))
    return decl;

  set<const NamedDecl*> all_redecls = GetClassRedecls(decl);
  CHECK_(!all_redecls.empty() && "Uncaught non-class decl");
  return *all_redecls.begin();    // arbitrary choice
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

const Type* GetCanonicalType(const Type* type) {
  QualType canonical_type = type->getCanonicalTypeUnqualified();
  return canonical_type.getTypePtr();
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

bool IsClassType(const clang::Type* type) {
  return (type && (isa<TemplateSpecializationType>(RemoveElaboration(type)) ||
                   isa<RecordType>(RemoveElaboration(type))));
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

bool HasImplicitConversionConstructor(const Type* type) {
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

map<const clang::Type*, const clang::Type*>
GetTplTypeResugarMapForClassNoComponentTypes(const clang::Type* type) {
  map<const Type*, const Type*> retval;
  type = RemoveElaboration(type);  // get rid of the class keyword
  const TemplateSpecializationType* tpl_spec_type = DynCastFrom(type);
  if (!tpl_spec_type)
    return retval;

  // Get the list of template args that apply to the decls.
  const NamedDecl* decl = TypeToDeclAsWritten(tpl_spec_type);
  const ClassTemplateSpecializationDecl* tpl_decl = DynCastFrom(decl);
  if (!tpl_decl)   // probably because tpl_spec_type is a dependent type
    return retval;
  const TemplateArgumentList& tpl_args
      = tpl_decl->getTemplateInstantiationArgs();

  // TemplateSpecializationType only includes explicitly specified
  // types in its args list, so we start with that.  Note that an
  // explicitly specified type may fulfill multiple template args:
  //   template <typename R, typename A1> struct Foo<R(A1)> { ... }
  set<unsigned> explicit_args;   // indices into tpl_args we've filled
  TypeEnumerator type_enumerator;
  for (unsigned i = 0; i < tpl_spec_type->getNumArgs(); ++i) {
    set<const Type*> arg_components
        = type_enumerator.Enumerate(tpl_spec_type->getArg(i));
    // Go through all template types mentioned in the arg-as-written,
    // and compare it against each of the types in the template decl
    // (the latter are all desugared).  If there's a match, update
    // the mapping.
    for (Each<const Type*> it(&arg_components); !it.AtEnd(); ++it) {
      for (unsigned i = 0; i < tpl_args.size(); ++i) {
        if (const Type* arg_type = GetTemplateArgAsType(tpl_args[i])) {
          if (GetCanonicalType(*it) == arg_type) {
            retval[arg_type] = *it;
            VERRS(6) << "Adding a template-class type of interest: "
                     << PrintableType(*it) << "\n";
            explicit_args.insert(i);
          }
        }
      }
    }
  }

  // Now take a look at the args that were not filled explicitly.
  for (unsigned i = 0; i < tpl_args.size(); ++i) {
    if (ContainsKey(explicit_args, i))
      continue;
    if (const Type* arg_type = GetTemplateArgAsType(tpl_args[i])) {
      retval[arg_type] = NULL;
      VERRS(6) << "Adding a template-class default type of interest: "
               << PrintableType(arg_type) << "\n";
    }
  }

  return retval;
}

map<const clang::Type*, const clang::Type*> GetTplTypeResugarMapForClass(
    const clang::Type* type) {
  return ResugarTypeComponents(  // add in the decomposition of retval
      GetTplTypeResugarMapForClassNoComponentTypes(type));
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

const ASTTemplateArgumentListInfo* GetExplicitTplArgs(const Expr* expr) {
  if (const DeclRefExpr* decl_ref = DynCastFrom(expr))
    return decl_ref->getOptionalExplicitTemplateArgs();
  if (const MemberExpr* member_expr = DynCastFrom(expr))
    return member_expr->getOptionalExplicitTemplateArgs();
  // Ugh, annoying casts needed because no const methods exist.
  if (const OverloadExpr* overload_expr = DynCastFrom(expr))
    return const_cast<OverloadExpr*>(overload_expr)
        ->getOptionalExplicitTemplateArgs();
  if (const DependentScopeDeclRefExpr* dependent_decl_ref = DynCastFrom(expr))
    return const_cast<DependentScopeDeclRefExpr*>(dependent_decl_ref)
        ->getOptionalExplicitTemplateArgs();
  return NULL;
}
   
   
clang::CXXConstructorDecl * GetConstructor(clang::CXXNewExpr* expr) {
  const Expr *Init = expr->getInitializer(); 
  if (const CXXConstructExpr *CCE = 
      dyn_cast_or_null<CXXConstructExpr>(Init)){
    return CCE->getConstructor();
  }
  return NULL;
} 
  

}  // namespace include_what_you_use
