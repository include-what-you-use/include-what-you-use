//===--- unary_expression.h - iwyu test -----------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef UNARY_EXPRESSION
#define UNARY_EXPRESSION

#include "expression.h"

class UnaryExpression final : public Expression {
public:
    inline static constexpr ExpressionKind kIRNodeKind = ExpressionKind::kUnary;
    UnaryExpression(Expression* expr) : Expression(kIRNodeKind), fExpr(expr) { }
    Expression* expr() const { return fExpr; }
private:
    Expression* fExpr;
};

#endif
