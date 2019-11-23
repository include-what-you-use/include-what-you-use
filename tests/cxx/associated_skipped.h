//===--- associated_skipped.h - test input file for iwyu ------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef INCLUDE_WHAT_YOU_USE_TESTS_CXX_ASSOCIATED_SKIPPED_H_
#define INCLUDE_WHAT_YOU_USE_TESTS_CXX_ASSOCIATED_SKIPPED_H_

static inline int twice(int v) {
  return v * 2;
}

#endif  // INCLUDE_WHAT_YOU_USE_TESTS_CXX_ASSOCIATED_SKIPPED_H_

/**** IWYU_SUMMARY

(tests/cxx/associated_skipped.h has correct #includes/fwd-decls)

***** IWYU_SUMMARY */
