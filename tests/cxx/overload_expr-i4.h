//===--- overload_expr-i4.h - test input file for iwyu --------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef INCLUDE_WHAT_YOU_USE_TESTS_CXX_OVERLOAD_EXPR_I4_H_
#define INCLUDE_WHAT_YOU_USE_TESTS_CXX_OVERLOAD_EXPR_I4_H_

// All the OverloadedFn2 overloads visible without ADL are in the same file.

void OverloadedFn2(int);
void OverloadedFn2(char);

// Variable template.
template <typename>
int var_tpl;

#endif  // INCLUDE_WHAT_YOU_USE_TESTS_CXX_OVERLOAD_EXPR_I4_H_
