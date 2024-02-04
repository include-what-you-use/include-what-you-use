//===--- include_cycle-d1.h - test input file for iwyu --------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef INCLUDE_WHAT_YOU_USE_TESTS_CXX_INTERNAL_INCLUDE_CYCLE_D1_H_
#define INCLUDE_WHAT_YOU_USE_TESTS_CXX_INTERNAL_INCLUDE_CYCLE_D1_H_

// Include itself (cycle length 1)
#include "tests/cxx/internal/include_cycle-d1.h"

// Include another file that includes itself (cycle length 2)
#include "tests/cxx/internal/include_cycle-i1.h"

struct IncludeCycleD1 {};

#endif  // INCLUDE_WHAT_YOU_USE_TESTS_CXX_INTERNAL_INCLUDE_CYCLE_D1_H_
