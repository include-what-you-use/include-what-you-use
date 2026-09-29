//===--- associated_icc_redundant_include.h - test input file for iwyu ----===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef INCLUDE_WHAT_YOU_USE_TESTS_CXX_ASSOCIATED_ICC_REDUNDANT_INCLUDE_H_
#define INCLUDE_WHAT_YOU_USE_TESTS_CXX_ASSOCIATED_ICC_REDUNDANT_INCLUDE_H_

class RedundantIccClass {
 public:
  void InlinedMethod();
};

#include "tests/cxx/associated_icc_redundant_include.icc"

#endif  // INCLUDE_WHAT_YOU_USE_TESTS_CXX_ASSOCIATED_ICC_REDUNDANT_INCLUDE_H_

/**** IWYU_SUMMARY

(tests/cxx/associated_icc_redundant_include.h has correct #includes/fwd-decls)

***** IWYU_SUMMARY */
