//===--- enum_reexport_func.h - test input file for iwyu ------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "tests/cxx/enum_reexport_enum.h"

// Declare a couple of free functions taking enums both nested and globally
// scoped.
void FreeFunction(Unscoped::Enum);
void FreeFunction(UnscopedEnum);

// Declare a class with methods taking enums, both nested and globally scoped.
class Class {
public:
  void Method(Unscoped::Enum);
  void Method(UnscopedEnum);
};
