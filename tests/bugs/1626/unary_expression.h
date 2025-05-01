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
