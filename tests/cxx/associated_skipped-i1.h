//===--- associated_skipped-i1.h - test input file for iwyu ---------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef INCLUDE_WHAT_YOU_USE_TESTS_CXX_ASSOCIATED_SKIPPED_I1_H_
#define INCLUDE_WHAT_YOU_USE_TESTS_CXX_ASSOCIATED_SKIPPED_I1_H_

static inline int quad(int v) {
  return v * 4;
}

#endif  // INCLUDE_WHAT_YOU_USE_TESTS_CXX_ASSOCIATED_SKIPPED_I1_H_
