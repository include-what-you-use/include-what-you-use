//===--- overload_expr-i4.h - test input file for iwyu --------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// All the OverloadedFn2 overloads visible without ADL are in the same file.

void OverloadedFn2(int);
void OverloadedFn2(char);
