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

#include <map>                          // for map
#include <set>                          // for set
#include <string>                       // for string

#include "port.h"  // for CHECK_
#include "llvm/Support/Casting.h"
#include "clang/AST/DeclBase.h"
#include "clang/AST/NestedNameSpecifier.h"
#include "clang/AST/Stmt.h"
#include "clang/AST/TemplateBase.h"
#include "clang/AST/Type.h"
#include "clang/AST/TypeLoc.h"
#include "clang/Basic/SourceLocation.h"

namespace clang {
class CXXConstructExpr;
class CXXConstructorDecl;
class CXXDeleteExpr;
class CXXDestructorDecl;
class CXXRecordDecl;
class CallExpr;
class CastExpr;
class ClassTemplateDecl;
class Expr;
class FunctionDecl;
class NamedDecl;
class SourceManager;
class TagDecl;
class TemplateDecl;
class TemplateName;
class TypeDecl;
class ValueDecl;
struct ExplicitTemplateArgumentList;
}  // namespace clang

namespace include_what_you_use {

using std::map;
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
  clang::SourceLocation GetLocation() const;

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
    return ::llvm::dyn_cast<To>(as_decl_);
  }
  template<typename To> const To* DynCast(const clang::Stmt*) const {
    if (kind_ != kStmtKind) return NULL;
    return ::llvm::dyn_cast<To>(as_stmt_);
  }
  template<typename To> const To* DynCast(const clang::Type*) const {
    // We also will cast ourselves to a type if we're a typeloc.
    // This simplifies a lot of code lower down that doesn't care
    // to distinguish.  For code that *does* care to distinguish,
    // it should check for typelocs first:
    //   if (node.IsA<FooTypeLoc>()) ... else if (node.IsA<FooType>()) ...
    if (kind_ == kTypelocKind)
      return ::llvm::dyn_cast<To>(as_typeloc_->getTypePtr());
    if (kind_ != kTypeKind) return NULL;
    return ::llvm::dyn_cast<To>(as_type_);
  }
  template<typename To> const To* DynCast(const clang::TypeLoc*) const {
    if (kind_ != kTypelocKind) return NULL;
    return ::llvm::dyn_cast<To>(as_typeloc_);
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
  bool FillLocationIfKnown(clang::SourceLocation* loc) const;

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
bool IsElaborationNode(const ASTNode* ast_node);

// See if a given ast_node is a namespace-qualified ElaboratedType
// node. (E.g. 'class ns::Foo myyvar'.)
bool IsNamespaceQualifiedNode(const ASTNode* ast_node);

// Return true if the given ast_node is inside a C++ method body.  Do
// this by walking up the AST tree until you find a CXXMethodDecl,
// then see if the node just before you reached it is the body.  We
// also check if the node is in an initializer (either explicitly or
// implicitly), or the implicit (non-body) code of a destructor.
bool IsNodeInsideCXXMethodBody(const ASTNode* ast_node);

// Return true if we're a nested class as written, that is, we're a
// class decl inside another class decl.  The parent class may be
// templated, but we should not be.  (We could extend the function to
// handle that case, but there's been no need yet.)
bool IsNestedClassAsWritten(const ASTNode* ast_node);

// Is ast_node the 'D' in the following:
//    template<template <typename A> class T = D> class C { ... }
// ('D' might be something like 'vector').
bool IsDefaultTemplateTemplateArg(const ASTNode* ast_node);

// Returns true if this node is a ConstructExpr that is being used to
// construct a field in a class (that is, it's part of a constructor
// initializer list).
bool IsCXXConstructExprInInitializer(const ASTNode* ast_node);

// If ASTNode is of a kind that has a qualifier (something that
// comes before the ::), return that, else return NULL.
const clang::NestedNameSpecifier* GetQualifier(const ASTNode* ast_node);

// Returns true if any parent is a typedef: my_typedef.a, or
// MyTypedef::a, or MyTypedef::subclass::a, etc.  Note it does
// *not* return true if the ast_node itself is a typedef.
bool IsMemberOfATypedef(const ASTNode* ast_node);

// Returns the decl-context of the deepest decl in the ast-chain.
const clang::DeclContext* GetDeclContext(const ASTNode* ast_node);


//------------------------------------------------------------
// Helper functions for working with raw Clang AST nodes.

// --- Printers.

string PrintableLoc(clang::SourceLocation loc);
string PrintableSourceRange(clang::SourceRange range);
string PrintableDecl(const clang::Decl* decl);
void PrintStmt(const clang::Stmt* stmt);
string PrintableType(const clang::Type* type);
string PrintableTypeLoc(const clang::TypeLoc& typeloc);
string PrintableNestedNameSpecifier(const clang::NestedNameSpecifier* nns);
string PrintableTemplateName(const clang::TemplateName& tpl_name);
string PrintableTemplateArgument(const clang::TemplateArgument& arg);
string PrintableTemplateArgumentLoc(const clang::TemplateArgumentLoc& arg);
// This prints to errs().  It's useful for debugging (e.g. inside gdb).
void PrintASTNode(const ASTNode* node);

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
internal::DynCastPtr<T> DynCastFrom(T* ptr) {
  return internal::DynCastPtr<T>(ptr);
}

// --- Utilities for Decl.

// Returns true if the decl is for a templatized function.
bool IsTemplatizedFunctionDecl(const clang::FunctionDecl* decl);

// Returns true if the given class has at least one implicit
// conversion constructor.
bool HasImplicitConversionCtor(const clang::CXXRecordDecl* cxx_class);

// If this decl is a (possibly templatized) class, return the decl
// that defines the class, if present.  Otherwise return NULL.
const clang::NamedDecl* GetDefinitionForClass(const clang::Decl* decl);

// Given a class, returns a SourceRange that encompasses the beginning
// of the class declaration (including template<> prefix, etc) to the
// class name.  Used to determine where forward-declares are.
clang::SourceRange GetSourceRangeOfClassDecl(const clang::Decl* decl);

// One can't have partial template specialization or default template
// args for function templates, but they're complicated in their own
// way: they can have deduced template arguments (deduced from the
// function arguments).  When a templated function call does not
// specify the template arguments explicitly, but instead derives them
// from the function arguments, clang canonicalizes (desugars) the
// template args.  For
//    template<class T> void MyFunc(T t) { ... }
//    MyFunc(typedef_type)
// clang will say this is a call to MyFunc<canonical_type>().  Also:
//    MyFunc(my_int_vector)
// clang will say this is MyFunc<vector<int, alloc<int> >(), with
// no indication that alloc<int> is actually a default parameter.
// Equally bad:
//    template<class T> void OtherFunc(MyClass<T> t) { ... }
//    typedef MyClass<Foo> FooClass;
//    OtherFunc(my_foo_class);
// clang will see T as MyClass<Foo> even though my_foo_class hides the
// use of Foo through the typedef.
//    This routine attempts to solve all these problems by looking at
// the type-as-written for the actual arguments (and return value) to
// try to reverse engineer the derived-argument matching that was
// done.  (It's easy in the rare cases the template args are
// explicitly specified.)  It returns a map from the unsugared
// (canonical) types of each template argument to its sugared
// (as-written) type.  For now we ignore non-type template args.
// We also include mappings for component types: if we have an entry
// 'vector<TypedefType>' -> 'vector<Foo>', we also add an entry
// 'TypedefType' -> 'Foo'.
// NOTE: This routine is far from perfect.  To really do this right,
// we'd need to refactor SemaTemplateDeduction to take an argument to
// not canonicalize deduced template arguments.
// calling_expr should be a CallExpr, CXXConstructExpr, or DeclRefExpr.
map<const clang::Type*, const clang::Type*> GetTplTypeResugarMapForFunction(
    const clang::FunctionDecl* decl, const clang::Expr* calling_expr);

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
const clang::NamedDecl* GetInstantiatedFromDecl(
    const clang::CXXRecordDecl* class_decl);

// For an implicitly instantiated templated c++ class -- that is, a
// class like vector<int> that isn't explicitly written in the source
// code but instead derived from vector<T> -- returns the
// class-as-written (vector<T>, in this case).  For an implicitly
// instantiated template function -- Fn<int> when all that is written
// in the source code is Fn<T> -- returns the function-as-written.
// (In each case, prefers the definition of the class or function
// as-written, rather than a forward-declaration.)  Otherwise, returns
// the original input.
const clang::NamedDecl* GetDefinitionAsWritten(const clang::NamedDecl* decl);

// True if this decl is for default (not placement-new)
// new/delete/new[]/delete[] from <new>.  The second argument
// is the quoted form of the file the decl comes from, e.g. '<new>'.
bool IsDefaultNewOrDelete(const clang::FunctionDecl* decl,
                          const string& decl_loc_as_quoted_include);

// Returns true if this decl is part of a friend decl.
bool IsFriendDecl(const clang::Decl* decl);

// Returns true if a Record-decl looks like a forward-declaration of a
// class (rather than a definition, a friend declaration, or an 'in
// place' declaration like 'struct Foo' in 'void MyFunc(struct Foo*);'
// Always returns false for enums.
bool IsForwardDecl(const clang::TagDecl* decl);

// Returns true if this decl is defined inside another class/struct.
// Unlike IsNestedClassAsWritten(), which works on an ASTNode, this
// function considers decl to be nested even if it's not syntactically
// written inside its outer class (that is, 'class Foo::Bar {...}' is
// considered nested, even though it's not written inside Foo).
bool IsNestedClass(const clang::TagDecl* decl);

bool HasDefaultTemplateParameters(const clang::TemplateDecl* decl);

// For any decl that inherits from clang::Redeclarable *except* for
// classes and class templates -- enums, typedefs, functions, vars --
// returns all the declarations of decl.  For any other decl, the
// output is just the input decl.  Output decls are guaranteed to be
// of the same type as the input Decl.  Because iwyu fundamentally
// treats classes different from other redeclarable types, it has
// its own separate function.  (If that proves to be annoying, we
// can merge them.)
set<const clang::NamedDecl*> GetNonclassRedecls(const clang::NamedDecl* decl);

// Given a class, returns a set of all declarations of that class
// (forward-declarations and, if present, the definition).  This
// accepts both RecordDecls and ClassTemplateDecls -- the return Decls
// are guaranteed to be of the same type as the input Decl.  Returns
// the empty set if the input is not a RecordDecl or
// ClassTemplateDecl.  Otherwise, always returns at least one element
// (since the input decl is its own redecl).
set<const clang::NamedDecl*> GetClassRedecls(const clang::NamedDecl* decl);

// Returns the redecl of decl that occurs first in the translation
// unit (that is, is the first one you'd see if you did 'cc -E').
// Returns NULL if the input is not a RecordDecl or ClassTemplateDecl.
const clang::NamedDecl* GetFirstRedecl(const clang::NamedDecl* decl);

// Given a class or class template, returns the declaration of that
// class that specifies the values of the default template arguments.
// If there are no default template arguments, returns NULL.
const clang::ClassTemplateDecl* GetClassRedeclSpecifyingDefaultTplArgs(
    const clang::ClassTemplateDecl* decl);

// Picks one redecl from GetClassRedecls() arbitrarily.
// This is used to recover from the clang bug that mixes friend decls
// with 'real' redecls (http://llvm.org/bugs/show_bug.cgi?id=8669);
// this function returns a 'real' redecl.  If the input decl is a
// friend decl, returns an arbitrary non-friend redecl of it;
// otherwise returns decl itself.
// TODO(csilvers): remove once PR 8669 is fixed.
const clang::NamedDecl* GetNonfriendClassRedecl(const clang::NamedDecl* decl);

// Returns true if the innermost DeclContext for each decl is the
// same, and it's a class (or struct).
bool DeclsAreInSameClass(const clang::Decl* decl1, const clang::Decl* decl2);


// --- Utilities for Type.

const clang::Type* GetTypeOf(const clang::Expr* expr);
// Returns the type of the given variable, function, or enum declaration.
const clang::Type* GetTypeOf(const clang::ValueDecl* decl);
// ...or class, struct, union, enum, typedef, or template type.
const clang::Type* GetTypeOf(const clang::TypeDecl* decl);

// Template parameters are always reduced to the canonical type.
const clang::Type* GetCanonicalType(const clang::Type* type);

// The ElaborationType -- which says whether a type is preceded by
// 'class' or 'struct' ('class Foo'), or whether the type-name has a
// namespace ('ns::Foo') -- often pops where it's not wanted.  This
// removes the elaboration if it exists, else it's a noop.  Note that
// if the type has both kinds of elaborations ('struct ns::Foo'), they
// will both be removed.
const clang::Type* RemoveElaboration(const clang::Type* type);

// Returns true if the type has any template arguments.
bool IsTemplatizedType(const clang::Type* type);

// Returns true if the type is a RecordType or a TemplateSpecializationType.
bool IsClassType(const clang::Type* type);

// Read past SubstTemplateTypeParmType to the underlying type, if type
// is itself a SubstTemplateTypeParmType.  Thus: T is converted to int
// if we are parsing a template instantiated with T being int.
// However, vector<T> is *not* converted to vector<int>.
const clang::Type* RemoveSubstTemplateTypeParm(const clang::Type* type);

// Returns true if type is a pointer type (pointer or reference,
// looking through elaborations like 'class Foo*' (vs 'Foo*'),
// but *not* following typedefs (which is why we can't just use
// type->isPointerType()).
// TODO(csilvers): what about array-type?
bool IsPointerOrReferenceAsWritten(const clang::Type* type);

const clang::Type* RemoveReferenceAsWritten(const clang::Type* type);

// Gets rid of all the pointers and references to get to the 'base'
// type.  Also removes all elaborations (like 'class' keyword).  We
// can't just use the default desugar() routine, because we *don't*
// want to look through typedefs.
const clang::Type* RemovePointersAndReferencesAsWritten(
    const clang::Type* type);

// Remove one layer of pointers (or references) from type.  We go
// through typedefs and the like, but only if we have to in order to
// figure out the dereferenced type, which is why we don't just use
// desugar().  Returns NULL if not a pointer.
const clang::Type* RemovePointerFromType(const clang::Type* type);

// This follows typedefs/etc to remove pointers, if necessary.
const clang::Type* RemovePointersAndReferences(const clang::Type* type);

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
const clang::NamedDecl* TypeToDeclAsWritten(const clang::Type* type);

// Returns true if it's possible to implicitly convert a value of a
// different type to 'type' via an implicit constructor.
bool HasImplicitConversionConstructor(const clang::Type* type);

// clang desugars template arguments: follows typedefs, etc.  We
// want the unsugared type, so this function provides a map from
// the desugared type back to the original type-as-written, as
// determined from the class's template arguments.  For default
// template arguments that are not specified by the caller, we
// map the type to NULL, to indicate there's no inherent sugaring.
// We also include mappings for component types: if we have an entry
// 'vector<TypedefType>' -> 'vector<Foo>', we also add an entry
// 'TypedefType' -> 'Foo'.
// For ease of calling, this accept any type, but will return an empty
// map for any input that's not a template specialization type.
map<const clang::Type*, const clang::Type*> GetTplTypeResugarMapForClass(
    const clang::Type* type);

// Like GetTplTypeResugarMapForClass, but if a type has
// components (for instance, 'Foo*' and 'vector<Foo>' both
// have a component Foo), we don't include the components
// in the result-map.
map<const clang::Type*, const clang::Type*>
GetTplTypeResugarMapForClassNoComponentTypes(const clang::Type* type);

// --- Utilities for Stmt.

// Returns true if the given expr is '&<something>'.
bool IsAddressOf(const clang::Expr* expr);

// If this function call comes from a class method -- either a normal
// one or a static one -- returns the type of the class.  Otherwise,
// returns NULL.  Note that static class methods do *not* have a
// CXXMemberCallExpr type, which is why we take a CallExpr.
const clang::Type* TypeOfParentIfMethod(const clang::CallExpr* expr);

// Given a function call, return the first argument that's a class
// (possibly a template specialization).  Note we ignore pointers to a
// class.  This is used with 'free' overloaded operators ('ostream&
// operator<<(ostream& a, int x)' to figure out what class the
// operator 'logically' belongs to.  This is a heuristic (the operator
// may "belong" to more than one argument, for instance), but covers
// all the common cases.  Returns NULL if no class-type argument is
// found.
const clang::Expr* GetFirstClassArgument(clang::CallExpr* expr);

// Returns NULL if we're deleting an argument that has no destructor.
const clang::CXXDestructorDecl* GetDestructorForDeleteExpr(
    const clang::CXXDeleteExpr* expr);

// Returns NULL if the constructor has no corresponding destructor.
const clang::CXXDestructorDecl* GetSiblingDestructorFor(
    const clang::CXXConstructorDecl* ctor);
const clang::CXXDestructorDecl* GetSiblingDestructorFor(
    const clang::CXXConstructExpr* ctor_expr);

// Figuring out the function type is non-trivial because the callee
// may be a function pointer.  This code is based on clang's Expr.cpp.
// Should never return NULL.
const clang::FunctionType* GetCalleeFunctionType(clang::CallExpr* expr);

// Figuring out whether the to-type is a reference or not is different
// depending on whether the cast is explicit or implicit.  (You'd
// think it would work to just look at expr->getSubExpr()->getType(),
// but that seems to strip off the reference.)
bool IsCastToReferenceType(const clang::CastExpr* expr);

// Returns the list of explicit template args for all exprs that support
// such a concept (declrefexpr, memberexpr), and NULL if none is present.
const clang::ExplicitTemplateArgumentList* GetExplicitTplArgs(
    const clang::Expr* expr);

}  // namespace include_what_you_use

#endif  // DEVTOOLS_MAINTENANCE_INCLUDE_WHAT_YOU_USE_IWYU_AST_UTIL_H_
