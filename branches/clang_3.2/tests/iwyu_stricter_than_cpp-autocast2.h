//===--- iwyu_stricter_than_cpp-autocast2.h - test input file for iwyu ----===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "tests/iwyu_stricter_than_cpp-d1.h"

// Because we do not forward-declare IndirectStruct2, we need the full
// definition here.  But that fact doesn't matter because the caller
// of TwiceDeclaredFunction (-d2.h), cannot see this definition.
void TwiceDeclaredFunction(IndirectStruct2 ic2);
void TwiceDeclaredRefFunction(const IndirectStruct2& ic2);
