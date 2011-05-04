//===--- iwyu_location_util.cpp - SourceLoc-related utilities for iwyu ----===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "iwyu_location_util.h"

#include "iwyu_ast_util.h"
#include "clang/AST/Decl.h"
#include "clang/AST/DeclCXX.h"
#include "clang/AST/DeclTemplate.h"
#include "clang/AST/Expr.h"
#include "clang/AST/ExprCXX.h"
#include "clang/AST/NestedNameSpecifier.h"
#include "clang/AST/Stmt.h"
#include "clang/AST/TemplateBase.h"
#include "clang/AST/TypeLoc.h"
#include "clang/Basic/SourceLocation.h"

using clang::BinaryOperator;
using clang::CXXDependentScopeMemberExpr;
using clang::CXXMethodDecl;
using clang::CXXOperatorCallExpr;
using clang::ClassTemplateSpecializationDecl;
using clang::ConditionalOperator;
using clang::FunctionDecl;
using clang::MemberExpr;
using clang::SourceLocation;
using clang::UnresolvedMemberExpr;


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
SourceLocation GetLocation(const clang::Decl* decl) {
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

// Unfortunately member_expr doesn't expose the location of the .  or
// ->.  If the base is implicit, there is no . or ->, and we just
// return the member loc.  Otherwise, we have to guess if the entire
// member-expression (all of 'b.m') is in a macro or not.  We look at
// getMemberLoc(), the start of the member ('m') , and
// getBase()->getLocEnd(), the end of the base ('b').  If they're both
// on the same line of the same file, then the . or -> must be there
// too, and return that as the location.  Otherwise, we assume that
// one or the other is in a macro, but the . or -> is not, and use the
// instantiation (not spelling) location of the macro.
static SourceLocation GetMemberExprLocation(const MemberExpr* member_expr) {
  const SourceLocation member_start = member_expr->getMemberLoc();
  const SourceLocation base_end = member_expr->getBase()->getLocEnd();

  if (member_expr->isImplicitAccess() || base_end.isInvalid())
    return member_start;
  // Weird: member_start can be 'invalid' for calls like bool(x),
  // where bool() is a class's own operator bool.  Shrug.
  if (member_start.isInvalid())
    return base_end;

  // If either the base or the member is not a macro, then we consider
  // the location of this member-expr to be outside the macro.
  if (!IsInMacro(member_start))
    return member_start;
  if (!IsInMacro(base_end))
    return base_end;

  // Now figure out if the base and member are in the same macro.  If
  // so, we say the whole member-expr is part of that macro.
  // Otherwise, we just say the member-expr is in the file where the
  // member and base macros are called.
  if (GetFileEntry(member_start) == GetFileEntry(base_end) &&
      GetLineNumber(member_start) == GetLineNumber(base_end)) {
    return member_start;
  }

  return GetInstantiationLoc(member_start);
}

SourceLocation GetLocation(const clang::Stmt* stmt) {
  if (stmt == NULL)  return SourceLocation();
  // For some expressions, we take the location to be the 'key' part
  // of the expression, not the beginning.  For instance, the
  // location of 'a << b' is the '<<', not the 'a'.  This is
  // important for code like 'MACRO << 5', where we want to make
  // sure the location we return is "here", and not inside MACRO.
  // (The price is we do worse for '#define OP <<; a OP b;'.)
  if (const CXXOperatorCallExpr* call_expr = DynCastFrom(stmt)) {
    return call_expr->getOperatorLoc();
  } else if (const MemberExpr* member_expr = DynCastFrom(stmt)) {
    return GetMemberExprLocation(member_expr);
  } else if (const UnresolvedMemberExpr* member_expr
             = DynCastFrom(stmt)) {
    if (member_expr->getOperatorLoc().isValid())
      return member_expr->getOperatorLoc();
  } else if (const CXXDependentScopeMemberExpr* member_expr
             = DynCastFrom(stmt)) {
    if (member_expr->getOperatorLoc().isValid())
      return member_expr->getOperatorLoc();
  } else if (const BinaryOperator* binary_op = DynCastFrom(stmt)) {
    return binary_op->getOperatorLoc();
  } else if (const ConditionalOperator* conditional_op =
             DynCastFrom(stmt)) {
    return conditional_op->getQuestionLoc();
  }

  return stmt->getLocStart();
}

SourceLocation GetLocation(const clang::TypeLoc* typeloc) {
  if (typeloc == NULL)  return SourceLocation();
  return typeloc->getBeginLoc();
}

SourceLocation GetLocation(const clang::NestedNameSpecifierLoc* nnsloc) {
  if (nnsloc == NULL)  return SourceLocation();
  return nnsloc->getBeginLoc();
}

SourceLocation GetLocation(const clang::TemplateArgumentLoc* argloc) {
  if (argloc == NULL)  return SourceLocation();
  return argloc->getLocation();
}

}  // namespace include_what_you_use
