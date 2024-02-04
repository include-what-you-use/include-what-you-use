//===--- iwyu_stricter_than_cpp-d1.h - test input file for iwyu -----------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef INCLUDE_WHAT_YOU_USE_TESTS_CXX_IWYU_STRICTER_THAN_CPP_D1_H_
#define INCLUDE_WHAT_YOU_USE_TESTS_CXX_IWYU_STRICTER_THAN_CPP_D1_H_

#include "tests/cxx/iwyu_stricter_than_cpp-i1.h"
#include "tests/cxx/iwyu_stricter_than_cpp-i2.h"

// Note all these structs have an 'autocast' constructor.

struct DirectStruct1 {
  DirectStruct1(int) {
  }
};
struct DirectStruct2 {
  DirectStruct2(int) {
  }
};
struct DirectStruct3 {
  DirectStruct3(int) {
  }
};
struct DirectStruct4 {
  DirectStruct4(int) {
  }
};
struct DirectStruct5 {
  DirectStruct5(int) {
  }
};
struct DirectStruct6 {
  DirectStruct6(int) {
  }
};
struct IndirectStructForwardDeclaredInD1;

template <typename T>
struct TplDirectStruct1 {
  TplDirectStruct1(int) {
  }
};
template <typename T>
struct TplDirectStruct2 {
  TplDirectStruct2(int) {
  }
};
template <typename T>
struct TplDirectStruct3 {
  TplDirectStruct3(int) {
  }
};
template <typename T>
struct TplDirectStruct4 {
  TplDirectStruct4(int) {
  }
};
template <typename T>
struct TplDirectStruct5 {
  TplDirectStruct5(int) {
  }
};
template <typename T>
struct TplDirectStruct6 {
  TplDirectStruct6(int) {
  }
};
template <typename T>
struct TplIndirectStructForwardDeclaredInD1;

#endif  // INCLUDE_WHAT_YOU_USE_TESTS_CXX_IWYU_STRICTER_THAN_CPP_D1_H_
