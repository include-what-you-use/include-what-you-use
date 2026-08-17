//===--- provided_sugar-typeof-defs.h - test input file for iwyu ----------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef INCLUDE_WHAT_YOU_USE_TESTS_CXX_PROVIDED_SUGAR_TYPEOF_DEFS_H_
#define INCLUDE_WHAT_YOU_USE_TESTS_CXX_PROVIDED_SUGAR_TYPEOF_DEFS_H_

class TypeofReturn {};

class TypeofArgument {
 public:
  TypeofArgument(int);
};

#endif  // INCLUDE_WHAT_YOU_USE_TESTS_CXX_PROVIDED_SUGAR_TYPEOF_DEFS_H_
