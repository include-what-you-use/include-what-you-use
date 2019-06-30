//===--- dotdot_indirect.h - test input file for iwyu ---------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// Include a file using relative path to help test canonicalization when IWYU
// suggests additions.

#ifndef INCLUDE_WHAT_YOU_USE_TESTS_DOTDOT_INDIRECT_H_
#define INCLUDE_WHAT_YOU_USE_TESTS_DOTDOT_INDIRECT_H_

#include "../indirect.h"

#endif  // INCLUDE_WHAT_YOU_USE_TESTS_DOTDOT_INDIRECT_H_
