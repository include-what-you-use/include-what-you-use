//===--- macro_defined_by_includer-d3.h - test input file for iwyu --------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// Tests nested guarded includes.  Covers the case when "public" mapping for
// some private include can be in fact private.

#define DIRECT_INCLUDE_GUARD_3
#include "tests/cxx/macro_defined_by_includer-g3.h"
