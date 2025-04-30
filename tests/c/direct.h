//===--- direct.h - test input file for iwyu ------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// This file is meant to be directly #included by a .c file.  All it
// does is #include another file (which the .c file is thus
// #including indirectly).

#include "tests/c/indirect.h"
