//===--- implementation_file-d1.h - test input file for iwyu --------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef INCLUDE_WHAT_YOU_USE_TESTS_CXX_IMPLEMENTATION_FILE_D1_H_
#define INCLUDE_WHAT_YOU_USE_TESTS_CXX_IMPLEMENTATION_FILE_D1_H_

class ImplFileD1Class {
 public:
  int InlineMethod();
};

#include "tests/cxx/implementation_file-d1.inl"

#endif  // INCLUDE_WHAT_YOU_USE_TESTS_CXX_IMPLEMENTATION_FILE_D1_H_
