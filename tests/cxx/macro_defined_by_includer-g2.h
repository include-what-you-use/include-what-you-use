//===--- macro_defined_by_includer-g2.h - test input file for iwyu --------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef INCLUDE_WHAT_YOU_USE_TESTS_CXX_MACRO_DEFINED_BY_INCLUDER_G2_H_
#define INCLUDE_WHAT_YOU_USE_TESTS_CXX_MACRO_DEFINED_BY_INCLUDER_G2_H_

#ifndef DIRECT_INCLUDE_GUARD_2
  #error Do not include directly
#else
  class GuardedInclude2 {};
#endif

#endif  // INCLUDE_WHAT_YOU_USE_TESTS_CXX_MACRO_DEFINED_BY_INCLUDER_G2_H_
