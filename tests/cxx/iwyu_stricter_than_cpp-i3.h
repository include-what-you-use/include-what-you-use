//===--- iwyu_stricter_than_cpp-i3.h - test input file for iwyu -----------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "tests/cxx/indirect.h"

struct IndirectStruct3 {
  typedef IndirectClass IndirectClassProvidingTypedef;

  using IndirectClassProvidingAl = IndirectClass;
};
