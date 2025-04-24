//===--- expression.h - iwyu test -----------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef EXPRESSION
#define EXPRESSION

enum class ExpressionKind : int {
    kUnary = 0,
    kLiteral,
};

class Expression {
public:
    Expression(ExpressionKind kind) : fKind(kind) {}

    template <typename T> bool is() const {
        return fKind == T::kIRNodeKind;
    }

    template <typename T> const T& as() const {
        return static_cast<const T&>(*this);
    }

private:
    ExpressionKind fKind;
};

#endif
