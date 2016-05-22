//===--- check_also-i1.h - test input file for iwyu -----------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// Tests the '--check_also' flag.

#ifndef INCLUDE_WHAT_YOU_USE_TESTS_CXX_CHECK_ALSO_I1_H_
#define INCLUDE_WHAT_YOU_USE_TESTS_CXX_CHECK_ALSO_I1_H_

#include <stddef.h>   // for NULL

const int kI1 = 1;

#endif  // INCLUDE_WHAT_YOU_USE_TESTS_CXX_CHECK_ALSO_I1_H_
