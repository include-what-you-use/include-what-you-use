//===--- iwyu_stricter_than_cpp-d4.h - test input file for iwyu -----------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef INCLUDE_WHAT_YOU_USE_TESTS_CXX_IWYU_STRICTER_THAN_CPP_D4_H_
#define INCLUDE_WHAT_YOU_USE_TESTS_CXX_IWYU_STRICTER_THAN_CPP_D4_H_

#include "tests/cxx/iwyu_stricter_than_cpp-i5.h"

template <typename T1, typename T2>
struct TplDirectStruct7 {
  TplDirectStruct7() = default;

  TplDirectStruct7(int) {
    // Type T1 is used both in class and in constructor definition.
    (void)sizeof(T1);
  }

  static constexpr auto s = sizeof(T1);
  T2* t2;
};

#endif  // INCLUDE_WHAT_YOU_USE_TESTS_CXX_IWYU_STRICTER_THAN_CPP_D4_H_
