//===--- user_defined_literal-indirect.h - test input file for iwyu -------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef INCLUDE_WHAT_YOU_USE_TESTS_CXX_USER_DEFINED_LITERAL_INDIRECT_H_
#define INCLUDE_WHAT_YOU_USE_TESTS_CXX_USER_DEFINED_LITERAL_INDIRECT_H_

constexpr unsigned long long operator""_x(unsigned long long value) {
  return value;
}

class IndirectClass {};

#endif  // INCLUDE_WHAT_YOU_USE_TESTS_CXX_USER_DEFINED_LITERAL_INDIRECT_H_
