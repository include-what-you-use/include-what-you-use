//===--- placement_new-i2.h - test input file for iwyu ----*- C++ -*-------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef INCLUDE_WHAT_YOU_USE_TESTS_CXX_PLACEMENT_NEW_I2_H_
#define INCLUDE_WHAT_YOU_USE_TESTS_CXX_PLACEMENT_NEW_I2_H_

// Naive implementation of std::addressof for test.
template<class T>
T* AddressOf(T& t) {
  return &t;
}

#endif  // INCLUDE_WHAT_YOU_USE_TESTS_CXX_PLACEMENT_NEW_I2_H_
