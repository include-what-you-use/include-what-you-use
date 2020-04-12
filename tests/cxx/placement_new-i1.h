//===--- placement_new-i1.h - test input file for iwyu ----*- C++ -*-------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef INCLUDE_WHAT_YOU_USE_TESTS_CXX_PLACEMENT_NEW_I1_H_
#define INCLUDE_WHAT_YOU_USE_TESTS_CXX_PLACEMENT_NEW_I1_H_

#include <new>

template <class T, class U>
class ClassTemplate {
 public:
  ClassTemplate() = default;

  T first;
  U second;
};

#endif  // INCLUDE_WHAT_YOU_USE_TESTS_CXX_PLACEMENT_NEW_I1_H_
