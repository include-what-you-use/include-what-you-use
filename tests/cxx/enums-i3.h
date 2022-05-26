//===--- enums-i3.h - test input file for iwyu ----------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef INCLUDE_WHAT_YOU_USE_TESTS_CXX_ENUMS_I3_H_
#define INCLUDE_WHAT_YOU_USE_TESTS_CXX_ENUMS_I3_H_

struct Struct1 {
  enum class IndirectEnum3 { A, B, C };
};

#endif  // INCLUDE_WHAT_YOU_USE_TESTS_CXX_ENUMS_I3_H_
