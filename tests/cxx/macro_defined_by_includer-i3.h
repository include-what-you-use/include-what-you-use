//===--- macro_defined_by_includer-i3.h - test input file for iwyu --------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// Header file to test including file that uses x-macro.

#define TYPE char
#include "tests/cxx/macro_defined_by_includer-xmacro.h"
#undef TYPE
