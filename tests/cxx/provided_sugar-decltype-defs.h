//===--- provided_sugar-decltype-defs.h - test input file for iwyu --------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef INCLUDE_WHAT_YOU_USE_TESTS_CXX_PROVIDED_SUGAR_DECLTYPE_DEFS_H_
#define INCLUDE_WHAT_YOU_USE_TESTS_CXX_PROVIDED_SUGAR_DECLTYPE_DEFS_H_

class DecltypeReturn {};

class DecltypeArgument {
 public:
  DecltypeArgument(int);
};

#endif  // INCLUDE_WHAT_YOU_USE_TESTS_CXX_PROVIDED_SUGAR_DECLTYPE_DEFS_H_
