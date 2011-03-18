//===--- iwyu_location_util.cpp - SourceLoc-related utilities for iwyu ----===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "clang/AST/Decl.h"
#include "clang/AST/DeclCXX.h"
#include "clang/AST/DeclTemplate.h"
#include "clang/AST/Expr.h"
#include "clang/AST/ExprCXX.h"
#include "clang/Basic/SourceLocation.h"
#include "iwyu_ast_util.h"
#include "iwyu_location_util.h"

using clang::SourceLocation;
using clang::Decl;
using clang::CXXMethodDecl;
using clang::ClassTemplateSpecializationDecl;
using clang::FunctionDecl;

namespace include_what_you_use {

// This works around two bugs(?) in clang where decl->getLocation()
// can be wrong for implicit template instantiations and functions.
// (1) Consider the following code:
//     template<class T> hash { ... };                    // tpl decl
//     template<class T> hash<basic_string<T> > { ... };  // partial spec decl
//     hash<basic_string<char> > myhash;
// The decl associated with hash<basic_string<char> > is a third decl
// that is formed implicitly from the partial-spec decl.  The bug(?) is
// that clang gives the third decl the wrong location: it should have
// the location of the partial-spec decl it is instantiating, but
// instead it has the location of original tpl decl.  (clang gets
// everything else right -- PrintableDecl(third_decl) shows the right
// class body -- but the location is wrong.)  We work around that here
// by using GetInstantiatedFromDecl to map an implicit decl back to
// the appropriate decl that actually defines the class.
// (2) Consider this code:
//     struct A { ... };
//     struct A;    // a little late, but a forward-declaration
// clang will associate the implicit constructors and destructor with
// the last declaration, which is the forward-declare, rather than the
// actual definition.  Luckily, the implicit constructor's parent is
// still correct, so we just use that as the location.  Implicit
// methods don't have their own location anyway.
//    Note the two issues can both be present, if an implicit method's
// parent is an implicit instantiation.
SourceLocation GetLocation(const Decl* decl) {
  if (decl == NULL)  return SourceLocation();

  if (const CXXMethodDecl* method_decl = DynCastFrom(decl)) {
    if (method_decl->isImplicit())
      decl = method_decl->getParent();
  }
  if (const ClassTemplateSpecializationDecl* spec = DynCastFrom(decl)) {
    decl = GetDefinitionAsWritten(spec);             // templated class
  } else if (const FunctionDecl* fn_decl = DynCastFrom(decl)) {
    if (fn_decl->getTemplateInstantiationPattern())  // templated function
      decl = GetDefinitionAsWritten(fn_decl);
  }
  return decl->getLocation();
}

clang::SourceLocation GetLocation(const clang::Stmt* stmt) {
  if (stmt == NULL)  return clang::SourceLocation();
  // For some expressions, we take the location to be the 'key' part
  // of the expression, not the beginning.  For instance, the
  // location of 'a << b' is the '<<', not the 'a'.  This is
  // important for code like 'MACRO << 5', where we want to make
  // sure the location we return is "here", and not inside MACRO.
  // (The price is we do worse for '#define OP <<; a OP b;'.)
  if (const clang::CXXOperatorCallExpr* call_expr = DynCastFrom(stmt)) {
    return call_expr->getOperatorLoc();
  } else if (const clang::MemberExpr* member_expr = DynCastFrom(stmt)) {
    // ExprLoc() is the location of the . or ->.  If it's invalid, the
    // parent-part of the expression is implicit, and there's no . or ->,
    // so just fall back on the default loc that clang gives for this expr.
    if (member_expr->getExprLoc().isValid())
      return member_expr->getExprLoc();
  } else if (const clang::UnresolvedMemberExpr* member_expr
             = DynCastFrom(stmt)) {
    if (member_expr->getOperatorLoc().isValid())
      return member_expr->getOperatorLoc();
  } else if (const clang::CXXDependentScopeMemberExpr* member_expr
             = DynCastFrom(stmt)) {
    if (member_expr->getOperatorLoc().isValid())
      return member_expr->getOperatorLoc();
  } else if (const clang::BinaryOperator* binary_op = DynCastFrom(stmt)) {
    return binary_op->getOperatorLoc();
  } else if (const clang::ConditionalOperator* conditional_op =
             DynCastFrom(stmt)) {
    return conditional_op->getQuestionLoc();
  }

  return stmt->getLocStart();
}

clang::SourceLocation GetLocation(const clang::TypeLoc* typeloc) {
  if (typeloc == NULL)  return clang::SourceLocation();
  return typeloc->getBeginLoc();
}

clang::SourceLocation GetLocation(const clang::NestedNameSpecifierLoc* nnsloc) {
  if (nnsloc == NULL)  return clang::SourceLocation();
  return nnsloc->getBeginLoc();
}

clang::SourceLocation GetLocation(const clang::TemplateArgumentLoc* argloc) {
  if (argloc == NULL)  return clang::SourceLocation();
  return argloc->getLocation();
}

}  // namespace include_what_you_use
