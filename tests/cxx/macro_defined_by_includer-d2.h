//===--- macro_defined_by_includer-d2.h - test input file for iwyu --------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// Tests case when file using macro is included from file defining macro but
// the macro user is first encountered as include in another file.

#define DIRECT_INCLUDE_GUARD_2
#include "tests/cxx/macro_defined_by_includer-i2.h"
#include "tests/cxx/macro_defined_by_includer-g2.h"
