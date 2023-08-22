//===--- iwyu_stricter_than_cpp-d4.h - test input file for iwyu -----------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef IWYU_STRICTER_THAN_CPP_D4_H_
#define IWYU_STRICTER_THAN_CPP_D4_H_

#include "tests/cxx/iwyu_stricter_than_cpp-i5.h"

template <typename T1, typename T2>
struct TplDirectStruct7 {
  static constexpr auto s = sizeof(T1);
  T2* t2;
};

#endif
