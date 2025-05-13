//===--- 1626.cc - iwyu test ----------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// IWYU_ARGS: -std=c++17
// IWYU_XFAIL

#include "unary_expression.h"
#include "expression.h"

// IWYU will suggest forward-declaring UnaryExpression, which causes this file
// to no longer compile.

template <typename T>
struct ExpressionVisitor {
  virtual ~ExpressionVisitor() = default;
  virtual bool visitExpression(typename T::Expression& e);
};

template <typename T>
bool ExpressionVisitor<T>::visitExpression(typename T::Expression& e) {
  if (e.template is<UnaryExpression>()) {
    auto& b = e.template as<UnaryExpression>();
    return bool(b.expr());
  }
  return false;
}

struct ExpressionVisitorTypes {
  using Expression = const Expression;
};
template struct ExpressionVisitor<ExpressionVisitorTypes>;

/**** IWYU_SUMMARY

(tests/bugs/1626/1626.cc has correct #includes/fwd-decls)

***** IWYU_SUMMARY */
