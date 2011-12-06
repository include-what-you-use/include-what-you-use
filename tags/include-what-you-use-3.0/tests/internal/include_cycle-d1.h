//===--- include_cycle-d1.h - test input file for iwyu --------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef INCLUDE_CYCLE_D1_H
#define INCLUDE_CYCLE_D1_H

// Include itself (cycle length 1)
#include "tests/internal/include_cycle-d1.h"

// Include another file that includes itself (cycle length 2)
#include "tests/internal/include_cycle-i1.h"

struct IncludeCycleD1 {};

#endif  /* #ifndef INCLUDE_CYCLE_D1_H */
