//===--- iwyu_stricter_than_cpp-i4.h - test input file for iwyu -----------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef INCLUDE_WHAT_YOU_USE_TESTS_CXX_IWYU_STRICTER_THAN_CPP_I4_H_
#define INCLUDE_WHAT_YOU_USE_TESTS_CXX_IWYU_STRICTER_THAN_CPP_I4_H_

class IndirectClass;

struct IndirectStruct4 {
  typedef IndirectClass IndirectClassNonProvidingTypedef;

  using IndirectClassNonProvidingAl = IndirectClass;
};

#endif  // INCLUDE_WHAT_YOU_USE_TESTS_CXX_IWYU_STRICTER_THAN_CPP_I4_H_
