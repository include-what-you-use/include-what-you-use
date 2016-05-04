//===--- macro_defined_by_includer-g3.h - test input file for iwyu --------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#define DIRECT_INCLUDE_GUARD_4
#include "tests/cxx/macro_defined_by_includer-g4.h"

#ifndef DIRECT_INCLUDE_GUARD_3
  #error Do not include directly
#else
  class GuardedInclude3 {};
#endif
