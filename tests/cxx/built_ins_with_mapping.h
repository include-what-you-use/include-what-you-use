//===--- built_ins_with_mapping.h - test input file for iwyu --------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef INCLUDE_WHAT_YOU_USE_TESTS_CXX_BUILT_INS_WITH_MAPPING_H_
#define INCLUDE_WHAT_YOU_USE_TESTS_CXX_BUILT_INS_WITH_MAPPING_H_

// This is to simulate the situation where a builtin exists on some compilers,
// and not others, so we need a mapping.  However, we need to check that the
// header mapped to (this header, in this case) is not forced to include itself
// if it uses that builtin.

void __builtin_invented_for_test();

inline void f()
{
  // A regular function mapped to this file
  __builtin_invented_for_test();
  // A builtin mapped to this file
  __builtin_strcmp("", "");
}

#endif  // INCLUDE_WHAT_YOU_USE_TESTS_CXX_BUILT_INS_WITH_MAPPING_H_

/**** IWYU_SUMMARY

(tests/cxx/built_ins_with_mapping.h has correct #includes/fwd-decls)

***** IWYU_SUMMARY */
