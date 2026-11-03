//===--- implementation_file-d2.h - test input file for iwyu --------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef INCLUDE_WHAT_YOU_USE_TESTS_CXX_IMPLEMENTATION_FILE_D2_H_
#define INCLUDE_WHAT_YOU_USE_TESTS_CXX_IMPLEMENTATION_FILE_D2_H_

#include "tests/cxx/indirect.h"

class ImplFileD2Class {
 public:
  int InlineMethod();

  IndirectClass member;
};

#include "tests/cxx/implementation_file-d2.inl"

#endif  // INCLUDE_WHAT_YOU_USE_TESTS_CXX_IMPLEMENTATION_FILE_D2_H_

/**** IWYU_SUMMARY

(tests/cxx/implementation_file-d2.h has correct #includes/fwd-decls)

***** IWYU_SUMMARY */
