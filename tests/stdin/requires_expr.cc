//===--- requires_expr.cc - test input file for iwyu ----------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// IWYU_ARGS: -std=c++20

template <typename>
concept Concept = true;

// Test that IWYU doesn't crash here.
bool req_expr_result = requires {
  { 1 } -> Concept;
};
