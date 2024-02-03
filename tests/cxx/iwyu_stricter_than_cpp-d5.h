//===--- iwyu_stricter_than_cpp-d5.h - test input file for iwyu -----------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "iwyu_stricter_than_cpp-i4.h"

// No forward-declaration of 'IndirectClass' here.

IndirectStruct4::IndirectClassNonProvidingTypedef RetNonProvidingTypedef();
