//===--- inheriting_ctor-i1.h - test input file for iwyu ------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef INCLUDE_WHAT_YOU_USE_TESTS_CXX_INHERITING_CTOR_I1_H_
#define INCLUDE_WHAT_YOU_USE_TESTS_CXX_INHERITING_CTOR_I1_H_

struct Base {
  Base(int);
};
struct Derived : Base {
  using Base::Base;
};

#endif  // INCLUDE_WHAT_YOU_USE_TESTS_CXX_INHERITING_CTOR_I1_H_
