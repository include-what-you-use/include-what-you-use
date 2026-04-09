//===--- unnamed-i2.h - iwyu test -----------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "tests/c/unnamed-i1.h"

extern typeof(var_of_unnamed_type) deduced_var;
