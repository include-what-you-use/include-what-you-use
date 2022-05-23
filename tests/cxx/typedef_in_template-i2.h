//===--- typedef_in_template-i2.h - test input file for iwyu --------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//


#ifndef INCLUDE_WHAT_YOU_USE_TESTS_CXX_TYPEDEF_IN_TEMPLATE_I2_H_
#define INCLUDE_WHAT_YOU_USE_TESTS_CXX_TYPEDEF_IN_TEMPLATE_I2_H_

class Class2 {};

template <class T, class U>
struct Pair {
  T first;
  U second;
};

#endif // INCLUDE_WHAT_YOU_USE_TESTS_CXX_TYPEDEF_IN_TEMPLATE_I2_H_
