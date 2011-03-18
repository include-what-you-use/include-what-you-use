//===--- iwyu_ast_util.h - clang-AST utilities for include-what-you-use ---===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// Utilities that make it easier to work with Clang's AST.

#ifndef DEVTOOLS_MAINTENANCE_INCLUDE_WHAT_YOU_USE_IWYU_AST_UTIL_H_
#define DEVTOOLS_MAINTENANCE_INCLUDE_WHAT_YOU_USE_IWYU_AST_UTIL_H_

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

namespace include_what_you_use {

using std::set;
using std::string;


//------------------------------------------------------------
// ASTNode and friends.

// ASTNode represents a single node of the AST tree.  An AST node may be
// a statement, declaration, type, template-name, etc.  ASTNode keeps
// track of its parent node, as we do AST traversal, allowing queries
// on the "context" of a node.
//
// We also store some state that's useful for iwyu.  For instance,
// we store whether a node is in a 'forward-declarable' context
// (such as a function parameter), meaning all types seen below
// that node are legal to fowrard-declare according to c++.
class ASTNode {
 public:
  // In each case, the caller owns the object, and must guarantee it
  // lives for at least as long as the ASTNode object does.
  ASTNode(const clang::Decl* decl, const clang::SourceManager& sm)
      : kind_(kDeclKind), as_decl_(decl),
        parent_(NULL), in_fwd_decl_context_(false), source_manager_(sm) { }
  ASTNode(const clang::Stmt* stmt, const clang::SourceManager& sm)
      : kind_(kStmtKind), as_stmt_(stmt),
        parent_(NULL), in_fwd_decl_context_(false), source_manager_(sm) { }
  ASTNode(const clang::Type* type, const clang::SourceManager& sm)
      : kind_(kTypeKind), as_type_(type),
        parent_(NULL), in_fwd_decl_context_(false), source_manager_(sm) { }
  ASTNode(const clang::TypeLoc* typeloc, const clang::SourceManager& sm)
      : kind_(kTypelocKind), as_typeloc_(typeloc),
        parent_(NULL), in_fwd_decl_context_(false), source_manager_(sm) { }
  ASTNode(const clang::NestedNameSpecifier* nns, const clang::SourceManager& sm)
      : kind_(kNNSKind), as_nns_(nns),
        parent_(NULL), in_fwd_decl_context_(false), source_manager_(sm) { }
  ASTNode(const clang::NestedNameSpecifierLoc* nnsloc,
          const clang::SourceManager& sm)
      : kind_(kNNSLocKind), as_nnsloc_(nnsloc),
        parent_(NULL), in_fwd_decl_context_(false), source_manager_(sm) { }
  ASTNode(const clang::TemplateName* template_name,
          const clang::SourceManager& sm)
      : kind_(kTemplateNameKind), as_template_name_(template_name),
        parent_(NULL), in_fwd_decl_context_(false), source_manager_(sm) { }
  ASTNode(const clang::TemplateArgument* template_arg,
          const clang::SourceManager& sm)
      : kind_(kTemplateArgumentKind), as_template_arg_(template_arg),
        parent_(NULL), in_fwd_decl_context_(false), source_manager_(sm) { }
  ASTNode(const clang::TemplateArgumentLoc* template_argloc,
          const clang::SourceManager& sm)
      : kind_(kTemplateArgumentLocKind), as_template_argloc_(template_argloc),
        parent_(NULL), in_fwd_decl_context_(false), source_manager_(sm) { }

  // A 'forward-declare' context means some parent of us can be
  // forward-declared, which means we can be too.  e.g. in
  // MyClass<Foo>* x, Foo is fwd-declarable because MyClass<Foo> is.
  bool in_forward_declare_context() const { return in_fwd_decl_context_; }
  void set_in_forward_declare_context(bool b) { in_fwd_decl_context_ = b; }

  const ASTNode* parent() const { return parent_; }
  void SetParent(const ASTNode* parent) {
    parent_ = parent;
    if (parent)  // We inherit this from parent.
      set_in_forward_declare_context(parent->in_forward_declare_context());
  }

  // The number of nodes above this node in the AST tree.
  int depth() const {
    int depth = 0;
    for (const ASTNode* node = this; node != NULL; node = node->parent_)
      depth++;
    return depth - 1;   // don't count "this"
  }

  // If this node knows its location, returns it.  If not, and it's
  // likely its location is very close (say, within a few lines) of
  // its parent, ask its parent.  Unfortunately, there's nothing which
  // tells us whether a parent's location is very close to its child.
  // We assume that they always are (empirically this is true)
  // *except* for the case the parent is in a macro: then it often
  // happens that the parent belongs at the spelling location, while
  // the child is a macro arg and hence belongs in the instantiation
  // location.  Those could be far away, even in different files.  For
  // example: '#define NEW_FUNC(cls) void Func(cls* x) {}'.  Func is
  // at the spelling loc, but its child Type 'cls' is at the
  // instantiation loc.  In that case, or if *no* ancestor of the
  // current node knows its location, returns an invalid SourceLocation.
  clang::SourceLocation GetLocation() const {
    clang::SourceLocation retval;
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
      clang::FullSourceLoc full_loc(retval, source_manager_);
      const clang::FileEntry* spelling_file =
          source_manager_.getFileEntryForID(
              source_manager_.getFileID(full_loc.getSpellingLoc()));
      const clang::FileEntry* instantiation_file =
          source_manager_.getFileEntryForID(
              source_manager_.getFileID(full_loc.getInstantiationLoc()));
      if (spelling_file != instantiation_file)
        return clang::SourceLocation();
    }

    return retval;
  }

  // Returns true if this node points to the exact same
  // decl/typeloc/etc as the one you pass in.  For Decl/Stmt/Type, the
  // pointer is canonical (every instance of type X has the same
  // clang::Type*).  But for most, the value is canonical (each type
  // has the same QualType but not QualType*).  The IsA<> checks are
  // needed to avoid false matches when target_node is NULL.
  bool ContentIs(const clang::Decl* target_node) const {
    return IsA<clang::Decl>() && GetAs<clang::Decl>() == target_node;
  }
  bool ContentIs(const clang::Stmt* target_node) const {
    return IsA<clang::Stmt>() && GetAs<clang::Stmt>() == target_node;
  }
  bool ContentIs(const clang::Type* target_node) const {
    return IsA<clang::Type>() && GetAs<clang::Type>() == target_node;
  }
  bool ContentIs(const clang::TypeLoc* target_node) const {
    if (!IsA<clang::TypeLoc>())
      return false;
    const clang::TypeLoc* type_loc = GetAs<clang::TypeLoc>();
    if (type_loc == NULL || target_node == NULL)
      return type_loc == target_node;
    return *type_loc == *target_node;
  }
  // We don't define ContentIs() for other kinds of AST nodes
  // (e.g. TemplateName) as it's non-trivial (Clang doesn't define
  // equality comparison functions for them) and we don't need that
  // yet.

  // Returns true if the current node or any ancestor of it contains
  // the exact same thing as ptr.  One use of this is to check for
  // recursion.
  template<typename T> bool StackContainsContent(const T* ptr) const {
    for (const ASTNode* node = this; node != NULL; node = node->parent_) {
      if (node->ContentIs(ptr))
        return true;
    }
    return false;
  }

  // Generation 0 == you, 1 == parent, etc.
  template<typename To> const To* GetAncestorAs(int generation) const {
    const ASTNode* target_node = this;
    for (; generation > 0; --generation) {
      if (target_node->parent_ == NULL)
        return NULL;
      target_node = target_node->parent_;
    }
    // DynCast needs a dummy argument of type To* to help its resolution.
    const To* dummy = NULL;
    return target_node->DynCast<To>(dummy);
  }

  // Convenience methods.

  template<typename To> bool AncestorIsA(int generation) const {
    return GetAncestorAs<To>(generation) != NULL;
  }

  // Returns true if this node or any of its ancestors contains a T*.
  template<typename T> bool HasAncestorOfType() const {
    for (const ASTNode* node = this; node != NULL; node = node->parent_) {
      if (node->IsA<T>())
        return true;
    }
    return false;
  }

  template<typename To> const To* GetParentAs() const {
    return GetAncestorAs<To>(1);
  }

  template<typename To> bool ParentIsA() const {
    return AncestorIsA<To>(1);
  }

  template<typename To> const To* GetAs() const {
    return GetAncestorAs<To>(0);
  }

  template<typename To> bool IsA() const {
    return AncestorIsA<To>(0);
  }

 private:
  enum NodeKind {
    kDeclKind, kStmtKind, kTypeKind, kTypelocKind, kNNSKind, kNNSLocKind,
    kTemplateNameKind, kTemplateArgumentKind, kTemplateArgumentLocKind
  };

  // Returns a casted pointer if this object actually is of the given
  // type (or a subclass of the given type), and NULL otherwise.  We
  // have to use overloading on To's kind_, in these helper
  // methods, in order to get llvm's dyn_cast to compile -- it gets
  // upset (at compile time, sadly) if from-type and to-type aren't in
  // the same type hierarchy.  So To must be specified both in the
  // template arg and in the method parameter.
  template<typename To> const To* DynCast(const clang::Decl*) const {
    if (kind_ != kDeclKind) return NULL;
    return dyn_cast<To>(as_decl_);
  }
  template<typename To> const To* DynCast(const clang::Stmt*) const {
    if (kind_ != kStmtKind) return NULL;
    return dyn_cast<To>(as_stmt_);
  }
  template<typename To> const To* DynCast(const clang::Type*) const {
    // We also will cast ourselves to a type if we're a typeloc.
    // This simplifies a lot of code lower down that doesn't care
    // to distinguish.  For code that *does* care to distinguish,
    // it should check for typelocs first:
    //   if (node.IsA<FooTypeLoc>()) ... else if (node.IsA<FooType>()) ...
    if (kind_ == kTypelocKind)
      return dyn_cast<To>(as_typeloc_->getTypePtr());
    if (kind_ != kTypeKind) return NULL;
    return dyn_cast<To>(as_type_);
  }
  template<typename To> const To* DynCast(const clang::TypeLoc*) const {
    if (kind_ != kTypelocKind) return NULL;
    return dyn_cast<To>(as_typeloc_);
  }
  template<typename To> const To* DynCast(
      const clang::NestedNameSpecifier*) const {
    // Like Type, this will cast to NNS if we're an NNSLoc.  For code
    // that cares to distinguish, it should check for nnslocs first.
    if (kind_ == kNNSLocKind)
      return as_nnsloc_->getNestedNameSpecifier();
    if (kind_ != kNNSKind) return NULL;
    return as_nns_;
  }
  template<typename To> const To* DynCast(
      const clang::NestedNameSpecifierLoc*) const {
    if (kind_ != kNNSLocKind) return NULL;
    return as_nnsloc_;
  }
  template<typename To> const To* DynCast(const clang::TemplateName*) const {
    if (kind_ != kTemplateNameKind) return NULL;
    return as_template_name_;
  }
  template<typename To> const To* DynCast(
      const clang::TemplateArgument*) const {
    // We also will cast ourselves to a templateargument if we're a
    // templateargumentloc.  This simplifies a lot of code lower down
    // that doesn't care to distinguish.  For code that *does* care to
    // distinguish, it should check for typelocs first.
    if (kind_ == kTemplateArgumentLocKind)
      return &as_template_argloc_->getArgument();
    if (kind_ != kTemplateArgumentKind) return NULL;
    return as_template_arg_;
  }
  template<typename To> const To* DynCast(
      const clang::TemplateArgumentLoc*) const {
    if (kind_ != kTemplateArgumentLocKind) return NULL;
    return as_template_argloc_;
  }
  // We also allow casting to void*
  template<typename Ignored> const void* DynCast(const void*) const {
    switch (kind_) {   // this is just to avoid aliasing violations.
      case kDeclKind:  return as_decl_;
      case kStmtKind:  return as_stmt_;
      case kTypeKind:  return as_type_;
      case kTypelocKind:  return as_typeloc_;
      case kNNSKind:  return as_nns_;
      case kNNSLocKind:  return as_nnsloc_;
      case kTemplateNameKind:  return as_template_name_;
      case kTemplateArgumentKind:  return as_template_arg_;
      case kTemplateArgumentLocKind:  return as_template_argloc_;
      default: CHECK_(false && "Unknown kind"); return NULL;
    }
  }

  // If this node is of a type that knows its location, sets loc and
  // returns true, otherwise returns false and leaves loc unchanged.
  bool FillLocationIfKnown(clang::SourceLocation* loc) const {
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

  const NodeKind kind_;
  union {
    const clang::Decl* as_decl_;
    const clang::Stmt* as_stmt_;
    const clang::Type* as_type_;
    const clang::TypeLoc* as_typeloc_;
    const clang::NestedNameSpecifier* as_nns_;
    const clang::NestedNameSpecifierLoc* as_nnsloc_;
    const clang::TemplateName* as_template_name_;
    const clang::TemplateArgument* as_template_arg_;
    const clang::TemplateArgumentLoc* as_template_argloc_;
  };
  const ASTNode* parent_;
  bool in_fwd_decl_context_;
  const clang::SourceManager& source_manager_;
};

// --- Helper classes for ASTNode.

// An object of this type modifies a given variable in the constructor
// and restores its original value in the destructor.
template<typename T> class ValueSaver {
 public:
  ValueSaver(T* p, const T& newval) : ptr_(p), oldval_(*ptr_) {
    *ptr_ = newval;
  }
  // The one-arg version just uses the current value as newval.
  explicit ValueSaver(T* p) : ptr_(p), oldval_(*ptr_) { }

  ~ValueSaver() { *ptr_ = oldval_; }

 private:
  T* const ptr_;
  const T oldval_;
};

// An object of this type updates current_ast_node_ to be the given
// node, and sets the given node's parent to be the old
// current_ast_node_.  It then undoes this work in its destructor.
// The caller owns both old_current_node and new_current_node, and
// must make sure each of them lives at least as long as this object.
class CurrentASTNodeUpdater {
 public:
  CurrentASTNodeUpdater(ASTNode** old_current_node,
                        ASTNode* new_current_node)
      : old_current_node_value_(*old_current_node),
        node_saver_(old_current_node, new_current_node) {
    new_current_node->SetParent(old_current_node_value_);
  }

 private:
  ASTNode* const old_current_node_value_;
  const ValueSaver<ASTNode*> node_saver_;
};

// --- Utilities for ASTNode.

// See if a given ast_node is a 'real' ElaboratedType(Loc).  (An
// elaboration is 'class Foo myvar' instead of just 'Foo myvar'.)
// We avoid 'fake' elaborations that are caused because clang also
// uses ElaboratedType for namespaces ('ns::Foo myvar').
inline bool IsElaborationNode(const ASTNode* ast_node) {
  if (ast_node == NULL)
    return false;
  const clang::ElaboratedType* elaborated_type =
      ast_node->GetAs<clang::ElaboratedType>();
  return elaborated_type && elaborated_type->getKeyword() != clang::ETK_None;
}

// See if a given ast_node is a namespace-qualified ElaboratedType
// node. (E.g. 'class ns::Foo myyvar'.)
inline bool IsNamespaceQualifiedNode(const ASTNode* ast_node) {
  if (ast_node == NULL)
    return false;
  const clang::ElaboratedType* elaborated_type =
      ast_node->GetAs<clang::ElaboratedType>();
  return (elaborated_type && elaborated_type->getQualifier()
          && elaborated_type->getQualifier()->getKind() ==
          clang::NestedNameSpecifier::Namespace);
}

// See if the given ast_node is the decl of a class or function that
// occurs in a friend context.  That is, for 'friend class Foo', this
// function matches the decl for 'class Foo'.  (It does not match the
// FriendDecl 'friend class Foo' -- we're looking to say whether we're
// the decl *inside* the friend decl.)
inline bool IsDeclNodeInsideFriend(const ASTNode* ast_node) {
  if (ast_node->ParentIsA<clang::FriendDecl>() ||
      ast_node->ParentIsA<clang::FriendTemplateDecl>())
    return true;

  // For 'template<class Foo> friend class X', 'class X's parent is
  // a ClassTemplateDecl.
  if (ast_node->ParentIsA<clang::ClassTemplateDecl>() &&
      (ast_node->AncestorIsA<clang::FriendDecl>(2) ||
       ast_node->AncestorIsA<clang::FriendTemplateDecl>(2)))
    return true;

  return false;
}

// Return true if the given ast_node is inside a C++ method body.  Do
// this by walking up the AST tree until you find a CXXMethodDecl,
// then see if the node just before you reached it is the body.  We
// also check if the node is in an initializer (either explicitly or
// implicitly), or the implicit (non-body) code of a destructor.
inline bool IsNodeInsideCXXMethodBody(const ASTNode* ast_node) {
  // If we're a destructor, we're definitely part of a method body;
  // destructors don't have any other parts to them.  This case is
  // triggered when we see implicit destruction of member vars.
  if (ast_node && ast_node->IsA<clang::CXXDestructorDecl>())
    return true;
  for (; ast_node != NULL; ast_node = ast_node->parent()) {
    // If we're a constructor, check if we're part of the
    // initializers, which also count as 'the body' of the method.
    if (const clang::CXXConstructorDecl* ctor =
        ast_node->GetParentAs<clang::CXXConstructorDecl>()) {
      for (clang::CXXConstructorDecl::init_const_iterator
               it = ctor->init_begin(); it != ctor->init_end(); ++it) {
        if (ast_node->ContentIs((*it)->getInit()))
          return true;
      }
      // Now fall through to see if we're the body of the constructor.
    }
    if (const clang::CXXMethodDecl* method_decl =
        ast_node->GetParentAs<clang::CXXMethodDecl>()) {
      return ast_node->ContentIs(method_decl->getBody());
    }
  }
  return false;
}

// Is ast_node the 'D' in the following:
//    template<template <typename A> class T = D> class C { ... }
// ('D' might be something like 'vector').
inline bool IsDefaultTemplateTemplateArg(const ASTNode* ast_node) {
  // D is a TemplateName, since it's a template, and its parent
  // is a TemplateArgument, since D is inside a template argument.
  // The only way a template name can be in a template argument
  // is if it's a default parameter.
  return (ast_node->IsA<clang::TemplateName>() &&
          ast_node->ParentIsA<clang::TemplateArgument>());
}

// Returns true if this node is a ConstructExpr that is being used to
// construct a field in a class (that is, it's part of a constructor
// initializer list).
inline bool IsCXXConstructExprInInitializer(const ASTNode* ast_node) {
  if (!ast_node->IsA<clang::CXXConstructExpr>())
    return false;

  CHECK_(ast_node->parent() && "Constructor should not be a top-level node!");

  // Typically, you can tell an initializer because its parent is a
  // constructor decl.  But sometimes -- I'm not exactly sure when --
  // there can be an ExprWithCleanups in the middle.
  return ((ast_node->ParentIsA<clang::CXXConstructorDecl>()) ||
          (ast_node->ParentIsA<clang::ExprWithCleanups>() &&
           ast_node->AncestorIsA<clang::CXXConstructorDecl>(2)));
}

template<typename T>
inline clang::NestedNameSpecifier* TryGetQualifier(const ASTNode* ast_node) {
  if (ast_node->IsA<T>())
    return ast_node->GetAs<T>()->getQualifier();
  return NULL;
}

// If ASTNode is of a kind that has a qualifier (something that
// comes before the ::), return that, else return NULL.
inline const clang::NestedNameSpecifier* GetQualifier(const ASTNode* ast_node) {
  const clang::NestedNameSpecifier* nns = NULL;
  if (ast_node->IsA<clang::TemplateName>()) {
    const clang::TemplateName* tn = ast_node->GetAs<clang::TemplateName>();
    if (const clang::DependentTemplateName* dtn
        = tn->getAsDependentTemplateName())
      nns = dtn->getQualifier();
    else if (const clang::QualifiedTemplateName* qtn
             = tn->getAsQualifiedTemplateName())
      nns = qtn->getQualifier();
  }
  if (!nns) nns = TryGetQualifier<clang::ElaboratedType>(ast_node);
  if (!nns) nns = TryGetQualifier<clang::DependentNameType>(ast_node);
  if (!nns)
    nns = TryGetQualifier<clang::DependentTemplateSpecializationType>(ast_node);
  if (!nns) nns = TryGetQualifier<clang::UsingDirectiveDecl>(ast_node);
  if (!nns) nns = TryGetQualifier<clang::EnumDecl>(ast_node);
  if (!nns) nns = TryGetQualifier<clang::RecordDecl>(ast_node);
  if (!nns) nns = TryGetQualifier<clang::DeclaratorDecl>(ast_node);
  if (!nns) nns = TryGetQualifier<clang::FunctionDecl>(ast_node);
  if (!nns) nns = TryGetQualifier<clang::CXXDependentScopeMemberExpr>(ast_node);
  if (!nns) nns = TryGetQualifier<clang::DeclRefExpr>(ast_node);
  if (!nns) nns = TryGetQualifier<clang::DependentScopeDeclRefExpr>(ast_node);
  if (!nns) nns = TryGetQualifier<clang::MemberExpr>(ast_node);
  return nns;
}

// Returns true if any parent is a typedef: my_typedef.a, or
// MyTypedef::a, or MyTypedef::subclass::a, etc.  Note it does
// *not* return true if the ast_node itself is a typedef.
inline bool IsMemberOfATypedef(const ASTNode* ast_node) {
  // TODO(csilvers): is this ever triggered in practice?
  if (ast_node->ParentIsA<clang::TypedefType>()) {      // my_typedef.a
    return true;
  }

  // If we're one of those objects that exposes its qualifier
  // (stuff before the ::), use that.
  const clang::NestedNameSpecifier* nns = GetQualifier(ast_node);

  // If that doesn't work, see if our parent in the tree is an nns
  // node.  We have to be a bit careful here: 1) If we're a typedef
  // ourselves, the nns-parent is just us.  We have to go a level up
  // to see our 'real' qualifier.  2) Often the parent will be an
  // elaborated type, and we get to the qualifier that way.
  if (!nns) {
    nns = ast_node->GetParentAs<clang::NestedNameSpecifier>();
    if (nns && ast_node->IsA<clang::TypedefType>()) {
      nns = nns->getPrefix();
    } else if (!nns) {
      // nns will be non-NULL when processing 'a' in MyTypedef::a::b
      // But typically, such as processing 'a' in MyTypedef::a or 'b' in
      // MyTypedef::a::b, the parent will be an ElaboratedType.
      if (const clang::ElaboratedType* elab_type =
          ast_node->GetParentAs<clang::ElaboratedType>())
        nns = elab_type->getQualifier();
    }
  }

  for (; nns; nns = nns->getPrefix()) {
    if (nns->getAsType() && isa<clang::TypedefType>(nns->getAsType()))
      return true;
  }
  return false;
}


//------------------------------------------------------------
// Helper functions for working with raw Clang AST nodes.

// --- Printers.

inline string PrintableLoc(clang::SourceLocation loc) {
  if (loc.isInvalid()) {
    return "Invalid location";
  } else {
    std::string buffer;  // llvm wants regular string, not our versa-string
    llvm::raw_string_ostream ostream(buffer);
    loc.print(ostream, *GlobalSourceManager());
    return NormalizeFilePath(ostream.str());
  }
}

inline string PrintableSourceRange(clang::SourceRange range) {
  return PrintableLoc(range.getBegin()) + " - " + PrintableLoc(range.getEnd());
}

inline string PrintableDecl(const clang::Decl* decl) {
  std::string buffer;    // llvm wants regular string, not our versa-string
  llvm::raw_string_ostream ostream(buffer);
  decl->print(ostream);  // Note: can also set indentation and printingpolicy
  return ostream.str();
}

inline void PrintStmt(const clang::Stmt* stmt) {
  stmt->dump(*GlobalSourceManager());  // This prints to errs().
}

inline string PrintableType(const clang::Type* type) {
  return clang::QualType(type, 0).getAsString();
}

inline string PrintableTypeLoc(const clang::TypeLoc& typeloc) {
  return PrintableType(typeloc.getTypePtr());
}

inline string PrintableNestedNameSpecifier(
    const clang::NestedNameSpecifier* nns) {
  std::string buffer;  // llvm wants regular string, not our versa-string
  llvm::raw_string_ostream ostream(buffer);
  nns->print(ostream, DefaultPrintPolicy());
  return ostream.str();
}

inline string PrintableTemplateName(const clang::TemplateName& tpl_name) {
  std::string buffer;    // llvm wants regular string, not our versa-string
  llvm::raw_string_ostream ostream(buffer);
  tpl_name.print(ostream, DefaultPrintPolicy());
  return ostream.str();
}

inline string PrintableTemplateArgument(const clang::TemplateArgument& arg) {
  return clang::TemplateSpecializationType::PrintTemplateArgumentList(
      &arg, 1, DefaultPrintPolicy());
}

inline string PrintableTemplateArgumentLoc(
    const clang::TemplateArgumentLoc& arg) {
  return clang::TemplateSpecializationType::PrintTemplateArgumentList(
      &arg, 1, DefaultPrintPolicy());
}

// This prints to errs().  It's useful for debugging (e.g. inside gdb).
inline void PrintASTNode(const ASTNode* node) {
  if (const clang::Decl* decl = node->GetAs<clang::Decl>()) {
    llvm::errs() << "[" << decl->getDeclKindName() << "Decl] "
                 << PrintableDecl(decl) << "\n";
  } else if (const clang::Stmt* stmt = node->GetAs<clang::Stmt>()) {
    llvm::errs() << "[" << stmt->getStmtClassName() << "] ";
    PrintStmt(stmt);
    llvm::errs() << "\n";
  } else if (const clang::Type* type = node->GetAs<clang::Type>()) { // +typeloc
    llvm::errs() << "[" << type->getTypeClassName()
                 << (node->IsA<clang::TypeLoc>() ? "TypeLoc" : "Type") << "] "
                 << PrintableType(type) << "\n";
  } else if (const clang::NestedNameSpecifier* nns
             = node->GetAs<clang::NestedNameSpecifier>()) {
    llvm::errs() << "[NestedNameSpecifier] "
                 << PrintableNestedNameSpecifier(nns) << "\n";
  } else if (const clang::TemplateName* tpl_name
             = node->GetAs<clang::TemplateName>()) {
    llvm::errs() << "[TemplateName] "
                 << PrintableTemplateName(*tpl_name) << "\n";
  } else if (const clang::TemplateArgumentLoc* tpl_argloc
             = node->GetAs<clang::TemplateArgumentLoc>()) {
    llvm::errs() << "[TemplateArgumentLoc] "
                 << PrintableTemplateArgumentLoc(*tpl_argloc) << "\n";
  } else if (const clang::TemplateArgument* tpl_arg
             = node->GetAs<clang::TemplateArgument>()) {
    llvm::errs() << "[TemplateArgument] "
                 << PrintableTemplateArgument(*tpl_arg) << "\n";
  } else {
    CHECK_(!"Unknown kind for ASTNode");
  }
}


// --- Type conversion utilities.

namespace internal {

// For implementing DynCastFrom() -- don't use directly.
template <typename T>
class DynCastPtr {
 public:
  explicit DynCastPtr(T* ptr) : ptr_(ptr) { }

  template <typename U> operator U*() const {
    return ::llvm::dyn_cast_or_null<U>(ptr_);
  }

 private:
  T* ptr_;
};

}  // namespace internal

// dyn_cast<> and dyn_cast_or_null<> require the user to write the
// type of the target pointer, which is redundant when the result is
// immediately assigned to a newly declared pointer variable of the
// target type (the typical case).  DynCastFrom() lets us omit the
// target type, e.g.
//
//   if (const CXXConstructExpr* expr = DynCastFrom(source_expr)) ...
//
// instead of
//
//   if (const CXXConstructExpr* expr =
//       dyn_cast_or_null<CXXConstructExpr>(source_expr)) ...
//
// For readability, DynCastFrom() should only be used as the
// initializer of a variable declaration, where the target type is
// obvious.
template <typename T>
inline internal::DynCastPtr<T> DynCastFrom(T* ptr) {
  return internal::DynCastPtr<T>(ptr);
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
inline void AddTypelikeTemplateArgTo(const clang::TemplateArgument& tpl_arg,
                                     set<const clang::Type*>* argset) {
  if (tpl_arg.getKind() == clang::TemplateArgument::Type) {
    // Holds all types seen in tpl_arg (may be more than one if tpl_arg
    // is a function prototype, with argument-types and a return-type).
    set<const clang::Type*> argtypes;
    argtypes.insert(tpl_arg.getAsType().getTypePtr());
    // If the type is a function (a rare case, but happens in code like
    // TplClass<char(int, int, int)>), then the parameters are types
    // we have to consider as well.
    // TODO(csilvers): also check a fn pointer and a fn taking a fn ptr:
    // TplClass<char(*)(int, int, int)>, TplClass<char(char(*)(int, int))>
    if (const clang::FunctionProtoType* fn_type
        = DynCastFrom(tpl_arg.getAsType().getTypePtr())) {
      argtypes.insert(fn_type->getResultType().getTypePtr());
      for (unsigned i = 0; i < fn_type->getNumArgs(); ++i) {
        argtypes.insert(fn_type->getArgType(i).getTypePtr());
      }
      // I *think* it's correct to ignore the exception specs here.
    }

    for (Each<const clang::Type*> it(&argtypes); !it.AtEnd(); ++it) {
      VERRS(6) << "Adding a template type of interest: "
               << PrintableType(*it) << "\n";
      argset->insert(*it);
      // Recurse if we ourself are a template type.  Read through elaborations.
      const clang::Type* subtype = *it;
      // TODO(csilvers): use RemoveElaboration() instead.
      if (const clang::ElaboratedType* elab_type = DynCastFrom(subtype))
        subtype = elab_type->getNamedType().getTypePtr();
      if (const clang::TemplateSpecializationType* tpl_type
          = DynCastFrom(subtype)) {
        for (unsigned i = 0; i < tpl_type->getNumArgs(); ++i)
          AddTypelikeTemplateArgTo(tpl_type->getArg(i), argset);
      }
    }
  } else if (tpl_arg.getKind() == clang::TemplateArgument::Template) {
    const clang::TemplateName& tpl_name = tpl_arg.getAsTemplate();
    VERRS(6) << "Noticing (but ignoring) a template template of interest: "
             << PrintableTemplateName(tpl_name) << "\n";
    // TODO(csilvers): add tpl_name to argset somehow.  This is a
    // lower-priority TODO, since for the moment we just always
    // assume template template args needs to be fully instantiated.
  }
}

// --- Utilities for Decl.

// Returns true if the decl is for a templatized function.
inline bool IsTemplatizedFunctionDecl(const clang::FunctionDecl* decl) {
  return decl && decl->getTemplateSpecializationArgs() != NULL;
}

// Returns true if the given class has at least one implicit
// conversion constructor.
inline bool HasImplicitConversionCtor(const clang::CXXRecordDecl* cxx_class) {
  for (clang::CXXRecordDecl::ctor_iterator ctor = cxx_class->ctor_begin();
       ctor != cxx_class->ctor_end(); ++ctor) {
    if ((*ctor)->isExplicit() || (*ctor)->getNumParams() != 1 ||
        (*ctor)->isCopyConstructor())
      continue;
    return true;
  }
  return false;
}

// If this decl is a (possibly templatized) class, return the decl
// that defines the class, if present.  Otherwise return NULL.
inline const clang::RecordDecl* GetDefinitionForClass(const clang::Decl* decl) {
  const clang::RecordDecl* as_record = DynCastFrom(decl);
  const clang::ClassTemplateDecl* as_tpl = DynCastFrom(decl);
  if (as_tpl)   // Convert the template to its underlying class defn.
    as_record = dyn_cast_or_null<clang::RecordDecl>(as_tpl->getTemplatedDecl());
  if (as_record) {
    if (const clang::RecordDecl* record_dfn = as_record->getDefinition())
      return record_dfn;
  }
  return NULL;
}

// Given a class, returns a SourceRange that encompasses the beginning
// of the class declaration (including template<> prefix, etc) to the
// class name.  Used to determine where forward-declares are.
inline clang::SourceRange GetSourceRangeOfClassDecl(const clang::Decl* decl) {
  // If we're a templatized class, go 'up' a level to get the
  // template<...> prefix as well.
  if (const clang::CXXRecordDecl* cxx_decl = DynCastFrom(decl)) {
    if (cxx_decl->getDescribedClassTemplate())
      return cxx_decl->getDescribedClassTemplate()->getSourceRange();
  }
  // We can get source ranges of classes and template classes.
  if (const clang::TagDecl* tag_decl = DynCastFrom(decl))
    return tag_decl->getSourceRange();
  if (const clang::TemplateDecl* tpl_decl = DynCastFrom(decl))
    return tpl_decl->getSourceRange();
  CHECK_(!"Cannot get source range for this decl type");
  return clang::SourceRange();
}

// Luckily, one can't have partial template specialization or
// default template args for function templates.
inline set<const clang::Type*> GetTplTypeArgsOfFunction(
    const clang::FunctionDecl* decl) {
  set<const clang::Type*> retval;
  if (!decl)
    return retval;
  const clang::TemplateArgumentList* tpl_list =
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

// If class_decl is instantiated from a class template,
// returns the decl for that template; otherwise returns class_decl.
// As an example, consider this code:
//    template<class T> class Foo { ... };   // template decl
//    template<> class Foo<int> { ... };     // explicit specialization
//    template class Foo<char>;              // note: no body specified
//    Foo<int> v1;
//    Foo<float> v2;
//    Foo<char> v3;
// The types Foo<int>, Foo<float>, and Foo<char> all have a
// corresponding decl.  In the case of Foo<int>, the decl is the
// explicit specialization.  This is *not* a decl that this routine
// deals with; the routine will just return its argument in this case.
// But in the case of Foo<float>, the decl is an implicit
// instantiation of Foo<T>, and this routine will return the decl for
// Foo<T>.  Foo<char> is a rarer corner case: an explicit
// instantiation definition (it just causes code for that template
// case to be generated).  It is treated the same as Foo<float>: we
// return the template decl, which provides the actual class body.
// We try to return a decl that's also a definition, when possible.
inline const clang::NamedDecl* GetInstantiatedFromDecl(
    const clang::CXXRecordDecl* class_decl) {
  if (const clang::ClassTemplateSpecializationDecl* tpl_sp_decl =
      DynCastFrom(class_decl)) {  // an instantiated class template
    llvm::PointerUnion<clang::ClassTemplateDecl*,
                       clang::ClassTemplatePartialSpecializationDecl*>
        instantiated_from = tpl_sp_decl->getInstantiatedFrom();
    if (const clang::ClassTemplateDecl* tpl_decl =
        instantiated_from.dyn_cast<clang::ClassTemplateDecl*>()) {
      // class_decl is instantiated from a non-specialized template.
      return tpl_decl;
    } else if (const clang::ClassTemplatePartialSpecializationDecl*
               partial_spec_decl =
               instantiated_from.dyn_cast<
                   clang::ClassTemplatePartialSpecializationDecl*>()) {
      // class_decl is instantiated from a template partial specialization.
      return partial_spec_decl;
    }
  }
  // class_decl is not instantiated from a template.
  return class_decl;
}

// For an implicitly instantiated templated c++ class -- that is, a
// class like vector<int> that isn't explicitly written in the source
// code but instead derived from vector<T> -- returns the
// class-as-written (vector<T>, in this case).  For an implicitly
// instantiated template function -- Fn<int> when all that is written
// in the source code is Fn<T> -- returns the function-as-written.
// (In each case, prefers the definition of the class or function
// as-written, rather than a forward-declaration.)  Otherwise, returns
// the original input.
inline const clang::NamedDecl* GetDefinitionAsWritten(
    const clang::NamedDecl* decl) {
  // First, get to decl-as-written.
  if (const clang::CXXRecordDecl* class_decl = DynCastFrom(decl)) {
    decl = GetInstantiatedFromDecl(class_decl);
    if (const clang::ClassTemplateDecl* tpl_decl = DynCastFrom(decl))
      decl = tpl_decl->getTemplatedDecl();  // convert back to CXXRecordDecl
  } else if (const clang::FunctionDecl* func_decl = DynCastFrom(decl)) {
    if (const clang::FunctionDecl* tpl_pattern =
        func_decl->getTemplateInstantiationPattern())
      decl = tpl_pattern;
  }
  // Then, get to definition.
  if (const clang::RecordDecl* class_dfn = GetDefinitionForClass(decl)) {
    // If we started this fn as a template, convert back to a template now.
    if (const clang::CXXRecordDecl* cxx_class_dfn = DynCastFrom(class_dfn)) {
      if (cxx_class_dfn->getDescribedClassTemplate())
        return cxx_class_dfn->getDescribedClassTemplate();
    }
    return class_dfn;
  } else if (const clang::FunctionDecl* fn_decl = DynCastFrom(decl)) {
    for (clang::FunctionDecl::redecl_iterator it = fn_decl->redecls_begin();
         it != fn_decl->redecls_end(); ++it) {
      if ((*it)->isThisDeclarationADefinition())
        return *it;
    }
  }
  // Couldn't find a definition, just return the original declaration.
  return decl;
}

// True if this decl is for default (not placement-new)
// new/delete/new[]/delete[] from <new>.  The second argument
// is the quoted form of the file the decl comes from, e.g. '<new>'.
inline bool IsDefaultNewOrDelete(const clang::FunctionDecl* decl,
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

// Returns true if this decl is part of a friend decl.
inline bool IsFriendDecl(const clang::Decl* decl) {
  // For 'template<...> friend class T', the decl will just be 'class T'.
  // We need to go 'up' a level to check friendship in the right place.
  if (const clang::CXXRecordDecl* cxx_decl = DynCastFrom(decl))
    if (cxx_decl->getDescribedClassTemplate())
      decl = cxx_decl->getDescribedClassTemplate();
  return decl->getFriendObjectKind() != clang::Decl::FOK_None;
}

inline bool HasDefaultTemplateParameters(const clang::TemplateDecl* decl) {
  clang::TemplateParameterList* tpl_params = decl->getTemplateParameters();
  return tpl_params->getMinRequiredArguments() < tpl_params->size();
}

inline set<const clang::RecordDecl*> GetClassRedecls(
    const clang::RecordDecl* decl) {
  set<const clang::RecordDecl*> redecls;
  for (clang::TagDecl::redecl_iterator it = decl->redecls_begin();
       it != decl->redecls_end(); ++it) {
    const clang::RecordDecl* redecl = cast<clang::RecordDecl>(*it);
    // If this decl is a friend decl, don't count it: friend decls
    // don't serve as forward-declarations.  (This should never
    // happen, I think, but it sometimes does due to a clang bug:
    // http://llvm.org/bugs/show_bug.cgi?id=8669)
    if (!IsFriendDecl(redecl))
      redecls.insert(redecl);
  }
  return redecls;
}

// Unlike GetClassRedecls, this accepts both RecordDecls and
// ClassTemplateDecls -- the return Decl is guaranteed to be of the
// same type as the input Decl.  It picks one redecl arbitrarily.
// This is used to recover from the clang bug that mixes friend decls
// with 'real' redecls (http://llvm.org/bugs/show_bug.cgi?id=8669);
// this function returns a 'real' redecl.  If the input decl is not a
// friend decl, or not a class decl at all, it will always be returned.
// TODO(csilvers): remove once PR 8669 is fixed.
inline const clang::NamedDecl* GetNonfriendClassRedecl(
    const clang::NamedDecl* decl) {
  const clang::RecordDecl* record_decl = DynCastFrom(decl);
  const clang::ClassTemplateDecl* tpl_decl = DynCastFrom(decl);
  if (tpl_decl)
    record_decl = tpl_decl->getTemplatedDecl();
  if (!record_decl || !IsFriendDecl(record_decl))
    return decl;

  const set<const clang::RecordDecl*> redecls = GetClassRedecls(record_decl);
  CHECK_(!redecls.empty() && "Should be at least once 'real' decl");
  const clang::RecordDecl* retval = *redecls.begin();  // arbitrary choice

  if (tpl_decl) {   // need to convert back to a ClassTemplateDecl
    CHECK_(isa<clang::CXXRecordDecl>(retval) &&
           cast<clang::CXXRecordDecl>(retval)->getDescribedClassTemplate());
    const clang::CXXRecordDecl* cxx_decl = cast<clang::CXXRecordDecl>(retval);
    return cxx_decl->getDescribedClassTemplate();
  }
  return retval;
}

// Returns true if the innermost DeclContext for each decl is the
// same, and it's a class (or struct).
inline bool DeclsAreInSameClass(const clang::Decl* decl1,
                                const clang::Decl* decl2) {
  if (!decl1 || !decl2)
    return false;
  if (decl1->getDeclContext() != decl2->getDeclContext())
    return false;
  return decl1->getDeclContext()->isRecord();
}


// --- Utilities for Type.

inline const clang::Type* GetTypeOf(const clang::Expr* expr) {
  return expr->getType().getTypePtr();
}

// Returns the type of the given variable, function, or enum declaration.
inline const clang::Type* GetTypeOf(const clang::ValueDecl* decl) {
  return decl->getType().getTypePtr();
}
// ...or class, struct, union, enum, typedef, or template type.
inline const clang::Type* GetTypeOf(const clang::TypeDecl* decl) {
  return decl->getTypeForDecl();
}


// The ElaborationType -- which says whether a type is preceded by
// 'class' or 'struct' ('class Foo'), or whether the type-name has a
// namespace ('ns::Foo') -- often pops where it's not wanted.  This
// removes the elaboration if it exists, else it's a noop.  Note that
// if the type has both kinds of elaborations ('struct ns::Foo'), they
// will both be removed.
inline const clang::Type* RemoveElaboration(const clang::Type* type) {
  if (const clang::ElaboratedType* elaborated_type = DynCastFrom(type))
    return elaborated_type->getNamedType().getTypePtr();
  else
    return type;
}

// Returns true if the type has any template arguments.
inline bool IsTemplatizedType(const clang::Type* type) {
  return (type &&
          isa<clang::TemplateSpecializationType>(RemoveElaboration(type)));
}

// Read past SubstTemplateTypeParmType to the underlying type, if type
// is itself a SubstTemplateTypeParmType.  Thus: T is converted to int
// if we are parsing a template instantiated with T being int.
// However, vector<T> is *not* converted to vector<int>.
inline const clang::Type* RemoveSubstTemplateTypeParm(const clang::Type* type) {
  if (const clang::SubstTemplateTypeParmType* subst_type = DynCastFrom(type))
    return subst_type->getReplacementType().getTypePtr();
  else
    return type;
}

// Returns true if type is a pointer type (pointer or reference,
// looking through elaborations like 'class Foo*' (vs 'Foo*'),
// but *not* following typedefs (which is why we can't just use
// type->isPointerType()).
// TODO(csilvers): what about array-type?
inline bool IsPointerOrReferenceAsWritten(const clang::Type* type) {
  type = RemoveElaboration(type);
  return isa<clang::PointerType>(type) || isa<clang::LValueReferenceType>(type);
}

// Gets rid of all the pointers and references to get to the 'base'
// type.  Also removes all elaborations (like 'class' keyword).  We
// can't just use the default desugar() routine, because we *don't*
// want to look through typedefs.
inline const clang::Type* RemovePointersAndReferencesAsWritten(
    const clang::Type* type) {
  type = RemoveElaboration(type);
  while (isa<clang::PointerType>(type) ||
         isa<clang::LValueReferenceType>(type)) {
    type = type->getPointeeType().getTypePtr();
  }
  return type;
}

// Remove one layer of pointers (or references) from type.  We go
// through typedefs and the like, but only if we have to in order to
// figure out the dereferenced type, which is why we don't just use
// desugar().  Returns NULL if not a pointer.
inline const clang::Type* RemovePointerFromType(const clang::Type* type) {
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
inline const clang::Type* RemovePointersAndReferences(const clang::Type* type) {
  while (1) {
    const clang::Type* deref_type = RemovePointerFromType(type);
    if (deref_type == NULL)   // type wasn't a pointer (or reference) type
      break;                  // removed all pointers
    type = deref_type;
  }
  return type;
}

// To the best of our ability, maps a type to a declaration of that
// type *that is written in the source code*.  For most types, of
// course, declarations are always written, but for implicit template
// specializations they typically aren't.  For instance, if the type
// is vector<int>, there is a decl for 'class vector<int>', but it's
// not written anywhere; what is written is generic code, for
// 'class vector<T>'.  'class vector<int>' is an implicit declaration,
// constructed at need.  This routine does not return that implicit
// declaration.
//    If the type is a substituted template parameter, we get a
// decl for the substituted type.  That is, for this code:
//   template<class T> void MyFunc() { T foo; }
//   MyFunc<MyClass>();
// If we're evaluating MyFunc<MyClass>() and see the type that's in
// the function body, this function returns a decl for MyClass.
//    If the type is built-in, or otherwise doesn't have a decl,
// this function returns NULL.
inline const clang::NamedDecl* TypeToDeclAsWritten(const clang::Type* type) {
  // Get past all the 'class' and 'struct' prefixes, and namespaces.
  type = RemoveElaboration(type);

  // Read past SubstTemplateTypeParmType (this can happen if a
  // template function returns the tpl-arg type: e.g. for
  // 'T MyFn<T>() {...}; MyFn<X>.a', the type of MyFn<X> will be a Subst.
  type = RemoveSubstTemplateTypeParm(type);

  CHECK_(!isa<clang::ObjCObjectType>(type) &&
         "IWYU doesn't support Objective-C");

  // We have to be a bit careful about the order, because we want
  // to keep typedefs as typedefs, so we do the record check last.
  // We use getAs<> when we can -- unfortunately, it only exists
  // for a few types so far.
  if (const clang::TypedefType* typedef_type = DynCastFrom(type)) {
    return typedef_type->getDecl();
  } else if (const clang::InjectedClassNameType* icn_type
             = type->getAs<clang::InjectedClassNameType>()) {
    return icn_type->getDecl();
  } else if (const clang::RecordType* record_type
             = type->getAs<clang::RecordType>()) {
    return record_type->getDecl();
  } else if (const clang::TagType* tag_type = DynCastFrom(type)) {
    return tag_type->getDecl();    // probably just enums
  } else if (const clang::TemplateSpecializationType* template_spec_type
             = DynCastFrom(type)) {
    // A non-concrete template class, such as 'Myclass<T>'
    return template_spec_type->getTemplateName().getAsTemplateDecl();
  } else if (const clang::FunctionType* function_type = DynCastFrom(type)) {
    // TODO(csilvers): is it possible to map from fn type to fn decl?
    (void)function_type;
    return NULL;
  }  else {
    return NULL;
  }
}

inline const clang::Type* RemoveReferenceAsWritten(const clang::Type* type) {
  if (const clang::LValueReferenceType* ref_type = DynCastFrom(type))
    return ref_type->getPointeeType().getTypePtr();
  else
    return type;
}

// Returns true if it's possible to implicitly convert a value of a
// different type to 'type' via an implicit constructor.
inline bool CanImplicitlyConvertTo(const clang::Type* type) {
  type = RemoveElaboration(type);  // get rid of the class keyword
  if (isa<clang::PointerType>(type))
    return false;  // can't implicitly convert to a pointer
  if (isa<clang::LValueReferenceType>(type) &&
      !type->getPointeeType().isConstQualified())
    return false;  // can't implicitly convert to a non-const reference

  type = RemoveReferenceAsWritten(type);
  const clang::NamedDecl* decl = TypeToDeclAsWritten(type);
  if (!decl)  // not the kind of type that has a decl (e.g. built-in)
    return false;

  const clang::CXXRecordDecl* cxx_class = DynCastFrom(decl);
  if (!cxx_class)
    return false;  // can't implicitly convert to a non-class type

  return HasImplicitConversionCtor(cxx_class);
}

// For ease of calling, this accept any type, but will return an empty
// set for any input that's not a template specialization type.
inline set<const clang::Type*> GetExplicitTplTypeArgsOf(
    const clang::Type* type) {
  set<const clang::Type*> retval;
  type = RemoveElaboration(type);  // get rid of the class keyword
  const clang::TemplateSpecializationType* tpl_spec_type = DynCastFrom(type);
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

// Returns true if the given expr is '&<something>'.
inline bool IsAddressOf(const clang::Expr* expr) {
  if (const clang::UnaryOperator* unary = DynCastFrom(expr->IgnoreParens()))
    return unary->getOpcode() == clang::UO_AddrOf;
  return false;
}

// If this function call comes from a class method -- either a normal
// one or a static one -- returns the type of the class.  Otherwise,
// returns NULL.  Note that static class methods do *not* have a
// CXXMemberCallExpr type, which is why we take a CallExpr.
inline const clang::Type* TypeOfParentIfMethod(const clang::CallExpr* expr) {
  // callee_expr is a MemberExpr if we're a normal class method, or
  // DeclRefExpr if we're a static class method or an overloaded operator.
  const clang::Expr* callee_expr = expr->getCallee()->IgnoreParenCasts();
  if (const clang::MemberExpr* member_expr = DynCastFrom(callee_expr)) {
    const clang::Type* class_type = GetTypeOf(member_expr->getBase());
    // For class->member(), class_type is a pointer.
    return RemovePointersAndReferencesAsWritten(class_type);
  } else if (const clang::DeclRefExpr* ref_expr = DynCastFrom(callee_expr)) {
    if (ref_expr->getQualifier()) {    // static methods like C<T>::a()
      return ref_expr->getQualifier()->getAsType();
    }
  }
  return NULL;
}

// Given a function call, return the first argument that's a class
// (possibly a template specialization).  Note we ignore pointers to a
// class.  This is used with 'free' overloaded operators ('ostream&
// operator<<(ostream& a, int x)' to figure out what class the
// operator 'logically' belongs to.  This is a heuristic (the operator
// may "belong" to more than one argument, for instance), but covers
// all the common cases.  Returns NULL if no class-type argument is
// found.
inline const clang::Expr* GetFirstClassArgument(clang::CallExpr* expr) {
  for (clang::CallExpr::arg_iterator it = expr->arg_begin();
       it != expr->arg_end(); ++it) {
    const clang::Type* argtype = GetTypeOf(*it);
    // Make sure we do the right thing given a function like
    //    template <typename T> void operator>>(const T& x, ostream& os);
    // In this case ('myclass >> os'), we want to be returning the
    // type of os, not of myclass, and we do, because myclass will be
    // a SubstTemplateTypeParmType, not a RecordType.
    if (isa<clang::SubstTemplateTypeParmType>(argtype))
      continue;
    // TODO(csilvers): uncomment this and fix up tests to match.
    //argtype = argtype->getUnqualifiedDesugaredType();  // see through typedefs
    if (isa<clang::RecordType>(argtype) ||
        isa<clang::TemplateSpecializationType>(argtype)) {
      return *it;
    }
  }
  return NULL;
}

// Returns NULL if we're deleting an argument that has no destructor.
inline const clang::CXXDestructorDecl* GetDestructorForDeleteExpr(
    const clang::CXXDeleteExpr* expr) {
  const clang::Type* type = expr->getDestroyedType().getTypePtrOrNull();
  // type is NULL when deleting a dependent type: 'T foo; delete foo'
  if (type == NULL)
    return NULL;
  const clang::NamedDecl* decl = TypeToDeclAsWritten(type);
  if (const clang::CXXRecordDecl* cxx_record = DynCastFrom(decl))
    return cxx_record->getDestructor();
  return NULL;
}

// Returns NULL if the constructor has no corresponding destructor.
inline const clang::CXXDestructorDecl* GetSiblingDestructorFor(
    const clang::CXXConstructorDecl* ctor) {
  return ctor ? ctor->getParent()->getDestructor() : NULL;
}
inline const clang::CXXDestructorDecl* GetSiblingDestructorFor(
    const clang::CXXConstructExpr* ctor_expr) {
  return GetSiblingDestructorFor(ctor_expr->getConstructor());
}

// Figuring out the function type is non-trivial because the callee
// may be a function pointer.  This code is based on clang's Expr.cpp.
// Should never return NULL.
inline const clang::FunctionType* GetCalleeFunctionType(clang::CallExpr* expr) {
  const clang::Type* callee_type = expr->getCallee()->getType().getTypePtr();
  if (const clang::PointerType* ptr_type
      = callee_type->getAs<clang::PointerType>()) {
    callee_type = ptr_type->getPointeeType().getTypePtr();
  } else if (const clang::BlockPointerType* bptr_type
             = callee_type->getAs<clang::BlockPointerType>()) {
    callee_type = bptr_type->getPointeeType().getTypePtr();
  } else if (const clang::MemberPointerType* mptr_type
             = callee_type->getAs<clang::MemberPointerType>()) {
    callee_type = mptr_type->getPointeeType().getTypePtr();
  }
  return callee_type->getAs<clang::FunctionType>();
}

// Figuring out whether the to-type is a reference or not is different
// depending on whether the cast is explicit or implicit.  (You'd
// think it would work to just look at expr->getSubExpr()->getType(),
// but that seems to strip off the reference.)
inline bool IsCastToReferenceType(const clang::CastExpr* expr) {
  if (const clang::ExplicitCastExpr* explicit_cast = DynCastFrom(expr)) {
    return explicit_cast->getTypeAsWritten()->isReferenceType();
  } else if (const clang::ImplicitCastExpr* implicit_cast = DynCastFrom(expr)) {
    return implicit_cast->getValueKind() == clang::VK_LValue;
  } else {
    CHECK_(false && "Unexpected type of cast expression");
    return false;
  }
}

}  // namespace include_what_you_use
#endif  // DEVTOOLS_MAINTENANCE_INCLUDE_WHAT_YOU_USE_IWYU_AST_UTIL_H_
