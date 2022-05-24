//===--- iwyu_stricter_than_cpp-d3.h - test input file for iwyu -----------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "tests/cxx/iwyu_stricter_than_cpp-i3.h"
#include "tests/cxx/iwyu_stricter_than_cpp-i4.h"

typedef IndirectStruct3 IndirectStruct3ProvidingTypedef;
typedef IndirectStruct4 IndirectStruct4ProvidingTypedef;

using IndirectStruct3ProvidingAl = IndirectStruct3;
using IndirectStruct4ProvidingAl = IndirectStruct4;
